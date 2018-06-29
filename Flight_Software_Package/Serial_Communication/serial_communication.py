from Flight_Software_Package.Common.FSW_Common import *


class SerialCommunication(FlightSoftwareParent):
    """
    This class is designed to handle generic serial communication for the RMC 549 balloon(s).

    Written by Daniel Letros, 2018-06-27
    """

    def __init__(self, logging_object) -> None:
        super().__init__("SerialCommunication", logging_object)

        self.read_request_buffer  = []
        self.write_request_buffer = []

    @staticmethod
    def find_serial_ports() -> list:
        """
        This function finds active serial ports and returns their names in a list.

        Code from : https://stackoverflow.com/questions/12090503/listing-available-com-ports-with-python

        Adapted by: Daniel Letros, 2018-06-27

        :raises EnvironmentError: On unsupported platform
        :return: List of the serial ports available
        """

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
        return result

    def readline_from_serial(self, port: str, baudrate: int = 19200, timeout: float = 1) -> None:
        """
        This function will read data on the port up to a EOL char and return it.

        Written by Daniel Letros, 2018-06-27

        :param port: port to do the communication over
        :param baudrate: buadrate of the connection
        :param timeout: Communication timeout
        :return: None
        """
        try:
            with serial.Serial(port=port, baudrate=baudrate,
                               parity=serial.PARITY_NONE,
                               stopbits=serial.STOPBITS_ONE,
                               bytesize=serial.EIGHTBITS,
                               timeout=timeout) as ser:
                self.log_data(ser.readline().decode('utf-8').strip())
        except Exception as err:
            self.log_error(str(err))

    def write_to_serial(self, message: str, port: str, baudrate: int = 19200, timeout: float = 1) -> None:
        """
        This function will write data to the port during serial communication.

        Written by Daniel Letros, 2018-06-27

        :param message: The message/data to write
        :param port: The port for the serial communication
        :param baudrate: The buadrate of the connection
        :param timeout: Connection timeout
        :return: None
        """
        try:
            with serial.Serial(port=port, baudrate=baudrate,
                               parity=serial.PARITY_NONE,
                               stopbits=serial.STOPBITS_ONE,
                               bytesize=serial.EIGHTBITS,
                               writeTimeout=timeout) as ser:
                ser.write(message.encode('utf-8'))
                self.log_info("sent [%s] over [%s]" % (message, port))
        except Exception as err:
            self.log_error(str(err))

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
