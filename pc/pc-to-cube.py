#!/usr/bin/env python

import serial, socket

serialPort = '/dev/ttyACM0'
tcpPort = 23

while True:
    # get the IP of the cube over serial
    port = serial.Serial(serialPort, 115200, timeout=1)
    port.write("x") # request IP by sending any character
    IP = port.readline().strip()
    port.close()

    if len(IP) > 0:
        break

print "Received IP from cube:", IP

# connect to tcp server
print "Connecting to server..."
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((IP, tcpPort))
print "Done."

outgoing = 'hello world'

s.send(outgoing + '\n')
incoming = ' '
while not (incoming == '\n' or len(incoming) == 0):
    incoming = s.recv(1024);
    print incoming

s.close()
