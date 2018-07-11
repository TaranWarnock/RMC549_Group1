import threading
import yaml
import os
import datetime
import time
import socket
from sys import platform

class Logger(threading.Thread):
    """
    This is the logging class of the RMC 549 Balloon(s). Every other thread/object will see this object and use it for
    logging.

    Since this is the logging class there will be no way of elegantly recording a failre here if it happens. Therefore,
    if something goes wrong in a bad way, do a hard crash (where it makes sense to do so) so the program won't run at
    all, that is probably be the best way to notify of a problem.

    Written by Daniel Letros, 2018-06-27
    """
    def __init__(self):
        """
        Init function of the logger class.

        Written by Daniel Letros, 2018-06-27
        """
        super().__init__()
        self.log_file_path           = None  # Path to where all the log files are stored.
        self.notifications_log_path  = None  # Path to where the notification file for today is stored.
        self.data_log_path           = None  # Path to where the data file for today is stored.
        self.main_delay              = None  # Thread delay time.

        self.should_thread_run       = True  # Should thread be running.
        self.log_file_verbose        = False # Should print log file activity to console.

        # Diagnostic variables
        self.run_logger_diagnostics                   = False
        self.current_logger_diagnostics_function_name = None
        self.function_logger_diagnostics_start_time   = None
        self.function_logger_diagnostics_end_time     = None

        self.class_name  = "Logger"             # Class name for logger
        self.system_name = socket.gethostname() # System name for logger

        self.notifications_logging_buffer = []  # Buffer of information to be logged to the notifications file.
        self.data_logging_buffer          = []  # Buffer of information to be logged to the data file.

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

        self.load_yaml_settings()
        self.instantiate_log_files()

    def load_yaml_settings(self)->None:
        """
        This function loads in settings from the master_config.yaml file.

        Written by Daniel Letros, 2018-06-30

        :return: None
        """
        dirname = os.path.dirname(__file__)
        filename = os.path.join(dirname, self.yaml_config_path)
        with open(filename, 'r') as stream:
            content = yaml.load(stream)['logger']

        if platform == "linux" or platform == "linux2":
            self.log_file_path = content['log_file_path_Linux']
        elif platform == "darwin":
            self.log_file_path = content['log_file_path_mac']
        elif platform == "win32":
            self.log_file_path = content['log_file_path_Windows']
        else:
            self.yaml_config_path = None


        self.log_file_verbose       = content['log_file_verbose']
        self.run_logger_diagnostics = content['run_logger_diagnostics']
        self.main_delay             = content['main_delay']

    def instantiate_log_files(self)->None:
        """
        This function makes the log files which data/error messages will be stored. Everyday a new logfile folder will
        be created and all log files for that day will be stored there.

        Written by Daniel Letros, 2018-06-27

        :return: None
        """
        # Follow the main logging path and then make a timestamped folder in there for the current day.
        # Within the current day's folder, make two timestamped files for the day - notifications and data.
        timestamp = datetime.datetime.utcnow().strftime("%Y%m%d")
        self.log_file_path += os.sep + timestamp
        self.notifications_log_path = self.log_file_path + os.sep + timestamp + "_notifications.txt"
        self.data_log_path          = self.log_file_path + os.sep + timestamp + "_data.txt"
        if not os.path.exists(self.log_file_path):
            os.makedirs(self.log_file_path)
        if not os.path.exists(self.notifications_log_path):
            open(self.notifications_log_path, 'w').close()
        if not os.path.exists(self.data_log_path):
            open(self.data_log_path, 'w').close()

    def write_notification_to_log(self)->None:
        """
        This function will write a item in the notifications log buffer to the appropriate log file and print it to
        console.

        Since new items are appended to the buffer write the first index (oldest in time) lines first and then delete
        them from the buffer.

        Written by Daniel Letros, 2018-06-27

        :return: None
        """

        # Do a try except here since there is no need to crash the program if something goes wrong with the file write.
        self.start_logger_diagnostics("write_notification_to_log")
        try:
            with open(self.notifications_log_path, 'a') as file:
                file.write(self.notifications_logging_buffer[0])
            if self.log_file_verbose:
                print("NOTIFICATION << " + self.notifications_logging_buffer[0])
            del self.notifications_logging_buffer[0]
        except:
            print('FAILED TO WRITE [%s] TO LOG FILE' %(self.notifications_logging_buffer[0]))
        self.end_logger_diagnostics("write_notification_to_log")

    def write_data_to_log(self)->None:
        """
        This function will write a item in the data log buffer to the appropriate log file and print it to
        console.

        Since new items are appended to the buffer write the first index (oldest in time) lines first and then delete
        them from the buffer.

        Written by Daniel Letros, 2018-06-27

        :return: None
        """

        # Do a try except here since there is no need to crash the program if something goes wrong with the file write.
        self.start_logger_diagnostics("write_data_to_log")
        try:
            with open(self.data_log_path, 'a') as file:
                file.write(self.data_logging_buffer[0])
            if self.log_file_verbose:
                print("DATA << " + self.data_logging_buffer[0])
            del self.data_logging_buffer[0]
        except:
            print('FAILED TO WRITE [%s] TO LOG FILE' % (self.data_logging_buffer[0]))
        self.end_logger_diagnostics("write_data_to_log")

    def start_logger_diagnostics(self, function_name: str):
        """
        This function will start function diagnostics on all logger functions

        Written by Daniel Letros, 2018-06-27

        :return: None
        """
        if self.run_logger_diagnostics:
            self.current_logger_diagnostics_function_name = function_name
            self.function_logger_diagnostics_start_time   = datetime.datetime.now()

    def end_logger_diagnostics(self, function_name: str):
        """
        This function will end function diagnostics on all logger functions

        Written by Daniel Letros, 2018-06-27

        :return: None
        """
        if self.run_logger_diagnostics:
            if self.current_logger_diagnostics_function_name != function_name:
                print("DIAGNOSTICS << NAMES DO NOT MATCH << GOT [%s] EXPECTED [%s]"
                      %(function_name, self.current_logger_diagnostics_function_name))
            self.function_logger_diagnostics_end_time = datetime.datetime.now()
            print("DIAGNOSTICS << [%s] << TIME_TAKEN [%f]"
                  %(function_name,
                    (self.function_logger_diagnostics_end_time -
                     self.function_logger_diagnostics_start_time).total_seconds()))
            print("\n")

    def run(self):
        """
        This is the main function of the logging thread.

        Written by Daniel Letros, 2018-06-27

        :return: None
        """
        print("%s << %s << Starting Thread" % (self.system_name, self.class_name))
        while self.should_thread_run:
            if len(self.notifications_logging_buffer) > 0:
                self.write_notification_to_log()
            elif len(self.data_logging_buffer) > 0:
                self.write_data_to_log()
            time.sleep(self.main_delay)
        print("%s << %s << Ending Thread" % (self.system_name, self.class_name))

