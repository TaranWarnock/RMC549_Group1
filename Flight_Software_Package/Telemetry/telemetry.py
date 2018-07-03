from Common.FSW_Common import *

class Telemetry(FlightSoftwareParent):
    """
    This class is designed to handle all telemetry on the RMC 549 Balloon(s).

    Written by Daniel Letros, 2018-07-02
    """

    def __init__(self, logging_object: Logger) -> None:
        self.main_delay = 0.05
        super().__init__("Telemetry", logging_object)

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
        self.main_delay = content['main_delay']


    def run(self) -> None:
        """
        This function is the main loop of the telemetry for the RMC 549 balloon(s).
        Software states and software/hardware communication is handled here.

        Written by Daniel Letros, 2018-07-02

        :return: None
        """

        print("%s << %s << Starting Thread" % (self.system_name, self.class_name))
        while self.should_thread_run:
            try:
                pass
            except:
                pass
            time.sleep(self.main_delay)
        print("%s << %s << End Thread" % (self.system_name, self.class_name))