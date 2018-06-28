import socket
import time

UDP_IP   = "0.0.0.0"
UDP_PORT = 55555

sock = socket.socket(socket.AF_INET,
                     socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

while True:
    data, addr = sock.recvfrom(2048)  # buffer size is 1024 bytes
    print(str(addr[0]) + "::" + str(UDP_PORT) + " << " + str(data.decode('utf-8')))
    time.sleep(3)
