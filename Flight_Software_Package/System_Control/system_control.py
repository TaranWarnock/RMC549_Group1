from Common.FSW_Common import *
from Serial_Communication.serial_communication import SerialCommunication

class SystemControl(FlightSoftwareParent):
    """
    This class is designed to handle system control on the RMC 549 Balloon(s).

    Written by Daniel Letros, 2018-07-03
    """

    def __init__(self, logging_object: Logger, serial_object: SerialCommunication) -> None:
        """
        Init of class.

        Written by Daniel Letros, 2018-07-03

        :param logging_object: Reference to logging object
        :param serial_object: Reference to serial object
        """
        self.main_delay        = 0.05                        # Main thread delay
        self.buffering_delay   = 0.05                        # A time delay for parts of code. Used mostly to debug.
        self.cutoff_pin_bcm    = 18                          # Pi pin in BCM layout which triggers the cutoff.
        self.cutoff_time_high  = 5                           # How long the cutoff trigger will remain high.
        # Declare some default cutoff conditions.
        self.cutoff_conditions = dict()
        self.cutoff_conditions['gps_altitude'] = [30]      # Altitude condition will cut if rises to or above level.
        self.cutoff_conditions['gps_lat']      = [22, 23]  # Latitude defines max/min limits which will cut if crossed.
        self.cutoff_conditions['gps_lon']      = [24, 25]  # Longitude defines max/min limits which will cut if crossed.
        self.cutoff_conditions['time']         = ["12:34"] # Clock time which will trigger a cut.

        super().__init__("SystemControl", logging_object)  # Run init of parent.
        self.serial_object = serial_object                 # Reference to serial object.

        self.board_ID    = None  # ID line from arduino board
        self.data_header = None  # Header line from arduino

        self.has_already_cut_payload = False # Keeps track if payload is cut.
        self.good_altitude_count     = 0     # Since altitude can be noisy there is a count of how many consecutive
                                             # altitude readings agree with being >= the cutoff limit. This keeps track
                                             # of that count.

        # Rewrite time cutoff condition as a datetime object instead of a string clock time.
        temp_time = datetime.datetime.utcnow().strftime("%Y%m%d_")
        temp_time = temp_time + self.cutoff_conditions['time'][0].split(':')[0] + ":" + \
                    self.cutoff_conditions['time'][0].split(':')[1]
        self.cutoff_conditions['time'][0] = datetime.datetime.strptime(temp_time, "%Y%m%d_%H:%M")
        try:
            # Configure the cutoff pin
            if self.system_name == 'MajorTom' or self.system_name == 'Rocky':
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
        This might not be the most efficient way of doing it, it could take a lot of time.

        Written by Daniel Letros, 2018-07-06

        :return: None
        """
        self.start_function_diagnostics("check_id_and_headers")
        # Keep track of found files.
        found_id     = False
        found_header = False
        # Read in whole file.
        with open(self.logger.notifications_log_path, 'r') as f:
            content = f.readlines()
        # Loop through file in reverse to get newest lines and check if found a header or ID file.
        for line_idx in range(len(content)-1, -1, -1):
            if found_id and found_header:
                # Found both, don't need to look anymore, break.
                break
            if content[line_idx].split("<<")[0].strip().lower() == 'id':
                # Found newest ID line, save it.
                found_id = True
                self.board_ID = content[line_idx].split("<<")[-1].strip()
            if content[line_idx].split("<<")[0].strip().lower() == 'header':
                # Found newest header line, save it.
                found_header     = True
                self.data_header = content[line_idx].split("<<")[-1].strip()
        # Declare header/ID none if not found.
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
                # "cut the mofo" = cut the payload.
                # "send header"  = ask the payload for list of data headers
                if command.lower() == 'cut the mofo' or command.lower() == 'send header':
                    commands_to_remove_and_use.append(command)
            for command in commands_to_remove_and_use:
                self.serial_object.last_uplink_commands.remove(command)
            self.serial_object.last_uplink_seen_by_system_control = True
            self.end_function_diagnostics("check_uplink_commands")

        return commands_to_remove_and_use

    def convert_NEMA_to_deci(self, nmea: str) -> float:
        """
        Converts GPS degrees/minutes/seconds to decimal degrees.

        Written by Daniel Letros, 2018-07-06

        :param dms: DMS value
        :return: Decimal value
        """
        self.start_function_diagnostics("convert_NEMA_to_deci")
        day  = int(float(nmea)/100)
        rest = (float(nmea) - (day*100))/60
        dec  = day + rest
        self.end_function_diagnostics("convert_NEMA_to_deci")
        return dec

    def check_auto_cutoff_conditions(self) -> bool:
        """
        This function will check for the automatic payload cutoff conditions.

        Written by Daniel Letros, 2018-07-06

        :return: True if payload should be cut
        """
        self.start_function_diagnostics("check_auto_cutoff_conditions")
        should_cut = False # declare false, will only be set to true if conditions is met.
        if self.system_name == 'MajorTom' or self.system_name == 'Rocky':
            # Check Pi timestamp against cutoff timestamp.
            try:
                if (self.cutoff_conditions['time'][0] - datetime.datetime.utcnow()).total_seconds() <= 0:
                    should_cut = True
                    self.log_info("Cutting payload due to Pi time trigger.")
            except Exception as err:
                self.log_error("Error checking for Pi timestamp payload cutoff [%s]" % str(err))

            # Check GPS conditions if Pi timestamp did not trigger it already.
            if self.data_header is not None and not should_cut and self.serial_object.ports_are_good:
                # Get a list of data and data header. Search through the data header to find where in the data line
                # the appropriate information is kept.
                last_data_line = self.read_last_line_in_data_log().split(',')
                header_list    = self.data_header.split(',')
                col_count      = 0
                have_gps_sat_lock = False  # This is a check to see if GPS data is good. The check is needed since the
                                           # GPS will store and report its last known good location if it does not
                                           # have a lock. If it does not have a lock then the invalid location is
                                           # ignored.
                for header in header_list:
                    if header == "Nsat":
                        # Check for good GPS as defined by 4 or more satellites being used.
                        try:
                            n_sat = float(last_data_line[col_count])
                            if n_sat >= 4:
                                have_gps_sat_lock = True
                            else:
                                have_gps_sat_lock = False
                                self.log_info("GPS has bad sat lock [%f]" % n_sat)
                        except Exception as err:
                            self.log_error("Error checking for GPS sat lock [%s]" % str(err))
                    elif header == "UTC":
                        # Check GPS timestamp against clock cutoff.
                        try:
                            gps_HHMMSS = str(last_data_line[col_count])
                            # reformat into datetime
                            temp_time = datetime.datetime.utcnow().strftime("%Y%m%d_")
                            temp_time = temp_time + gps_HHMMSS[0:2] + ":" + \
                                        gps_HHMMSS[2:4] + ":" + gps_HHMMSS[4:6]
                            if (self.cutoff_conditions['time'][0] -
                                    datetime.datetime.strptime(temp_time, "%Y%m%d_%H:%M:%S")).total_seconds() <= 0:
                                should_cut = True
                                self.log_info("Cutting payload due to GPS time trigger.")
                        except Exception as err:
                            self.log_error("Error checking for GPS timestamp payload cutoff [%s]" % str(err))
                    elif header == "LtDgMn":
                        # Check for latitude boundaries.
                        try:
                            deci_deg    = self.convert_NEMA_to_deci(str(last_data_line[col_count]))
                            if deci_deg >= np.max(self.cutoff_conditions['gps_lat']) or \
                                    deci_deg <= np.min(self.cutoff_conditions['gps_lat']):
                                should_cut = True
                                self.log_info("Cutting payload due to GPS latitude [%f,%f/%f] trigger." % (deci_deg,
                                                                                                           np.min(self.cutoff_conditions['gps_lat']),
                                                                                                           np.max(self.cutoff_conditions['gps_lat'])))
                        except Exception as err:
                            self.log_error("Error checking for GPS latitude payload cutoff [%s]" % str(err))
                    elif header == "LnDgMn":
                        # Check for longitude boundaries.
                        try:
                            deci_deg = self.convert_NEMA_to_deci(str(last_data_line[col_count]))
                            if deci_deg >= np.max(self.cutoff_conditions['gps_lon']) or \
                                    deci_deg <= np.min(self.cutoff_conditions['gps_lon']):
                                should_cut = True
                                self.log_info("Cutting payload due to GPS longitude [%f, %f/%f] trigger." % (deci_deg,
                                                                                                             np.min(self.cutoff_conditions['gps_lon']),
                                                                                                             np.max(self.cutoff_conditions['gps_lon'])))
                        except Exception as err:
                            self.log_error("Error checking for GPS longitude payload cutoff [%s]" % str(err))
                    elif header == "Alt":
                        # Check for altitude boundaries.
                        try:
                            altitude        = float(last_data_line[col_count])
                            alt_units       = None
                            alt_units_count = 0
                            for header in header_list:
                                # Check altitude units. Assume km if they are not defined as m.
                                if header == 'Altu':
                                    alt_units = str(last_data_line[alt_units_count])
                                alt_units_count += 1
                            if alt_units == "M":
                                # Convert to km if in m.
                                altitude /= 1000
                            if altitude >= self.cutoff_conditions['gps_altitude'][0]:
                                self.good_altitude_count += 1
                                if self.good_altitude_count >= 10:
                                    should_cut = True
                                self.log_info("Cutting payload due to GPS altitude [%f]" % altitude)
                            else:
                                self.good_altitude_count = 0
                        except Exception as err:
                            self.log_error("Error checking for GPS altitude cutoff [%s]" % str(err))
                    col_count += 1
                should_cut = should_cut and have_gps_sat_lock  # only have GPS cutoff if GPS data is good.
                if not have_gps_sat_lock:
                    self.good_altitude_count = 0
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
                self.check_id_and_headers()  # Update ID and header information

                # Check for any uplink commands
                if self.serial_object.last_uplink_commands_valid:
                    commands_to_remove_and_use = self.check_uplink_commands()
                    for command in commands_to_remove_and_use:
                        if command.lower() == 'cut the mofo':
                            try:
                                if (self.system_name == 'MajorTom' or self.system_name == 'Rocky') \
                                        and not self.has_already_cut_payload:
                                    # Ground said cut the payload so do it.
                                    self.log_info("Cutting payload form uplink command.")
                                    GPIO.output(self.cutoff_pin_bcm, GPIO.HIGH)
                                    time.sleep(self.cutoff_time_high)
                                    GPIO.output(self.cutoff_pin_bcm, GPIO.LOW)
                                    self.has_already_cut_payload = True
                            except Exception as err:
                                self.log_error("Could not cut payload with reported error [%s]" % str(err))

                        if command.lower() == 'send header':
                            if self.serial_object.ports_are_good:
                                for port in self.serial_object.port_list:
                                    with self.serial_object.serial_mutex:
                                        # send down the last known header file.
                                        time.sleep(self.buffering_delay)
                                        self.serial_object.write_request_buffer.append([port, "TX{%s" % self.data_header])
                                        time.sleep(self.buffering_delay)
                                        self.serial_object.read_request_buffer.append([port, "TX"])
                                        time.sleep(self.buffering_delay)

                # Check for other automatic cutoff conditions based off of data line.
                if self.check_auto_cutoff_conditions():
                    try:
                        if (self.system_name == 'MajorTom' or self.system_name == 'Rocky') \
                                and not self.has_already_cut_payload:
                            self.log_info("Cutting payload form auto trigger.")
                            GPIO.output(self.cutoff_pin_bcm, GPIO.HIGH)
                            time.sleep(self.cutoff_time_high)
                            GPIO.output(self.cutoff_pin_bcm, GPIO.LOW)
                            self.has_already_cut_payload = True
                    except Exception as err:
                        self.log_error("Could not cut payload with reported error [%s]" % str(err))
            except Exception as err:
                self.log_error("Main function error [%s]" % str(err))
            time.sleep(self.main_delay)
        print("%s << %s << End Thread" % (self.system_name, self.class_name))
