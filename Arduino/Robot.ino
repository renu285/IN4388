#include <EEPROM.h>
#include "PacketSerial.h"

#define STARTBYTE 0xFF
#define PACKETSIZE 254
#define MINPACKET 8
#define TCP 1
#define ADHOC 0
#define FWD 0
#define ACK 1
#define ADDRESS 0
#define MOTOR_RIGHT_P  9
#define MOTOR_RIGHT_N  10
#define MOTOR_LEFT_P  3
#define MOTOR_LEFT_N  11
#define ULTRASONIC_FRONT_TRIGGER   4
#define ULTRASONIC_LEFT_TRIGGER   12
#define ULTRASONIC_RIGHT_TRIGGER   7
#define ULTRASONIC_FRONT_ECHO   5
#define ULTRASONIC_LEFT_ECHO   13
#define ULTRASONIC_RIGHT_ECHO   2
#define TIMER1COUNT 64286  //50Hz

//External commands, communicated with another robot (in Adhoc mode) or TCP
#define NOCOMMAND 0
#define MOVEFORWARD 0x01
#define MOVEFORWARDTIME 0x02
#define MOVEBACK 0x03
#define MOVEBACKTIME 0x04
#define TURNLEFT 0x05
#define TURNRIGHT 0x06
#define STOP 0x07
#define DISTANCELEFT 0x08
#define DISTANCERIGHT 0x09
#define DISTANCEFRONT 0x0A
#define GETMGN 0x0D 
#define GETID 0x0F

//Internal commands, communicated with ESP8266
#define INT_ID 0x01
#define INT_SSID_PWD 0x02
#define INT_MATRIX 0x03
#define INT_RSSI 0x04
#define INT_IP 0x05
#define INT_DEMO 0x06

#define NODECOUNT 12
char matrix[NODECOUNT][NODECOUNT]={{0,1,0,0,1,0,0,1,0,0,1,0},
                                   {1,0,1,1,0,0,1,0,0,1,0,0},
                                   {0,1,0,0,0,1,0,1,0,0,0,1},
                                   {0,1,0,0,1,0,0,1,0,0,0,1},
                                   {1,0,0,1,0,1,1,0,0,0,1,0},
                                   {0,0,1,0,1,0,0,0,1,0,0,1},
                                   {1,0,0,0,0,1,0,1,0,1,0,0},
                                   {0,1,0,0,1,0,1,0,1,0,0,1},
                                   {0,0,1,0,0,1,0,1,0,0,1,0},
                                   {1,0,0,1,0,0,1,0,0,0,1,0},
                                   {1,0,0,0,1,0,0,0,1,1,0,1},
                                   {0,1,0,1,0,1,0,1,0,0,1,0}};
char ssid[] = "wifi_ssid";
char password[] = "password";
char ip[] = {192,168,43,193};

char Command = 0;
long Rssi = 0;
unsigned long distance = 0;

char nodeID = 0;
unsigned char movementTime = 0;
unsigned char tempMovementTime = 0;

uint16_t PacketCounter = 0;
long RSSI_Value = 0;

PacketSerial packetSerial;

//Handle commands. USER CAN ADD MODE COMMANDS IF NECESSARY
void handleCommands(char src, char dst, char internal, char tcp, char fwd, char counterH, char counterL, char datalen, char command, char *data)
{

    char tempData[32] = {0};
    data = data + 1;
    switch(command)
    {
      case MOVEFORWARD : Command = MOVEFORWARD;
                         moveForward();
                         break;

      case MOVEFORWARDTIME: Command = MOVEFORWARDTIME;
                            moveForwardForTime(data);
                            break;

      case MOVEBACK: Command = MOVEBACK; 
                     moveBack();
                     break;
                     
      case MOVEBACKTIME: Command = MOVEBACKTIME; 
                         moveBackForTime(data);
                         break;
                         
      case STOP: Command = STOP; 
                 stopMotors();
                 break;
                
      case TURNLEFT: Command = TURNLEFT;
                     turnLeft(data);
                     break;

      case TURNRIGHT: Command = TURNRIGHT;
                      turnRight(data);
                      break;

      case DISTANCEFRONT: distance = getDistanceFront();
                          if(distance > 254)
                          {
                           distance = 254;
                          }
                          tempData[0] = command;
                          tempData[1] = distance & 0xFF;
                          tempData[2] = 0;
                          sendPacket(dst, src, internal, tcp, ACK, counterH, counterL, 2, tempData);
                          break;

      case DISTANCELEFT: distance = getDistanceLeft();
                         if(distance > 254)
                         {
                          distance = 254;
                         }
                         tempData[0] = command;
                         tempData[1] = distance & 0xFF;
                         tempData[2] = 0;
                         sendPacket(dst, src, internal, tcp, ACK, counterH, counterL, 2, tempData);
                         break;

      case DISTANCERIGHT: distance = getDistanceRight();
                          if(distance > 254)
                          {
                           distance = 254;
                          }
                          tempData[0] = command;
                          tempData[1] = distance & 0xFF;
                          tempData[2] = 0;
                          sendPacket(dst, src, internal, tcp, ACK, counterH, counterL, 2, tempData);
                          break;
                           
      case GETMGN: break;   

      case GETID: nodeID = getID();
                  tempData[0] = command;
                  tempData[1] = nodeID;
                  sendPacket(dst, src, internal, tcp, ACK, counterH, counterL, 2, tempData);
                  break; 
    }
  }

