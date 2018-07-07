import socket
import time
import glob
import os

time.sleep(15)

"""
This script is run automatically on start up. It will broadcast the Pi's ID, IP, and log/notification files.
"""


def read_last_line_in_data_log(path) -> str:
    file_name = path
    try:
        with open(file_name, 'rb') as f:
            f.seek(-2, os.SEEK_END)
            while f.read(1) != b'\n':
                f.seek(-2, os.SEEK_CUR)
            content = f.readline().decode()
    except:
        with open(file_name, 'rb') as f:
            content = f.readlines()[-1].decode()
    return content


def get_newest_data_file(path):
    list_of_files = glob.glob(path + os.sep + '*_data.txt')  # * means all if need specific format then *.csv
    latest_file = max(list_of_files, key=os.path.getctime)
    return latest_file


def get_newest_notification_file(path):
    list_of_files = glob.glob(path + os.sep + '*_notifications.txt')  # * means all if need specific format then *.csv
    latest_file = max(list_of_files, key=os.path.getctime)
    return latest_file


sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

path_to_log_files = r'../Flight_Software_Package/logs'

while True:
    last_data = read_last_line_in_data_log(get_newest_data_file(path_to_log_files))
    last_note = read_last_line_in_data_log(get_newest_notification_file(path_to_log_files))

    note_message = "%s<<%s" % (socket.gethostname(), last_note)

    sock.sendto(bytes(message, 'utf-8'), ('<broadcast>', 55555))
    time.sleep(0.3)

    data_message = "%s<<%s" % (socket.gethostname(), last_data)

    sock.sendto(bytes(message, 'utf-8'), ('<broadcast>', 55555))
    time.sleep(0.3)

