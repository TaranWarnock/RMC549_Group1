import socket
import time

time.sleep(15)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

message = "%s" % (socket.gethostname())

while True:
    sock.sendto(bytes(message, 'utf-8'), ('<broadcast>', 55555))
    time.sleep(3)