//Timer 1 interrupt service routine
ISR(TIMER1_OVF_vect)
{
 
 long time = millis();

 switch(Command)
 {

  case MOVEFORWARDTIME:
  case TURNLEFT:
  case TURNRIGHT:
  case MOVEBACKTIME: if(((millis()/1000) - tempMovementTime) >= movementTime)
                      {
                        stopMotors();  
                        Command = NOCOMMAND; 
                      }
                     break;
 }

 TCNT1 = TIMER1COUNT;
}

void initGPIO()
{
 pinMode(ULTRASONIC_FRONT_TRIGGER, OUTPUT);
 pinMode(ULTRASONIC_LEFT_TRIGGER, OUTPUT);
 pinMode(ULTRASONIC_RIGHT_TRIGGER, OUTPUT);
 pinMode(ULTRASONIC_FRONT_ECHO, INPUT);
 pinMode(ULTRASONIC_LEFT_ECHO, INPUT);
 pinMode(ULTRASONIC_RIGHT_ECHO, INPUT);

 pinMode(MOTOR_RIGHT_P, OUTPUT);
 pinMode(MOTOR_RIGHT_N, OUTPUT);
 pinMode(MOTOR_LEFT_P, OUTPUT);
 pinMode(MOTOR_LEFT_N, OUTPUT); 

 stopMotors();
}


void initTimer()
{
  noInterrupts();   
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = TIMER1COUNT;
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << TOIE1);
  interrupts();
}

