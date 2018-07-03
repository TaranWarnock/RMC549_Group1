from Common.FSW_Common import *

class SystemControl(FlightSoftwareParent):
    """
    This class is designed to handle system control on the RMC 549 Balloon(s).

    Written by Daniel Letros, 2018-07-03
    """

    def __init__(self, logging_object: Logger) -> None:
        self.main_delay        = 0.05
        self.cutoff_pin_bcm    = 18
        self.cutoff_conditions = dict()
        self.cutoff_conditions['gps_altitude'] = 30
        self.cutoff_conditions['gps_lat']      = [22, 23]
        self.cutoff_conditions['gps_lon']      = [24, 25]
        self.cutoff_conditions['time']         = "12:34"
        super().__init__("SystemControl", logging_object)

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
                time.sleep(20)
                log_line = self.read_last_line_in_data_log()
                print("SYSTEM CONTROL DEBUG: LOG LINE [%s]" % log_line)
                GPIO.output(self.cutoff_pin_bcm, not GPIO.input(self.cutoff_pin_bcm))
            except:
                pass
            time.sleep(self.main_delay)
        print("%s << %s << End Thread" % (self.system_name, self.class_name))
