from Flight_Software_Package.Common.FSW_Common import *
from Flight_Software_Package.Serial_Communication.serial_communication import SerialCommunication

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
            try:
                if not self.serial_object.ports_are_good:
                    self.serial_object.find_serial_ports()
                    if len(self.serial_object.port_list) > 0:
                        self.serial_object.ports_are_good = True
                if self.serial_object.ports_are_good:
                    for port in self.serial_object.port_list:
                        self.serial_object.read_request_buffer.append(port)
                else:
                    self.log_warning("Don't see any devices.")
            except:
                self.serial_object.ports_are_good = False
        print("%s << %s << End Thread" % (self.system_name, self.class_name))