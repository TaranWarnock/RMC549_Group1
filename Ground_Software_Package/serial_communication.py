import sys
import glob
import serial
import time
import threading
import datetime
import os
img = ""
header_data = "dummy"

def find_serial_ports(baudrate, timeout, previous_port_list):
    """
    This function finds active serial ports and makes the serial connections. This function should
    only be run on startup or if something has changed with the connections.

    Code from : https://stackoverflow.com/questions/12090503/listing-available-com-ports-with-python

    Adapted by: Daniel Letros, 2018-06-27
    Adapted by: Curtis Puetz, 2018-07-03

    :param baudrate: buadrate of the connection
    :param timeout: Communication timeout
    :param previous_port_list: the previous port list (if used)
    :raises EnvironmentError: On unsupported platform
    :return: List of the serial ports available, the port of connection within the list
    """

    # Clear old connections if any.
    port_list = previous_port_list
    for port in port_list:
        port_list[port].close()
    port_list.clear()

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
            port_ret = port
        except (OSError, serial.SerialException):
            pass

    # Open ports
    for port in result:
        port_list[port] = serial.Serial(port=port, baudrate=baudrate,
                                             parity=serial.PARITY_NONE,
                                             stopbits=serial.STOPBITS_ONE,
                                             bytesize=serial.EIGHTBITS,
                                             timeout=timeout,
                                             writeTimeout=timeout)
    return port_list, port_ret

def readline_from_serial(port_list, port):
    """
    This function will read data on the port up to a EOL char and return it.

    Written by Daniel Letros, 2018-06-27
    Adapted by: Curtis Puetz, 2018-07-03

    :param port_list: the dictionary of the port
    :param port: port to do the communication over
    :return: None
    """
    try:
        new_data = port_list[port].readline().decode('utf-8').strip()
        if new_data is "":
            fail = "[%s] returned no data." % port
            print(fail); instantiate_and_write_to_log_files(fail)
            port_list[port].reset_input_buffer()
        elif new_data[0] == '{':
            new_data = new_data[1:]
            while not new_data[-1] == ",":
                new_data = new_data[:-1]
            new_data += 'RSSI'
            print(new_data); instantiate_and_write_to_log_files(new_data)
        else:
            print(new_data); instantiate_and_write_to_log_files(new_data)
    except:
        another_fail = "code to read line from serial failed"
        print(another_fail); instantiate_and_write_to_log_files(another_fail)

def write_to_serial(port_list, port, message):
    """
    This function will write data to the port during serial communication.

    :param port_list: the dictionary of the port
    :param port: The port for the serial communication
    :param message: The message/data to write
    :return: None
    """
    port_list[port].write(message.encode('utf-8'))

def instantiate_and_write_to_log_files(data_to_log):
    """
    This function makes the log files which data/error messages will be stored. Everyday a new logfile folder will
    be created and all log files for that day will be stored there.

    Written by Daniel Letros, 2018-06-27
    Adapted by: Curtis Puetz, 2018-07-03

    :param data_to_log: the message to log to the log file

    :return: None
    """
    # instantiate the log file
    timestamp = datetime.datetime.utcnow().strftime("%Y%m%d")
    log_file_path = r"C:\Users\puetz\Desktop\Telemtry_logs"
    log_file_path += os.sep + timestamp
    data_log_path = log_file_path + os.sep + timestamp + "_data.txt"
    if not os.path.exists(log_file_path):
        os.makedirs(log_file_path)
    if not os.path.exists(data_log_path):
        open(data_log_path, 'w').close()

    # write to the log file
    with open(data_log_path, 'a') as file:
        file.write(data_to_log+"\n")

def continuous_read_data(delay, new_port_list, new_port):
    '''
    function on one of two threads that continuously reads data lines from the arduino
    and suspends for time 'delay'

    Written by Curtis Puetz 2018-07-07

    :param delay: suspend time in the function so we arent checking for data much
    faster than it is being sent
    :param new_port_list: the dictionary of the port
    :param new_port: The port for the serial communication
    :return: None
    '''
    while True:
        readline_from_serial(new_port_list, new_port)
        time.sleep(delay)

def make_uplink_cmd(port_list, port):
    '''
    function on one of two threads that takes input from the user and if the input matches
    the 'command list' then the command string is sent to the Arduino and ultimately to
    the balloon.

    Written by Curtis Puetz 2018-07-07

    :param port_list: the dictionary of the port
    :param port: The port for the serial communication
    :return: None
    '''
    while True:
        the_input = input()
        if the_input == "cut the mofo":
            write_to_serial(port_list, port, "cut the mofo,"); instantiate_and_write_to_log_files("Uplink: cut the mofo")
        elif the_input == "send header":
            write_to_serial(port_list, port, "send header,"); instantiate_and_write_to_log_files("Uplink: send header")

'''
note to users: 

1) you must hard code in the location you want to SAVE your log files within the 
instantiate_and_write_to_log_files() function on the line:
log_file_path = r"C:\\Users\puetz\Desktop\Telemtry_logs"

2) the 'check_for_data_delay' should be a little bit faster than the rate of 
data sent from the arduino/balloon (right now is comes down about every 9 seconds
'''
if __name__ == "__main__":
    check_for_data_delay = 3  # note to self: go a little bit faster than the rate of data sent from the arduino
    port_list = dict()
    new_port_list, new_port = find_serial_ports(9600, 0, port_list)
    t1 = threading.Thread(target=continuous_read_data, args=(check_for_data_delay, new_port_list, new_port))
    t2 = threading.Thread(target=make_uplink_cmd, args=(new_port_list, new_port))
    t1.start()
    t2.start()
