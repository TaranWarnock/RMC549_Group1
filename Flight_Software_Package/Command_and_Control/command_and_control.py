from Common.FSW_Common import *
from Serial_Communication.serial_communication import SerialCommunication
from Telemetry.telemetry import Telemetry
from System_Control.system_control import SystemControl


class CommandAndControl(FlightSoftwareParent):
    """
    This class is designed to handle all commanding and controlling of the flight software for the RMC 549 balloon(s).

    Written by Daniel Letros, 2018-06-27
    """

    def __init__(self,
                 logging_object: Logger,
                 serial_object: SerialCommunication,
                 telemetry_object: Telemetry,
                 system_control_object: SystemControl) -> None:
        """
        Init function of the command and control class.

        :param logging_object: Reference to the logging object.
        :param serial_object: Reference to the serial object.
        :param telemetry_object: Reference to the telemetry object.
        :param system_control_object: Reference to the system control object.

        Written by Daniel Letros, 2018-06-27
        """
        self.buffering_delay              = 0.1  # A time delay for some aspects of code. Used primarily to debugging.
        self.que_data_delay               = 5    # How often data from the sensors will be asked for.

        super().__init__("CommandAndControl", logging_object)  # Call parent init.
        self.serial_object         = serial_object             # Reference to serial object
        self.system_control_object = system_control_object     # Reference to system control object
        self.telemetry_object      = telemetry_object          # Reference to telemetry object.

    def load_yaml_settings(self)->None:
        """
        This function loads in settings from the master_config.yaml file.

        Written by Daniel Letros, 2018-06-30

        :return: None
        """
        dirname = os.path.dirname(__file__)
        filename = os.path.join(dirname, self.yaml_config_path)
        with open(filename, 'r') as stream:
            content = yaml.load(stream)['command_and_control']
        self.buffering_delay              = content['buffering_delay']
        self.que_data_delay               = content['que_data_delay']

    def run(self) -> None:
        """
        This function is the main loop of the flight control software for the RMC 549 balloon(s).
        Software states and software/hardware communication is handled here.

        Written by Daniel Letros, 2018-06-30

        :return: None
        """

        print("%s << %s << Starting Thread" % (self.system_name, self.class_name))
        while self.should_thread_run:
            try:
                # Register all devices/sensors which this software can reach.
                # This will get re run if there is an error in communication
                if not self.serial_object.ports_are_good:
                    # Don't see anything connected, try to find devices.
                    self.serial_object.find_serial_ports()
                    if len(self.serial_object.port_list) > 0:
                        # Found some devices, YAY!
                        self.serial_object.ports_are_good = True
                        # Get ID's of devices and data headers.
                        with self.serial_object.serial_mutex:
                            for port in self.serial_object.port_list:
                                time.sleep(self.buffering_delay)
                                self.serial_object.write_request_buffer.append([port, "ID"])
                                time.sleep(self.buffering_delay)
                                self.serial_object.read_request_buffer.append( [port, "ID"])
                                time.sleep(self.buffering_delay)
                                self.serial_object.write_request_buffer.append([port, "HEADER"])
                                time.sleep(self.buffering_delay)
                                self.serial_object.read_request_buffer.append( [port, "HEADER"])
                                time.sleep(self.buffering_delay)

                if self.serial_object.ports_are_good:
                    with self.serial_object.serial_mutex:
                        # Everything is established so collect data.
                        for port in self.serial_object.port_list:
                            # Get some data and log it
                            time.sleep(self.buffering_delay)
                            self.serial_object.write_request_buffer.append([port, "DATA"])
                            time.sleep(self.buffering_delay)
                            self.serial_object.read_request_buffer.append([port, "DATA"])
                            time.sleep(self.buffering_delay)


                else:
                    self.log_warning("Don't see any devices.")
            except Exception as err:
                self.log_error("Main function error [%s]" % str(err))
            time.sleep(self.que_data_delay)
        print("%s << %s << End Thread" % (self.system_name, self.class_name))