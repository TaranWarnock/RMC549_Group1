from Common.FSW_Common import *
from Serial_Communication.serial_communication import SerialCommunication

class SystemControl(FlightSoftwareParent):
    """
    This class is designed to handle system control on the RMC 549 Balloon(s).

    Written by Daniel Letros, 2018-07-03
    """

    def __init__(self, logging_object: Logger, serial_object: SerialCommunication) -> None:
        self.main_delay        = 0.05
        self.cutoff_pin_bcm    = 18
        self.cutoff_time_high  = 5
        self.cutoff_conditions = dict()
        self.cutoff_conditions['gps_altitude'] = 30
        self.cutoff_conditions['gps_lat']      = [22, 23]
        self.cutoff_conditions['gps_lon']      = [24, 25]
        self.cutoff_conditions['time']         = "12:34"
        super().__init__("SystemControl", logging_object)
        self.serial_object = serial_object

        self.board_IDs   = None
        self.data_header = None

        try:
            # Configure the cutoff pin
            GPIO.setwarnings(False)
            GPIO.setmode(GPIO.BCM)
            GPIO.setup(self.cutoff_pin_bcm, GPIO.OUT, initial=GPIO.LOW)
        except:
            self.log_error("Could not configure BCM pin [%s]"% self.cutoff_pin_bcm)

    def load_yaml_settings(self)->None:
        """
        This function loads in settings from the master_config.yaml file.

        Written by Daniel Letros, 2018-07-03

        :return: None
        """
        dirname = os.path.dirname(__file__)
        filename = os.path.join(dirname, self.yaml_config_path)
        with open(filename, 'r') as stream:
            content = yaml.load(stream)['system_control']
        self.main_delay        = content['main_delay']
        self.cutoff_pin_bcm    = content['cutoff_BCM_pin_number']
        self.cutoff_conditions = content['cut_conditions']
        self.cutoff_time_high  = content['cutoff_time_high']

    def check_id_and_headers(self):
        with open(self.logger.notifications_log_path, 'r') as f:
            content = f.readlines()
        for line_idx in range(len(content), 0, -1):
            if content[line_idx].split("<<")[0].strip() is "ID":
                pass # TODO finish looking for header files

    def run(self) -> None:
        """
        This function is the main loop of the system control for the RMC 549 balloon(s).
        Payload cutoff is handled here.

        Written by Daniel Letros, 2018-07-03

        :return: None
        """

        print("%s << %s << Starting Thread" % (self.system_name, self.class_name))
        while self.should_thread_run:
            try:
                # log_line = self.read_last_line_in_data_log()
                if self.serial_object.last_uplink_commands_valid:
                    # Loop through list of uplink commands and check if any of them are useful to this class.
                    # If they are delete them from main uplink list before use and then carry out the commands.
                    # if no commands are left in main uplink list then set valid flag to false.
                    # It is done this way to minimize time with mutex lock.
                    with self.serial_object.uplink_commands_mutex:
                        commands_to_remove_and_use = []
                        for command in self.serial_object.last_uplink_commands:
                            if command.lower() == 'cut the mofo':
                                commands_to_remove_and_use.append(command)
                        for command in commands_to_remove_and_use:
                            self.serial_object.last_uplink_commands.remove(command)
                        self.serial_object.last_uplink_seen_by_system_control = True

                    for command in commands_to_remove_and_use:
                        if command.lower() == 'cut the mofo':
                            print("CUTTING PAYLOAD")
                            # GPIO.output(self.cutoff_pin_bcm, not GPIO.input(self.cutoff_pin_bcm))
                            GPIO.output(self.cutoff_pin_bcm, not GPIO.HIGH)
                            time.sleep(self.cutoff_time_high)
            except:
                pass
            time.sleep(self.main_delay)
        print("%s << %s << End Thread" % (self.system_name, self.class_name))
