from Flight_Software_Package.Common.FSW_Common import *


class SerialCommunication(FlightSoftwareParent):
    """
    This class is designed to handle generic serial communication for the RMC 549 balloon(s).

    Written by Daniel Letros, 2018-06-27
    """

    def __init__(self, logging_object) -> None:
        super().__init__("SerialCommunication", logging_object)

        self.port_list      = dict()
        self.ports_are_good = False

        self.read_request_buffer  = []
        self.write_request_buffer = []

    def find_serial_ports(self, baudrate: int = 9600, timeout: float = 0) -> None:
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

    def readline_from_serial(self, port: str) -> None:
        """
        This function will read data on the port up to a EOL char and return it.

        Written by Daniel Letros, 2018-06-27

        :param port: port to do the communication over
        :return: None
        """
        self.start_function_diagnostics("readline_from_serial")
        try:
            self.log_data(self.port_list[port].readline().decode('utf-8').strip())
        except Exception as err:
            self.log_error(str(err))
            self.ports_are_good = False
        self.end_function_diagnostics("readline_from_serial")

    def write_to_serial(self, message: str, port: str) -> None:
        """
        This function will write data to the port during serial communication.

        Written by Daniel Letros, 2018-06-27

        :param message: The message/data to write
        :param port: The port for the serial communication
        :return: None
        """
        self.start_function_diagnostics("write_to_serial")
        try:
            self.port_list[port].write(message.encode('utf-8'))
            self.log_info("sent [%s] over [%s]" % (message, port))
        except Exception as err:
            self.log_error(str(err))
            self.ports_are_good = False
        self.end_function_diagnostics("write_to_serial")

    def run(self):
        print("%s << %s << Starting Thread" % (self.system_name, self.class_name))
        while self.should_thread_run:
            try:
                if len(self.read_request_buffer) > 0:
                    self.readline_from_serial(self.read_request_buffer[0])
                    del self.read_request_buffer[0]
            except:
                pass
        print("%s << %s << Exiting Thread" % (self.system_name, self.class_name))
