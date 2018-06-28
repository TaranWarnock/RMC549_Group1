from Flight_Software_Package.Common.FSW_Common import *
from Flight_Software_Package.Serial_Communication.serial_sommunication import SerialCommunication

class CommandAndControl(FlightSoftwareParent):
    """
    This class is designed to handle all commanding and controlling of the flight software for the RMC 549 balloon(s).

    Written by Daniel Letros, 2018-06-27
    """

    def __init__(self, logging_object, serial_object: SerialCommunication) -> None:
        super().__init__("CommandAndControl", logging_object)
        self.serial_object = serial_object

    def run(self):
        # Quick and dirty testing
        print("%s << %s << Starting Thread" % (self.system_name, self.class_name))
        while self.should_thread_run:
            if len(self.serial_object.find_serial_ports()) > 0:
                self.serial_object.read_request_buffer.append(self.serial_object.find_serial_ports()[0])
            else:
                self.log_warning("Don't see device.")
            time.sleep(1)
        print("%s << %s << End Thread" % (self.system_name, self.class_name))