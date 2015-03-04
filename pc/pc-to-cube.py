#!/usr/bin/env python

import serial, socket
import random, string
import sys, time

if not len(sys.argv) == 3:
    print "must give # of packets, packet size (bytes)"
    sys.exit(1)

packetCount = int(sys.argv[1])
packetSize = int(sys.argv[2])
serialPort = '/dev/ttyACM0'
tcpPort = 23
timeout = 0.5 # seconds

if sys.platform == "win32":
    # On Windows, the best timer is time.clock()
    default_timer = time.clock
else:
    # On most other platforms the best timer is time.time()
    default_timer = time.time

while True:
    # get the IP of the cube over serial
    port = serial.Serial(serialPort, 115200, timeout=0.1)
    port.write(str(packetSize) + "\n")
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

packet = ''.join([random.choice(string.letters) for i in range(0, packetSize)])

dropped = []
timeTaken = []
for i in range(0, packetCount):
    startTime = default_timer()
    s.send(packet)

    dropped += [0]
    incoming = ' '
    while not incoming == 'y':
        incoming = s.recv(256)

        if default_timer() - startTime > timeout:
            s.send(' ')
            dropped[-1] += 1

    endTime = default_timer()
    timeTaken += [endTime - startTime]

print "average (ms):", (sum(timeTaken) / len(timeTaken)) * 1000

s.close()