//Get distance in cm from front ultrasonic sensor (In blocking mode)
unsigned long getDistanceFront()
{
 digitalWrite(ULTRASONIC_FRONT_TRIGGER, LOW); 
 delayMicroseconds(2); 

 digitalWrite(ULTRASONIC_FRONT_TRIGGER, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(ULTRASONIC_FRONT_TRIGGER, LOW);
 long duration = pulseIn(ULTRASONIC_FRONT_ECHO, HIGH);

 return duration/58.2;
}

//Get distance from left ultrasonic sensor (In blocking mode)
unsigned long getDistanceLeft()
{
 digitalWrite(ULTRASONIC_LEFT_TRIGGER, LOW); 
 delayMicroseconds(2); 

 digitalWrite(ULTRASONIC_LEFT_TRIGGER, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(ULTRASONIC_LEFT_TRIGGER, LOW);
 long duration = pulseIn(ULTRASONIC_LEFT_ECHO, HIGH);

 return duration/58.2;
}

//Get distance from right ultrasonic sensor (In blocking mode)
unsigned long getDistanceRight()
{
 digitalWrite(ULTRASONIC_RIGHT_TRIGGER, LOW); 
 delayMicroseconds(2); 

 digitalWrite(ULTRASONIC_RIGHT_TRIGGER, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(ULTRASONIC_RIGHT_TRIGGER, LOW);
 long duration = pulseIn(ULTRASONIC_RIGHT_ECHO, HIGH);

 return duration/58.2;
}

//Move forward
void moveForward()
{
  digitalWrite(MOTOR_LEFT_P,LOW);
  digitalWrite(MOTOR_LEFT_N,HIGH);
  digitalWrite(MOTOR_RIGHT_P,HIGH);
  digitalWrite(MOTOR_RIGHT_N,LOW);
}

//Stop movement
void stopMotors()
{
  digitalWrite(MOTOR_LEFT_P,LOW);
  digitalWrite(MOTOR_LEFT_N,LOW);
  digitalWrite(MOTOR_RIGHT_P,LOW);
  digitalWrite(MOTOR_RIGHT_N,LOW);
}

//Move forward for specific time (in seconds)
void moveForwardForTime(char *data)
{
  moveForward();
  movementTime = *data;
  tempMovementTime = millis()/1000;
}

//Move back for specific time (in seconds)
void moveBackForTime(char *data)
{
  moveBack();
  movementTime = *data;
  tempMovementTime = millis()/1000;
}

//Move back
void moveBack()
{
  digitalWrite(MOTOR_LEFT_P,HIGH);
  digitalWrite(MOTOR_LEFT_N,LOW);
  digitalWrite(MOTOR_RIGHT_P,LOW);
  digitalWrite(MOTOR_RIGHT_N,HIGH);
}

//Turn left for specific time (in seconds)
void turnLeft(char *data)
{
  digitalWrite(MOTOR_LEFT_P,LOW);
  digitalWrite(MOTOR_LEFT_N,HIGH);
  digitalWrite(MOTOR_RIGHT_P,LOW);
  digitalWrite(MOTOR_RIGHT_N,LOW);  
  movementTime = *data;
  tempMovementTime = millis()/1000;
}

//Turn right for specific time (in seconds)
void turnRight(char *data)
{
  digitalWrite(MOTOR_LEFT_P,LOW);
  digitalWrite(MOTOR_LEFT_N,LOW);
  digitalWrite(MOTOR_RIGHT_P,HIGH);
  digitalWrite(MOTOR_RIGHT_N,LOW); 
  movementTime = *data;
  tempMovementTime = millis()/1000; 
}

//Get RSSI from ESP8266
/* Get RSSI from ESP8266 with hinternal command INT_RSSI. 
   Once ESP8266 reads its RSSI, it replies to Arduino with 
   internal command INT_RSSI. Reception of RSSI is handled 
   in onPacket function
 */

void getRSSI()
{
  char data[1];
  sendPacket(nodeID, nodeID, INT_RSSI, TCP, FWD, 0, 0, 0, data);
  //RSSI value is updated in RSS_Value variable as soon as there is reply from ESP8266. This is implemented in OnPacket() function
}

//This is internal API used to enable demo mode in ESP8266. Demo mode should be enabled in all the robots to make it work
void enableDemo()
{
  char data[1];
  sendPacket(nodeID, nodeID, INT_DEMO , TCP, FWD, 0, 0, 0, data);
}

//Get ID of robot
char getID()
{
  return EEPROM.read(ADDRESS);
}

//Set ID of robot
void setID(char ID)
{
  EEPROM.write(ADDRESS, ID);
  delay(50);
  nodeID = getID();
}

//Send ID of robot to ESP8266
void sendID()
{
  nodeID = getID();
  sendPacket(nodeID,nodeID,INT_ID,TCP,FWD,0,0,1,&nodeID);
}

//Send connection matrix to ESP8266
void sendMatrix()
{
  nodeID = getID();
  sendPacket(nodeID,nodeID,INT_MATRIX,TCP,FWD,0,0,NODECOUNT,(char*)matrix[nodeID-1]);
}

//Send IP address of server to ESP8266
void sendIP()
{
  sendPacket(nodeID,nodeID,INT_IP,TCP,FWD,0,0,sizeof(ip),ip);
}

//Send AP SSID and password to ESP8266
void sendSSIDandPassword()
{
  char *ssid_pwd = (char*)calloc(strlen(ssid)+strlen(password)+2,sizeof(char));
  strcpy(ssid_pwd,ssid);
  int delimiterLoc = strlen(ssid);
  ssid_pwd[delimiterLoc] = 0xA9;
  strcat(ssid_pwd,password);
  sendPacket(nodeID,nodeID,INT_SSID_PWD,TCP,FWD,0,0,strlen(ssid_pwd),ssid_pwd);
  free(ssid_pwd);
}

//This function is called when data is received from serial port (from PacketSerial library)
void onPacket(const uint8_t* buffer, size_t size)
{
  unsigned char src, dst, internal, tcp, fwd, counterH, counterL, datalen, command, *data;
  nodeID = getID();
  if((buffer[0] != STARTBYTE))
  {
    return;
  }
  src = (buffer[1] >> 4) & 0x0F;
  dst = buffer[1] & 0x0F;
  internal = (buffer[3] >> 5) & 0x07;
  tcp = (buffer[3] >> 4) & 0x01;
  fwd = (buffer[3] >> 3) & 0x01;
  counterH = buffer[4];
  counterL = buffer[5];
  datalen = buffer[6];
  command = buffer[7];
  data = (buffer + 7);
  // Checksum is not calculated. Can be implemented if necessary
  
  //Check if the command is internal, especially get RSSI from ESP8266
  if(internal == INT_RSSI)
  {
    if(datalen != 4)
    {
      return;
    }
    //Update RSSI_Value variable with hlatest RSSI
    RSSI_Value = (buffer[7] << 24) | (buffer[8] << 16) | (buffer[9] << 8) | (buffer[10]);
    char temp[5];
    temp[0] = 0x08;
    temp[1] = RSSI_Value >> 24;
    temp[2] = RSSI_Value >> 16;
    temp[3] = RSSI_Value >> 8;
    temp[4] = RSSI_Value;
    sendPacket(0, 0, 0, TCP, ACK, counterH, counterL, 5, temp);
  }

  //Call callback function
  OnReceive(src, dst, internal, tcp, fwd, counterH, counterL, datalen, command, data);
}

//For internal use only
void sendPacket(char src, char dst, char internal, char isTCP, char isACK, char counterHigh, char counterLow, char dataLength, char *data)
{
 char packet[PACKETSIZE] = {0};
 int index = 0;
 char checksum = 0;
 nodeID = getID();
 
 packet[0] = STARTBYTE;
 packet[1] = (src<<4) | (dst & 0x0F);
 packet[2] = nodeID << 4;
 packet[3] = (internal << 5) | (isTCP << 4) | (isACK << 3);
 packet[4] = counterHigh;
 packet[5] = counterLow;
 packet[6] = dataLength;
 for(index=0; index<dataLength; index++)
 {
  packet[7+index] = data[index];
 }
// packet[7+index] = checksum;  // Checksum is not calculated. Can be implemented if necessary
 packetSerial.send(packet,7+index);
}

//Initial setup
void setup() 
{
//setID(0x0A);
nodeID = getID();
packetSerial.setPacketHandler(&onPacket);
packetSerial.begin(9600);
initGPIO();
initTimer();
delay(1000);
sendID();
delay(1000);
sendMatrix();
delay(1000);
sendIP();
delay(1000);
sendSSIDandPassword();

}

void loop() 
{
  packetSerial.update();
  
}
/** USER FUNCTION FOR AD-HOC NETWORKS COURSE. Create function and send over WiFi network
src -> ID of robot. Send nodeID variable here (This is don't care for TCP packet)
dst -> ID of robot to which you want to send the packet (This is don't care for TCP packet)
isTCP -> Set with macro TCP or ADHOC depending where you want to send
dataLength -> Length of data
data -> Data that has to be sent. The first byte is COMMAND and subsequent bytes are arguments
**/
void CreatePacket(char src, char dst, char isTCP, char dataLength, char *data)
{
  PacketCounter++;
  char counterLow = PacketCounter & 0xFF;
  char counterHigh = (PacketCounter >> 8) & 0xFF;
  sendPacket(src, dst, 0x00, isTCP, FWD, counterHigh, counterLow, dataLength, data);
  
}

/** USER FUNCTION FOR AD-HOC NETWORKS COURSE. This function is called when a packet is received from TCP or AD-HOC node
//void onPacket(const uint8_t* buffer, size_t size) calls this function after parsing the packet.
**/
void OnReceive(char src, char dst, char internal, char tcp, char fwd, char counterH, char counterL, char datalen, char command, char *data)
{

  //Execute commands if the command is from TCP OR if ID is equal to destination (in Ad-hoc mode)
  if(tcp == TCP || ((tcp == ADHOC) && (nodeID == dst)))
  { 
      handleCommands(src, dst, internal, tcp, fwd, counterH, counterL, datalen, command, data);
  }
}
=======
#include <EEPROM.h>
#include "PacketSerial.h"

#define STARTBYTE 0xFF
#define PACKETSIZE 254
#define MINPACKET 8
#define TCP 1
#define ADHOC 0
#define FWD 0
#define ACK 1
#define ADDRESS 0
#define MOTOR_RIGHT_P  9
#define MOTOR_RIGHT_N  10
#define MOTOR_LEFT_P  3
#define MOTOR_LEFT_N  11
#define ULTRASONIC_FRONT_TRIGGER   4
#define ULTRASONIC_LEFT_TRIGGER   12
#define ULTRASONIC_RIGHT_TRIGGER   7
#define ULTRASONIC_FRONT_ECHO   5
#define ULTRASONIC_LEFT_ECHO   13
#define ULTRASONIC_RIGHT_ECHO   2
#define TIMER1COUNT 64286  //50Hz

//External commands, communicated with another robot (in Adhoc mode) or TCP
#define NOCOMMAND 0
#define MOVEFORWARD 0x01
#define MOVEFORWARDTIME 0x02
#define MOVEBACK 0x03
#define MOVEBACKTIME 0x04
#define TURNLEFT 0x05
#define TURNRIGHT 0x06
#define STOP 0x07
#define DISTANCELEFT 0x08
#define DISTANCERIGHT 0x09
#define DISTANCEFRONT 0x0A
#define GETMGN 0x0D 
#define GETID 0x0F

//Internal commands, communicated with ESP8266
#define INT_ID 0x01
#define INT_SSID_PWD 0x02
#define INT_MATRIX 0x03
#define INT_RSSI 0x04
#define INT_IP 0x05
#define INT_DEMO 0x06

#define NODECOUNT 12
char matrix[NODECOUNT][NODECOUNT]={{0,1,0,0,1,0,0,1,0,0,1,0},
                                   {1,0,1,1,0,0,1,0,0,1,0,0},
                                   {0,1,0,0,0,1,0,1,0,0,0,1},
                                   {0,1,0,0,1,0,0,1,0,0,0,1},
                                   {1,0,0,1,0,1,1,0,0,0,1,0},
                                   {0,0,1,0,1,0,0,0,1,0,0,1},
                                   {1,0,0,0,0,1,0,1,0,1,0,0},
                                   {0,1,0,0,1,0,1,0,1,0,0,1},
                                   {0,0,1,0,0,1,0,1,0,0,1,0},
                                   {1,0,0,1,0,0,1,0,0,0,1,0},
                                   {1,0,0,0,1,0,0,0,1,1,0,1},
                                   {0,1,0,1,0,1,0,1,0,0,1,0}};
char ssid[] = "wifi_ssid";
char password[] = "password";
char ip[] = {192,168,43,193};

char Command = 0;
long Rssi = 0;
unsigned long distance = 0;

char nodeID = 0;
unsigned char movementTime = 0;
unsigned char tempMovementTime = 0;

uint16_t PacketCounter = 0;
long RSSI_Value = 0;

PacketSerial packetSerial;

//Handle commands. USER CAN ADD MODE COMMANDS IF NECESSARY
void handleCommands(char src, char dst, char internal, char tcp, char fwd, char counterH, char counterL, char datalen, char command, char *data)
{

    char tempData[32] = {0};
    data = data + 1;
    switch(command)
    {
      case MOVEFORWARD : Command = MOVEFORWARD;
                         moveForward();
                         break;

      case MOVEFORWARDTIME: Command = MOVEFORWARDTIME;
                            moveForwardForTime(data);
                            break;

      case MOVEBACK: Command = MOVEBACK; 
                     moveBack();
                     break;
                     
      case MOVEBACKTIME: Command = MOVEBACKTIME; 
                         moveBackForTime(data);
                         break;
                         
      case STOP: Command = STOP; 
                 stopMotors();
                 break;
                
      case TURNLEFT: Command = TURNLEFT;
                     turnLeft(data);
                     break;

      case TURNRIGHT: Command = TURNRIGHT;
                      turnRight(data);
                      break;

      case DISTANCEFRONT: distance = getDistanceFront();
                          if(distance > 254)
                          {
                           distance = 254;
                          }
                          tempData[0] = command;
                          tempData[1] = distance & 0xFF;
                          tempData[2] = 0;
                          sendPacket(dst, src, internal, tcp, ACK, counterH, counterL, 2, tempData);
                          break;

      case DISTANCELEFT: distance = getDistanceLeft();
                         if(distance > 254)
                         {
                          distance = 254;
                         }
                         tempData[0] = command;
                         tempData[1] = distance & 0xFF;
                         tempData[2] = 0;
                         sendPacket(dst, src, internal, tcp, ACK, counterH, counterL, 2, tempData);
                         break;

      case DISTANCERIGHT: distance = getDistanceRight();
                          if(distance > 254)
                          {
                           distance = 254;
                          }
                          tempData[0] = command;
                          tempData[1] = distance & 0xFF;
                          tempData[2] = 0;
                          sendPacket(dst, src, internal, tcp, ACK, counterH, counterL, 2, tempData);
                          break;
                           
      case GETMGN: break;   

      case GETID: nodeID = getID();
                  tempData[0] = command;
                  tempData[1] = nodeID;
                  sendPacket(dst, src, internal, tcp, ACK, counterH, counterL, 2, tempData);
                  break; 
    }
  }

//Timer 1 interrupt service routine
ISR(TIMER1_OVF_vect)
{
 
 long time = millis();

 switch(Command)
 {

  case MOVEFORWARDTIME:
  case TURNLEFT:
  case TURNRIGHT:
  case MOVEBACKTIME: if(((millis()/1000) - tempMovementTime) >= movementTime)
                      {
                        stopMotors();  
                        Command = NOCOMMAND; 
                      }
                     break;
 }

 TCNT1 = TIMER1COUNT;
}

void initGPIO()
{
 pinMode(ULTRASONIC_FRONT_TRIGGER, OUTPUT);
 pinMode(ULTRASONIC_LEFT_TRIGGER, OUTPUT);
 pinMode(ULTRASONIC_RIGHT_TRIGGER, OUTPUT);
 pinMode(ULTRASONIC_FRONT_ECHO, INPUT);
 pinMode(ULTRASONIC_LEFT_ECHO, INPUT);
 pinMode(ULTRASONIC_RIGHT_ECHO, INPUT);

 pinMode(MOTOR_RIGHT_P, OUTPUT);
 pinMode(MOTOR_RIGHT_N, OUTPUT);
 pinMode(MOTOR_LEFT_P, OUTPUT);
 pinMode(MOTOR_LEFT_N, OUTPUT); 

 stopMotors();
}


void initTimer()
{
  noInterrupts();   
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = TIMER1COUNT;
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << TOIE1);
  interrupts();
}

//Get distance in cm from front ultrasonic sensor (In blocking mode)
unsigned long getDistanceFront()
{
 digitalWrite(ULTRASONIC_FRONT_TRIGGER, LOW); 
 delayMicroseconds(2); 

 digitalWrite(ULTRASONIC_FRONT_TRIGGER, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(ULTRASONIC_FRONT_TRIGGER, LOW);
 long duration = pulseIn(ULTRASONIC_FRONT_ECHO, HIGH);

 return duration/58.2;
}

//Get distance from left ultrasonic sensor (In blocking mode)
unsigned long getDistanceLeft()
{
 digitalWrite(ULTRASONIC_LEFT_TRIGGER, LOW); 
 delayMicroseconds(2); 

 digitalWrite(ULTRASONIC_LEFT_TRIGGER, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(ULTRASONIC_LEFT_TRIGGER, LOW);
 long duration = pulseIn(ULTRASONIC_LEFT_ECHO, HIGH);

 return duration/58.2;
}

//Get distance from right ultrasonic sensor (In blocking mode)
unsigned long getDistanceRight()
{
 digitalWrite(ULTRASONIC_RIGHT_TRIGGER, LOW); 
 delayMicroseconds(2); 

 digitalWrite(ULTRASONIC_RIGHT_TRIGGER, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(ULTRASONIC_RIGHT_TRIGGER, LOW);
 long duration = pulseIn(ULTRASONIC_RIGHT_ECHO, HIGH);

 return duration/58.2;
}

//Move forward
void moveForward()
{
  digitalWrite(MOTOR_LEFT_P,LOW);
  digitalWrite(MOTOR_LEFT_N,HIGH);
  digitalWrite(MOTOR_RIGHT_P,HIGH);
  digitalWrite(MOTOR_RIGHT_N,LOW);
}

//Stop movement
void stopMotors()
{
  digitalWrite(MOTOR_LEFT_P,LOW);
  digitalWrite(MOTOR_LEFT_N,LOW);
  digitalWrite(MOTOR_RIGHT_P,LOW);
  digitalWrite(MOTOR_RIGHT_N,LOW);
}

//Move forward for specific time (in seconds)
void moveForwardForTime(char *data)
{
  moveForward();
  movementTime = *data;
  tempMovementTime = millis()/1000;
}

//Move back for specific time (in seconds)
void moveBackForTime(char *data)
{
  moveBack();
  movementTime = *data;
  tempMovementTime = millis()/1000;
}

//Move back
void moveBack()
{
  digitalWrite(MOTOR_LEFT_P,HIGH);
  digitalWrite(MOTOR_LEFT_N,LOW);
  digitalWrite(MOTOR_RIGHT_P,LOW);
  digitalWrite(MOTOR_RIGHT_N,HIGH);
}

//Turn left for specific time (in seconds)
void turnLeft(char *data)
{
  digitalWrite(MOTOR_LEFT_P,LOW);
  digitalWrite(MOTOR_LEFT_N,HIGH);
  digitalWrite(MOTOR_RIGHT_P,LOW);
  digitalWrite(MOTOR_RIGHT_N,LOW);  
  movementTime = *data;
  tempMovementTime = millis()/1000;
}

//Turn right for specific time (in seconds)
void turnRight(char *data)
{
  digitalWrite(MOTOR_LEFT_P,LOW);
  digitalWrite(MOTOR_LEFT_N,LOW);
  digitalWrite(MOTOR_RIGHT_P,HIGH);
  digitalWrite(MOTOR_RIGHT_N,LOW); 
  movementTime = *data;
  tempMovementTime = millis()/1000; 
}

/* 
 * Get RSSI from ESP8266 with internal command INT_RSSI. 
 * Once ESP8266 reads its RSSI, it replies to Arduino with 
 * internal command "INT_RSSI". Reception of RSSI is handled 
 * in onPacket function
 */
 
void getRSSI()
{
  char data[1];
  sendPacket(nodeID, nodeID, INT_RSSI, TCP, FWD, 0, 0, 0, data);
  //RSSI value is updated in RSS_Value variable as soon as there is reply from ESP8266. This is implemented in OnPacket() function
}

//This is internal API used to enable demo mode in ESP8266. Demo mode should be enabled in all the robots to make it work
void enableDemo()
{
  char data[1];
  sendPacket(nodeID, nodeID, INT_DEMO , TCP, FWD, 0, 0, 0, data);
}

//Get ID of robot


 
char getID()
{
  return EEPROM.read(ADDRESS);
}

//Set ID of robot
void setID(char ID)
{
  EEPROM.write(ADDRESS, ID);
  delay(50);
  nodeID = getID();
}

//Send ID of robot to ESP8266
void sendID()
{
  nodeID = getID();
  sendPacket(nodeID,nodeID,INT_ID,TCP,FWD,0,0,1,&nodeID);
}

//Send connection matrix to ESP8266
void sendMatrix()
{
  nodeID = getID();
  sendPacket(nodeID,nodeID,INT_MATRIX,TCP,FWD,0,0,NODECOUNT,(char*)matrix[nodeID-1]);
}

//Send IP address of server to ESP8266
void sendIP()
{
  sendPacket(nodeID,nodeID,INT_IP,TCP,FWD,0,0,sizeof(ip),ip);
}

//Send AP SSID and password to ESP8266
void sendSSIDandPassword()
{
  char *ssid_pwd = (char*)calloc(strlen(ssid)+strlen(password)+2,sizeof(char));
  strcpy(ssid_pwd,ssid);
  int delimiterLoc = strlen(ssid);
  ssid_pwd[delimiterLoc] = 0xA9;
  strcat(ssid_pwd,password);
  sendPacket(nodeID,nodeID,INT_SSID_PWD,TCP,FWD,0,0,strlen(ssid_pwd),ssid_pwd);
  free(ssid_pwd);
}

//This function is called when data is received from serial port (from PacketSerial library)
void onPacket(const uint8_t* buffer, size_t size)
{
  unsigned char src, dst, internal, tcp, fwd, counterH, counterL, datalen, command, *data;
  nodeID = getID();
  if((buffer[0] != STARTBYTE))
  {
    return;
  }
  src = (buffer[1] >> 4) & 0x0F;
  dst = buffer[1] & 0x0F;
  internal = (buffer[3] >> 5) & 0x07;
  tcp = (buffer[3] >> 4) & 0x01;
  fwd = (buffer[3] >> 3) & 0x01;
  counterH = buffer[4];
  counterL = buffer[5];
  datalen = buffer[6];
  command = buffer[7];
  data = (buffer + 7);
  // Checksum is not calculated. Can be implemented if necessary
  
  //Check if the command is internal, especially get RSSI from ESP8266
  if(internal == INT_RSSI)
  {
    if(datalen != 4)
    {
      return;
    }
    //Update RSSI_Value variable with hlatest RSSI
    RSSI_Value = (buffer[7] << 24) | (buffer[8] << 16) | (buffer[9] << 8) | (buffer[10]);
    char temp[5];
    temp[0] = 0x08;
    temp[1] = RSSI_Value >> 24;
    temp[2] = RSSI_Value >> 16;
    temp[3] = RSSI_Value >> 8;
    temp[4] = RSSI_Value;
    sendPacket(0, 0, 0, TCP, ACK, counterH, counterL, 5, temp);
  }

  //Call callback function
  OnReceive(src, dst, internal, tcp, fwd, counterH, counterL, datalen, command, data);
}

//For internal use only
void sendPacket(char src, char dst, char internal, char isTCP, char isACK, char counterHigh, char counterLow, char dataLength, char *data)
{
 char packet[PACKETSIZE] = {0};
 int index = 0;
 char checksum = 0;
 nodeID = getID();
 
 packet[0] = STARTBYTE;
 packet[1] = (src<<4) | (dst & 0x0F);
 packet[2] = nodeID << 4;
 packet[3] = (internal << 5) | (isTCP << 4) | (isACK << 3);
 packet[4] = counterHigh;
 packet[5] = counterLow;
 packet[6] = dataLength;
 for(index=0; index<dataLength; index++)
 {
  packet[7+index] = data[index];
 }
// packet[7+index] = checksum;  // Checksum is not calculated. Can be implemented if necessary
 packetSerial.send(packet,7+index);
}

//Initial setup
void setup() 
{
//setID(0x0A);
nodeID = getID();
packetSerial.setPacketHandler(&onPacket);
packetSerial.begin(9600);
initGPIO();
initTimer();
delay(1000);
sendID();
delay(1000);
sendMatrix();
delay(1000);
sendIP();
delay(1000);
sendSSIDandPassword();

}

void loop() 
{
  packetSerial.update();
  
}
/** USER FUNCTION FOR AD-HOC NETWORKS COURSE. Create function and send over WiFi network
src -> ID of robot. Send nodeID variable here (This is don't care for TCP packet)
dst -> ID of robot to which you want to send the packet (This is don't care for TCP packet)
isTCP -> Set with macro TCP or ADHOC depending where you want to send
dataLength -> Length of data
data -> Data that has to be sent. The first byte is COMMAND and subsequent bytes are arguments
**/
void CreatePacket(char src, char dst, char isTCP, char dataLength, char *data)
{
  PacketCounter++;
  char counterLow = PacketCounter & 0xFF;
  char counterHigh = (PacketCounter >> 8) & 0xFF;
  sendPacket(src, dst, 0x00, isTCP, FWD, counterHigh, counterLow, dataLength, data);
  
}

/** USER FUNCTION FOR AD-HOC NETWORKS COURSE. This function is called when a packet is received from TCP or AD-HOC node
//void onPacket(const uint8_t* buffer, size_t size) calls this function after parsing the packet.
**/
void OnReceive(char src, char dst, char internal, char tcp, char fwd, char counterH, char counterL, char datalen, char command, char *data)
{

  //Execute commands if the command is from TCP OR if ID is equal to destination (in Ad-hoc mode)
  if(tcp == TCP || ((tcp == ADHOC) && (nodeID == dst)))
  { 
      handleCommands(src, dst, internal, tcp, fwd, counterH, counterL, datalen, command, data);
  }
}
