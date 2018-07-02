from Common.FSW_Common import *


class SerialCommunication(FlightSoftwareParent):
    """
    This class is designed to handle generic serial communication for the RMC 549 balloon(s).

    Written by Daniel Letros, 2018-06-27
    """

    def __init__(self, logging_object) -> None:
        super().__init__("SerialCommunication", logging_object)

        self.port_list        = dict()
        self.ports_are_good   = False
        self.default_buadrate = 9600
        self.default_timeout  = 0
        self.main_delay       = 0.5

        self.expect_read_after_write = False  # Used to facilitate a call and respond system by default.
                                              # Can be circumvented by changing state elsewhere in code.

        self.read_request_buffer  = []  # buffer of ports to read from, [port, message_type], ...]
        self.write_request_buffer = []  # buffer of ports to write to, [[port, message], ...]

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
            content = yaml.load(stream)['serial_communication']
        self.default_buadrate = content['default_baud_rate']
        self.default_timeout  = content['default_timeout']
        self.main_delay       = content['main_delay']


    def find_serial_ports(self, baudrate: int = None, timeout: float = None) -> None:
        """
        This function finds active serial ports and makes the serial connections. This function should
        only be run on startup or if something has changed with the connections.

        Code from : https://stackoverflow.com/questions/12090503/listing-available-com-ports-with-python

        Adapted by: Daniel Letros, 2018-06-27

        :param baudrate: buadrate of the connection
        :param timeout: Communication timeout
        :raises EnvironmentError: On unsupported platform
        :return: List of the serial ports available
        """
        self.start_function_diagnostics("find_serial_ports")

        if baudrate is None:
            baudrate = self.default_buadrate
        if timeout is None:
            timeout = self.default_timeout

        # Clear old connections if any.
        for port in self.port_list:
            self.port_list[port].close()
        self.port_list.clear()

        # Determine ports on current OS
        if sys.platform.startswith('win'):
            ports = ['COM%s' % (i + 1) for i in range(256)]
        elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
            # this excludes your current terminal "/dev/tty"
            ports = glob.glob('/dev/tty[A-Za-z]*')
        elif sys.platform.startswith('darwin'):
            ports = glob.glob('/dev/tty.*')
        else:
            raise EnvironmentError('Unsupported platform')

        result = []
        for port in ports:
            try:
                s = serial.Serial(port)
                s.close()
                result.append(port)
            except (OSError, serial.SerialException):
                pass

        # Open ports
        for port in result:
            self.port_list[port] = serial.Serial(port=port, baudrate=baudrate,
                               parity=serial.PARITY_NONE,
                               stopbits=serial.STOPBITS_ONE,
                               bytesize=serial.EIGHTBITS,
                               timeout=timeout,
                               writeTimeout=timeout)

        self.end_function_diagnostics("find_serial_ports")

    def readline_from_serial(self, port: str, type: str) -> None:
        """
        This function will read data on the port up to a EOL char and return it.

        Written by Daniel Letros, 2018-06-27

        :param port: port to do the communication over
        :param type: type of data expected, dictated which file it is logged to.
        :return: None
        """
        self.start_function_diagnostics("readline_from_serial")
        try:
            new_data = self.port_list[port].readline().decode('utf-8').strip()
            if new_data is "":
                self.log_error("[%s] returned no data." % port)
                self.port_list[port].reset_input_buffer()
            elif type is "DATA":
                self.log_data(new_data)
                self.log_info("received [%s] information over [%s]" % (type, port))
            elif type is "ID":
                self.log_id(new_data)
                self.log_info("received [%s] information over [%s]" % (type, port))
            elif type is "HEADER":
                self.log_header(new_data)
                self.log_info("received [%s] information over [%s]" % (type, port))
        except Exception as err:
            self.log_error(str(err))
            self.ports_are_good = False
        self.end_function_diagnostics("readline_from_serial")

    def write_to_serial(self, port: str, message: str) -> None:
        """
        This function will write data to the port during serial communication.

        Written by Daniel Letros, 2018-06-27

        :param port: The port for the serial communication
        :param message: The message/data to write
        :return: None
        """
        self.start_function_diagnostics("write_to_serial")
        try:
            self.port_list[port].write(message.encode('utf-8'))
            self.log_info("sent [%s] over [%s]" % (message, port))
        except Exception as err:
            self.log_error(str(err))
            self.port_list[port].reset_output_buffer()
            self.ports_are_good = False
        self.end_function_diagnostics("write_to_serial")

    def log_id(self, log_message: str) -> None:
        """
        This function will que the input message to be logged as a device id line to the notifications log file.

        Written by Daniel Letros, 2018-06-27

        :param log_message: Info message to log
        :return: None
        """
        self.logger.notifications_logging_buffer.append("ID << %s << %s << %s << %s\n" % (
            datetime.datetime.utcnow().strftime("%Y%m%d_%H:%M:%S"), self.system_name, self.class_name, log_message))

    def log_header(self, log_message: str) -> None:
        """
        This function will que the input message to be logged as a data header to the notifications log file.

        Written by Daniel Letros, 2018-06-27

        :param log_message: Info message to log
        :return: None
        """
        self.logger.notifications_logging_buffer.append("HEADER << %s << %s << %s << %s\n" % (
            datetime.datetime.utcnow().strftime("%Y%m%d_%H:%M:%S"), self.system_name, self.class_name, log_message))


    def run(self):
        print("%s << %s << Starting Thread" % (self.system_name, self.class_name))
        while self.should_thread_run:
            try:
                if len(self.read_request_buffer) > 0 and self.expect_read_after_write:
                    self.readline_from_serial(self.read_request_buffer[0][0], self.read_request_buffer[0][1])
                    del self.read_request_buffer[0]
                    self.expect_read_after_write = False
                elif len(self.write_request_buffer) > 0 and not self.expect_read_after_write:
                    self.write_to_serial(self.write_request_buffer[0][0], self.write_request_buffer[0][1])
                    del self.write_request_buffer[0]
                    self.expect_read_after_write = True
            except:
                pass
            time.sleep(self.main_delay)
        print("%s << %s << Exiting Thread" % (self.system_name, self.class_name))
