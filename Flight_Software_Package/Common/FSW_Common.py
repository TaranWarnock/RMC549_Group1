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
from Logger.logger import *

"""
This package acts as a super parent which all other things (except the logger) inherent from.
Any code but the logger running on the RMC 549 balloon(s) will have these libs, functions, and properties.
The logger will still have most of the same properties.
"""
class FlightSoftwareParent(threading.Thread):
    def __init__(self, class_name: str, logging_object: Logger) -> None:
        super().__init__()
        self.class_name               = class_name
        self.system_name              = socket.gethostname()
        self.logger                   = logging_object
        self.should_thread_run        = True

        self.run_function_diagnostics          = False
        self.current_diagnostics_function_name = None
        self.function_diagnostics_start_time   = None
        self.function_diagnostics_end_time     = None

        try:
            self.load_yaml_settings()
        except:
            self.log_warning("Failed to load yaml settings. Default values used.")

    def load_yaml_settings(self)->None:
        """
        This function loads in settings from the master_config.yaml file.

        Written by Daniel Letros, 2018-06-30

        :return: None
        """
        dirname = os.path.dirname(__file__)
        filename = os.path.join(dirname, '..\\Config\\master_config.yaml')
        with open(filename, 'r') as stream:
            content = yaml.load(stream)['general']
        self.run_function_diagnostics = content['run_function_diagnostics']

    def log_error(self, log_message: str) -> None:
        """
        This function will que the input message to be logged as an error to the notifications log file.

        Written by Daniel Letros, 2018-06-27

        :param log_message: Error message to log
        :return: None
        """
        self.logger.notifications_logging_buffer.append("ERROR << %s << %s << %s << %s\n" % (
            datetime.datetime.utcnow().strftime("%Y%m%d_%H:%M:%S"), self.system_name, self.class_name, log_message))

    def log_warning(self, log_message: str) -> None:
        """
        This function will que the input message to be logged as an warning to the notifications log file.

        Written by Daniel Letros, 2018-06-27

        :param log_message: Warning message to log
        :return: None
        """
        self.logger.notifications_logging_buffer.append("WARNING << %s << %s << %s << %s\n" % (
            datetime.datetime.utcnow().strftime("%Y%m%d_%H:%M:%S"), self.system_name, self.class_name, log_message))

    def log_info(self, log_message: str) -> None:
        """
        This function will que the input message to be logged as general information to the notifications log file.

        Written by Daniel Letros, 2018-06-27

        :param log_message: Info message to log
        :return: None
        """
        self.logger.notifications_logging_buffer.append("INFO << %s << %s << %s << %s\n" % (
            datetime.datetime.utcnow().strftime("%Y%m%d_%H:%M:%S"), self.system_name, self.class_name, log_message))

    def log_data(self, log_message: str) -> None:
        """
        This function will que the input message to be logged as data to the data log file.

        Written by Daniel Letros, 2018-06-27

        :param log_message: Info message to log
        :return: None
        """
        self.logger.data_logging_buffer.append("%s, %s\n" % (
            datetime.datetime.utcnow().strftime("%Y%m%d_%H:%M:%S"), log_message))

    def start_function_diagnostics(self, function_name: str):
        if self.run_function_diagnostics:
            self.current_diagnostics_function_name = function_name
            self.function_diagnostics_start_time   = datetime.datetime.now()

    def end_function_diagnostics(self, function_name: str):
        if self.run_function_diagnostics:
            if self.current_diagnostics_function_name != function_name:
                print("DIAGNOSTICS << NAMES DO NOT MATCH << GOT [%s] EXPECTED [%s]"
                      %(function_name, self.current_diagnostics_function_name))
            self.function_diagnostics_end_time = datetime.datetime.now()
            print("DIAGNOSTICS << [%s] << TIME_TAKEN [%f]"
                  %(function_name,
                    (self.function_diagnostics_end_time - self.function_diagnostics_start_time).total_seconds()))