import yaml
import os
import sys
import glob
import serial
import socket
import numpy as np
import threading
import time
import datetime
from sys import platform
if socket.gethostname() == "Rocky" or socket.gethostname() == "MajorTom":
    import RPi.GPIO as GPIO
    import smbus
from Logger.logger import *


"""
This code acts as a parent class which all other things (except the logger) inherent from.
Each class which is a child of this one is intended to run as its own thread. This means that
the code in each class can run in parallel with all the other code. Inter-thread communication 
is limited and handled largely by references and information saved in logged text files.
Any code but the logger running on the RMC 549 balloon(s) will have these libs, functions, and properties.
The logger will still have most of the same properties but declared separately.

None of this code should need to be adjusted to change configurable settings. All configuration/user defined settings
can and should only be adjusted using the master_config.yaml file.

Written by Daniel Letros, 2018-06-30
"""
class FlightSoftwareParent(threading.Thread):
    def __init__(self, class_name: str, logging_object: Logger) -> None:
        """
        Init function of class. This will be run by all child classes.

        :param class_name: Defines the name of the class as seen by the log files
        :param logging_object: Reference to the logging class

        Written by Daniel Letros, 2018-06-30
        """
        super().__init__()
        self.class_name               = class_name            # Defines the name of the class as seen by the log files.
        self.system_name              = socket.gethostname()  # Gets the name of the computer for logging purposes.
        self.logger                   = logging_object        # Reference to the logging class.
        self.should_thread_run        = True                  # Tells the code if the thread should run.

        self.run_function_diagnostics          = False        # Tells the code if function diagnostics need to be done.
        self.current_diagnostics_function_name = None         # Holds name of function being diagnosed.
        self.function_diagnostics_start_time   = None         # Start of the diagnostic timer.
        self.function_diagnostics_end_time     = None         # End of diagnostic timer.

        # Checks for current operating system and defines paths to configuration file. Linux path needs to be absolute
        # since the method of starting the program on pi power up needs absolute path for the proper file to be found.
        # Running the program through pycharm or manually through a terminal does not need absolute paths.
        if platform == "linux" or platform == "linux2":
            self.yaml_config_path = '/home/pi/RMC549Repos/RMC549_Group1/Flight_Software_Package/Config/master_config.yaml'
        elif platform == "darwin":
            self.yaml_config_path = '../Config/master_config.yaml'
        elif platform == "win32":
            self.yaml_config_path = '..\\Config\\master_config.yaml'
        else:
            self.yaml_config_path = None

        try:
            self.load_yaml_settings()
        except:
            self.log_warning("[%s] Failed to load yaml settings. Default values used." % self.class_name)

    def load_yaml_settings(self)->None:
        """
        This function loads in settings from the master_config.yaml file. This function gets overloaded (redefined by
        child classes).

        Written by Daniel Letros, 2018-06-30

        :return: None
        """

        # Construct path to yaml file
        dirname = os.path.dirname(__file__)
        filename = os.path.join(dirname, self.yaml_config_path)
        # Open yaml and read it.
        with open(filename, 'r') as stream:
            content = yaml.load(stream)['general']
        # Pull out data from yaml which is relevant to this parent class
        self.run_function_diagnostics = content['run_function_diagnostics']

    def log_error(self, log_message: str) -> None:
        """
        This function will que the input message to be logged as an error to the notifications log file.

        Written by Daniel Letros, 2018-06-27

        :param log_message: Error message to log
        :return: None
        """
        self.logger.notifications_logging_buffer.append("ERROR << %s << %s << %s << %s\n" % (
            datetime.datetime.utcnow().strftime("%Y%m%d_%H:%M:%S.%f"), self.system_name, self.class_name, log_message))

    def log_warning(self, log_message: str) -> None:
        """
        This function will que the input message to be logged as an warning to the notifications log file.

        Written by Daniel Letros, 2018-06-27

        :param log_message: Warning message to log
        :return: None
        """
        self.logger.notifications_logging_buffer.append("WARNING << %s << %s << %s << %s\n" % (
            datetime.datetime.utcnow().strftime("%Y%m%d_%H:%M:%S.%f"), self.system_name, self.class_name, log_message))

    def log_info(self, log_message: str) -> None:
        """
        This function will que the input message to be logged as general information to the notifications log file.

        Written by Daniel Letros, 2018-06-27

        :param log_message: Info message to log
        :return: None
        """
        self.logger.notifications_logging_buffer.append("INFO << %s << %s << %s << %s\n" % (
            datetime.datetime.utcnow().strftime("%Y%m%d_%H:%M:%S.%f"), self.system_name, self.class_name, log_message))

    def log_data(self, log_message: str) -> None:
        """
        This function will que the input message to be logged as data to the data log file.

        Written by Daniel Letros, 2018-06-27

        :param log_message: Info message to log
        :return: None
        """
        self.logger.data_logging_buffer.append("%s,%s\n" % (
            datetime.datetime.utcnow().strftime("%Y%m%d_%H:%M:%S.%f"), log_message))

    def read_last_line_in_data_log(self) -> str:
        """
        This function will read the last line in the data log file and return it

        Written by Daniel Letros, 2018-07-03

        :return: None
        """
        # Open file and try to read it backwards as a binary data. Stop when have only one line as identified by
        # the newline characters ('\n'). This is a time efficient way to get the last line in a file that is very large.
        file_name = self.logger.data_log_path
        try:
            with open(file_name, 'rb') as f:
                f.seek(-2, os.SEEK_END)
                while f.read(1) != b'\n':
                    f.seek(-2, os.SEEK_CUR)
                content = f.readline().decode()
        except:
            # If the data is such that the above method will fail then load all of it and just get the last line.
            with open(file_name, 'rb') as f:
                content = f.readlines()[-1].decode()
        return content

    def start_function_diagnostics(self, function_name: str):
        """
        This function will begin function diagnostics on all functions other then logger functions

        Written by Daniel Letros, 2018-06-27

        :return: None
        """
        # Currently the diagnostics only measure runtime. Do this by getting a start timestamp.
        if self.run_function_diagnostics:
            self.current_diagnostics_function_name = function_name
            self.function_diagnostics_start_time   = datetime.datetime.now()

    def end_function_diagnostics(self, function_name: str):
        """
        This function will end function diagnostics on all functions other then logger functions

        Written by Daniel Letros, 2018-06-27

        :return: None
        """
        # Do a rudimentary check to see if the logic behind using the diagnostic functions is subverted by checking
        # for consistent function names. After, get the ending time and report how long the function took.
        if self.run_function_diagnostics:
            if self.current_diagnostics_function_name != function_name:
                print("DIAGNOSTICS << NAMES DO NOT MATCH << GOT [%s] EXPECTED [%s]"
                      %(function_name, self.current_diagnostics_function_name))
            self.function_diagnostics_end_time = datetime.datetime.now()
            print("DIAGNOSTICS << [%s] << TIME_TAKEN [%f]"
                  %(function_name,
                    (self.function_diagnostics_end_time - self.function_diagnostics_start_time).total_seconds()))
            print("\n")