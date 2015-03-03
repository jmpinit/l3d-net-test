#!/usr/bin/env python

import serial, socket

serialPort = '/dev/ttyACM0'
tcpPort = 23

# get the IP of the cube over serial
port = serial.Serial(serialPort, 9600, timeout=1)
ser.write("x") # request IP by sending any character
IP = port.readline()
port.close()

print "Received IP from cube:", IP

# connect to tcp server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((IP, serialPort))
s.send('hello world')
data = s.recv(1024)
s.close()

print "Received data from server:", data
