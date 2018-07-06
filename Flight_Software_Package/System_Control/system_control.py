from Common.FSW_Common import *
from Serial_Communication.serial_communication import SerialCommunication

class SystemControl(FlightSoftwareParent):
    """
    This class is designed to handle system control on the RMC 549 Balloon(s).

    Written by Daniel Letros, 2018-07-03
    """

    def __init__(self, logging_object: Logger, serial_object: SerialCommunication) -> None:
        self.main_delay        = 0.05
        self.buffering_delay   = 0.05
        self.cutoff_pin_bcm    = 18
        self.cutoff_time_high  = 5
        self.cutoff_conditions = dict()
        self.cutoff_conditions['gps_altitude'] = [30]
        self.cutoff_conditions['gps_lat']      = [22, 23]
        self.cutoff_conditions['gps_lon']      = [24, 25]
        self.cutoff_conditions['time']         = ["12:34"]
        super().__init__("SystemControl", logging_object)
        self.serial_object = serial_object

        self.board_ID    = None
        self.data_header = None

        # Rewrite time cutoff condition as a datetime object
        temp_time = datetime.datetime.utcnow().strftime("%Y%m%d_")
        temp_time = temp_time + self.cutoff_conditions['time'][0].split(':')[0] + ":" + \
                    self.cutoff_conditions['time'][0].split(':')[1]
        self.cutoff_conditions['time'][0] = datetime.datetime.strptime(temp_time, "%Y%m%d_%H:%M")
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
        self.buffering_delay   = content['buffering_delay']
        self.cutoff_pin_bcm    = content['cutoff_BCM_pin_number']
        self.cutoff_conditions = content['cut_conditions']
        self.cutoff_time_high  = content['cutoff_time_high']

    def check_id_and_headers(self) -> None:
        """
        This function just looks into the log file for the most recent header and data logging lines.
        This might not be the most efficient way of doing it either, it could take a lot of time.

        :return: None
        """
        self.start_function_diagnostics("check_id_and_headers")
        found_id     = False
        found_header = False
        with open(self.logger.notifications_log_path, 'r') as f:
            content = f.readlines()
        for line_idx in range(len(content)-1, -1, -1):
            if found_id and found_header:
                break
            if content[line_idx].split("<<")[0].strip().lower() == 'id':
                found_id = True
                self.board_ID = content[line_idx].split("<<")[-1].strip()
            if content[line_idx].split("<<")[0].strip().lower() == 'header':
                found_header     = True
                self.data_header = content[line_idx].split("<<")[-1].strip()
        if not found_header:
            self.data_header = None
        if not found_id:
            self.board_ID = None
        self.end_function_diagnostics("check_id_and_headers")

    def check_uplink_commands(self) -> list:
        """
        This function checks for any un-processed uplink commands relevant to system control

        Written by Daniel Letros, 2018-07-06

        :return: Returns a list of the commands that need to be processed by this class
        """
        # Loop through list of uplink commands and check if any of them are useful to this class.
        # If they are delete them from main uplink list before use and then carry out the commands.
        # if no commands are left in main uplink list then set valid flag to false.
        # It is done this way to minimize time with mutex lock.
        self.start_function_diagnostics("check_uplink_commands")
        with self.serial_object.uplink_commands_mutex:
            commands_to_remove_and_use = []
            for command in self.serial_object.last_uplink_commands:
                if command.lower() == 'cut the mofo' or command.lower() == 'send header':
                    commands_to_remove_and_use.append(command)
            for command in commands_to_remove_and_use:
                self.serial_object.last_uplink_commands.remove(command)
            self.serial_object.last_uplink_seen_by_system_control = True
            self.end_function_diagnostics("check_uplink_commands")

        return commands_to_remove_and_use

    def check_auto_cutoff_conditions(self) -> bool:
        """
        This function will check for the automatic payload cutoff conditions.


        :return: True if payload should be cut
        """
        self.start_function_diagnostics("check_auto_cutoff_conditions")
        should_cut = False
        try:
            # Check Pi timestamp
            try:
                if (self.cutoff_conditions['time'][0] - datetime.datetime.utcnow()).total_seconds() <= 0:
                    should_cut = True
                    self.log_info("Cutting payload due to Pi time trigger.")
            except:
                self.log_error("Error checking for Pi timestamp payload cutoff")

            # Check GPS if Pi time says don't do it yet
            if self.data_header is not None and not should_cut:
                last_data_line = self.read_last_line_in_data_log().split(',')
                header_list    = self.data_header.split(',')
                col_count = 0
                for header in header_list:
                    if header == "UTC":
                        # Check GPS Timestamp
                        try:
                            # Example GPS timestamp: 230648.00
                            gps_HHMMSS = str(last_data_line[col_count])
                            # reformat into datetime
                            temp_time = datetime.datetime.utcnow().strftime("%Y%m%d_")
                            temp_time = temp_time + gps_HHMMSS[0:2] + ":" + \
                                        gps_HHMMSS[2:4] + ":" + gps_HHMMSS[4:6]
                            if (self.cutoff_conditions['time'][0] -
                                    datetime.datetime.strptime(temp_time, "%Y%m%d_%H:%M:%s")).total_seconds() <= 0:
                                should_cut = True
                                self.log_info("Cutting payload due to GPS time trigger.")
                        except:
                            self.log_error("Error checking for GPS timestamp payload cutoff")
                    elif header == "LtDgMn":
                        try:
                            min_sec_deg = float(last_data_line[col_count])
                            deci_deg    = int(min_sec_deg / 100) + (min_sec_deg - int(min_sec_deg/100) * 100)
                            if deci_deg >= np.max(self.cutoff_conditions['gps_lat']) or \
                                    deci_deg <= np.min(self.cutoff_conditions['gps_lat']):
                                should_cut = True
                                self.log_info("Cutting payload due to GPS latitude [%f] trigger." % deci_deg)
                        except:
                            self.log_error("Error checking for GPS latitude payload cutoff")
                    elif header == "LnDgMn":
                        try:
                            min_sec_deg = float(last_data_line[col_count])
                            deci_deg    = int(min_sec_deg / 100) + (min_sec_deg - int(min_sec_deg/100) * 100)
                            if deci_deg >= np.max(self.cutoff_conditions['gps_lon']) or \
                                    deci_deg <= np.min(self.cutoff_conditions['gps_lon']):
                                should_cut = True
                                self.log_info("Cutting payload due to GPS longitude [%f] trigger." % deci_deg)
                        except:
                            self.log_error("Error checking for GPS longitude payload cutoff")
                    elif header == "Alt":
                        try:
                            altitude        = float(last_data_line[col_count])
                            alt_units       = None
                            alt_units_count = 0
                            for header in header_list:
                                if header == 'Altu':
                                    alt_units = str(last_data_line[alt_units_count])
                                alt_units_count += 1
                            if alt_units == "M" and alt_units is not None:
                                # Convert to km
                                altitude /= 1000
                                if altitude >= self.cutoff_conditions['gps_altitude']:
                                    should_cut = True
                                    self.log_info("Cutting payload due to GPS altitude [%f]" % altitude)
                            else:
                                pass
                        except:
                            self.log_error("Error checking for GPS altitude cutoff")
                    col_count += 1
            else:
                pass
        except Exception as err:
            self.log_error("Error [%s]" % str(err))
        self.end_function_diagnostics("check_auto_cutoff_conditions")
        return should_cut

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
                self.check_id_and_headers()

                # Check for any uplink commands
                if self.serial_object.last_uplink_commands_valid:
                    commands_to_remove_and_use = self.check_uplink_commands()
                    for command in commands_to_remove_and_use:
                        if command.lower() == 'cut the mofo':
                            try:
                                self.log_info("Cutting payload form uplink command.")
                                GPIO.output(self.cutoff_pin_bcm, GPIO.HIGH)
                                time.sleep(self.cutoff_time_high)
                                GPIO.output(self.cutoff_pin_bcm, GPIO.LOW)
                            except Exception as err:
                                self.log_error("Could not cut payload with reported error [%s]" % str(err))

                        if command.lower() == 'send header':
                            if self.serial_object.ports_are_good:
                                for port in self.serial_object.port_list:
                                    with self.serial_object.serial_mutex:
                                        # send down the last known header file
                                        time.sleep(self.buffering_delay)
                                        self.serial_object.write_request_buffer.append([port, "TX{%s" % self.data_header])
                                        time.sleep(self.buffering_delay)
                                        self.serial_object.read_request_buffer.append([port, "TX"])
                                        time.sleep(self.buffering_delay)

                # Check for other automatic cutoff conditions based off of data line
                if self.check_auto_cutoff_conditions():
                    try:
                        self.log_info("Cutting payload form auto trigger.")
                        GPIO.output(self.cutoff_pin_bcm, GPIO.HIGH)
                        time.sleep(self.cutoff_time_high)
                        GPIO.output(self.cutoff_pin_bcm, GPIO.LOW)
                    except Exception as err:
                        self.log_error("Could not cut payload with reported error [%s]" % str(err))
            except Exception as err:
                self.log_error("Main function error [%s]" % str(err))
            time.sleep(self.main_delay)
        print("%s << %s << End Thread" % (self.system_name, self.class_name))
