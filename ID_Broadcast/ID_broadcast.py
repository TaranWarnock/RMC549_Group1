import socket
import time
import glob
import os
import os
import stat
import datetime as dt
import argparse
from pprint import pprint
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

    """
    gets a list of files sorted by modified time

    keyword args:
    num_files -- the n number of files you want to print
    directory -- the starting root directory of the search

    """
    parser = argparse.ArgumentParser()
    parser.add_argument('-n',
                        '--number',
                        help='number of items to return',
                        type=int,
                        default=2)
    parser.add_argument('-d',
                        '--directory',
                        help='specify a directory to search in',
                        default=path)

    args = parser.parse_args()
    num_files, directory = args.number, args.directory

    modified = []
    rootdir = os.path.join(os.getcwd(), directory)

    for root, sub_folders, files in os.walk(rootdir):
        for file in files:
            try:
                unix_modified_time = os.stat(os.path.join(root, file))[stat.ST_MTIME]
                human_modified_time = dt.datetime.fromtimestamp(unix_modified_time).strftime('%Y-%m-%d %H:%M:%S')
                filename = os.path.join(root, file)
                if ".txt" == filename[-4:]:
                    modified.append((human_modified_time, filename))
            except:
                pass

    modified.sort(key=lambda a: a[0], reverse=True)
    # pprint(modified[:num_files])
    return modified[:num_files]

path_to_log_files = r'/home/pi/RMC549Repos/RMC549_Group1/Flight_Software_Package/logs'
note_message      = ""
data_message      = ""
while True:
    new_log_list = get_newest_data_file(path_to_log_files)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)


    # last_data = read_last_line_in_data_log(get_newest_data_file(path_to_log_files))
    # last_note = read_last_line_in_data_log(get_newest_notification_file(path_to_log_files))

    note_message = "%s<<%s" % (socket.gethostname(), new_log_list[0])

    sock.sendto(bytes(note_message, 'utf-8'), ('<broadcast>', 55555))
    time.sleep(0.3)

    data_message = "%s<<%s" % (socket.gethostname(), new_log_list[1])

    sock.sendto(bytes(data_message, 'utf-8'), ('<broadcast>', 55555))
    time.sleep(0.3)

    sock.close()
    time.sleep(0.2)