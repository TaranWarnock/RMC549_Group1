import socket
import time
import datetime

Look_At_Rocky    = True
Look_At_MajorTom = False

UDP_IP   = "0.0.0.0"
UDP_PORT = 55555

sock = socket.socket(socket.AF_INET,
                     socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

while True:
    data, addr = sock.recvfrom(32768)
    line = str(datetime.datetime.utcnow().strftime("%Y%m%d_%H:%M:%S.%f")) + "::" + \
           str(addr[0]) + "::" + str(UDP_PORT) + " << " + str(data.decode('utf-8'))
    if line.split(" << ")[1] == "MajorTom" and Look_At_MajorTom:
        print(line)
    if line.split(" << ")[1] == "Rocky" and Look_At_Rocky:
        print(line)
    time.sleep(0.1)
