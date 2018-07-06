from Common.FSW_Common import *
from Serial_Communication.serial_communication import SerialCommunication

class Telemetry(FlightSoftwareParent):
    """
    This class is designed to handle all telemetry on the RMC 549 Balloon(s).

    Written by Daniel Letros, 2018-07-02
    """

    def __init__(self, logging_object: Logger, serial_object: SerialCommunication) -> None:
        self.main_delay           = 0.5
        self.data_downlink_delay  = 9
        self.buffering_delay      = 0.05
        self.enable_telemetry     = True
        super().__init__("Telemetry", logging_object)
        self.serial_object         = serial_object

    def load_yaml_settings(self)->None:
        """
        This function loads in settings from the master_config.yaml file.

        Written by Daniel Letros, 2018-07-02

        :return: None
        """
        dirname = os.path.dirname(__file__)
        filename = os.path.join(dirname, self.yaml_config_path)
        with open(filename, 'r') as stream:
            content = yaml.load(stream)['telemetry']
        self.data_downlink_delay  = content['data_downlink_delay']
        self.buffering_delay      = content['buffering_delay']
        self.main_delay           = content['main_delay']
        self.enable_telemetry     = content['enable_telemetry']

    def run(self) -> None:
        """
        This function is the main loop of the telemetry for the RMC 549 balloon(s).

        Written by Daniel Letros, 2018-07-02

        :return: None
        """

        print("%s << %s << Starting Thread" % (self.system_name, self.class_name))
        tx_timer_start = datetime.datetime.now()
        tx_timer_end   = datetime.datetime.now()
        while self.should_thread_run:
            try:
                if self.enable_telemetry:
                    if self.serial_object.ports_are_good:
                        for port in self.serial_object.port_list:
                            with self.serial_object.serial_mutex:
                                # Check for uplink command
                                time.sleep(self.buffering_delay)
                                self.serial_object.write_request_buffer.append([port, "RX"])
                                time.sleep(self.buffering_delay)
                                self.serial_object.read_request_buffer.append([port, "RX"])
                                time.sleep(self.buffering_delay)

                            with self.serial_object.serial_mutex:
                                if (tx_timer_end - tx_timer_start).total_seconds() >= self.data_downlink_delay:
                                    # Send down some telemetry
                                    tx_timer_start = datetime.datetime.now()
                                    log_line = self.read_last_line_in_data_log()
                                    time.sleep(self.buffering_delay)
                                    self.serial_object.write_request_buffer.append([port, "TX%s" % log_line])
                                    time.sleep(self.buffering_delay)
                                    self.serial_object.read_request_buffer.append([port, "TX"])
                                    time.sleep(self.buffering_delay)

                            with self.serial_object.uplink_commands_mutex:
                                # All commands deleted OR all threads have seen what they want to.
                                if len(self.serial_object.last_uplink_commands) == 0 or \
                                        self.serial_object.last_uplink_seen_by_system_control:
                                    self.serial_object.last_uplink_seen_by_system_control = False
                                    self.serial_object.last_uplink_commands_valid         = False
            except:
                pass
                tx_timer_end = datetime.datetime.now()
            time.sleep(self.main_delay)
        print("%s << %s << End Thread" % (self.system_name, self.class_name))