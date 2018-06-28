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
from Flight_Software_Package.Logger.logger import *

"""
This package acts as a super parent which all other things (except the logger) inherent from.
Any code but the logger running on the RMC 549 balloon(s) will have these libs, functions, and properties.
The logger will still have most of the same properties.
"""
class FlightSoftwareParent(threading.Thread):
    def __init__(self, class_name: str, logging_object: Logger) -> None:
        super().__init__()
        self.class_name        = class_name
        self.system_name       = socket.gethostname()
        self.logger            = logging_object
        self.should_thread_run = True

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

