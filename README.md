# IN4388
Repository containing Arduino and server code for Adhoc Networking course (2016-17)

Arduino directory contains the working Arduino code.
Server directory contains TCP server code 

Guidelines for hardware handling
================================

You have three options to power up the robot/circuit board: 

(1)Power bank that can be charged using mobile chargers 

(2) four 1.5V AA batteries 

(3) 5V DC adapter (which will be given with third robot) 



NOTE: Whenever you get a new robot, please make sure that the ID is set properly. 
Else, ESP8266 will drop the packet because of the connection matrix. Use 5V 1A 
chargers to charge the power bank. Another option would be to charge it from 
Laptop/PC USB

Arduino library for Queue to execute received commands one after the other:

http://playground.arduino.cc/Code/QueueList
http://playground.arduino.cc/Code/QueueArray


Note: Implement Queue only for movement commands such as move forward, move forward 
for time, move backward, turn left. For commands such as getdistance, getrssi, it 
is better to reply immediately so that the requested node doesn't have to wait 
for reply.
