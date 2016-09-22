#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>  
#include "CommandList.h" 
#include "AdhocServer.h"


/* 
 * Global variables to keep track of src and
 * dst ID 
 */

int bot_num = 0;
int src_id = 0;
int dst_id = 0;
uint16_t counter = 2;
char *BOT_ID;


/* 
 * 
 * Function to parse the commands
 * present in file and call
 * appropraite APIs
 *
 */

void parse_cmd(char command,char *line) {


  char *open_brace;
  char *arg_src;
  char *arg_dst;
  char *arg_value;

  open_brace = strchr(line,'(');

  if(!open_brace) {
    printf("Error Invalid command\n");
    return;
  }

  arg_src = open_brace + 1;
  arg_dst = arg_src + 2;
  arg_value = arg_src + 4;
  printf("Command called with args %c and %c\n",*arg_src,*arg_dst);

  switch(command) {

    case MOVEFORWARD :
      send_forward_time(atoi(arg_src),atoi(arg_dst),0);
      break;

    case MOVEFORWARD_TIME:
      send_forward_time(atoi(arg_src),atoi(arg_dst),atoi(arg_value));
      break;

    case MOVE_REVERSE:
      send_reverse_time(atoi(arg_src),atoi(arg_dst),0);
      break;

    case MOVE_REVERSE_TIME:
      send_reverse_time(atoi(arg_src),atoi(arg_dst),atoi(arg_value));
      break;

    case MOVEFORWARD_DIST:
      send_forward_dist(atoi(arg_src),atoi(arg_dst),atoi(arg_value));
      break;

    case MOVE_REVERSE_DIST:
      send_reverse_dist(atoi(arg_src),atoi(arg_dst),atoi(arg_value));
      break;

    case ROTATE_LEFT:
      send_rotate_left(atoi(arg_src),atoi(arg_dst),atoi(arg_value));
      break;

    case ROTATE_RIGHT:
      send_rotate_right(atoi(arg_src),atoi(arg_dst),atoi(arg_value));
      break;

    case STOP_BOT:
      stop_bot(atoi(arg_src),atoi(arg_dst));
      break;

    case GET_OBSTACLE_LEFT:
      get_obstacle_data(atoi(arg_src),atoi(arg_dst),1);
      break;

    case GET_OBSTACLE_RIGHT:
      get_obstacle_data(atoi(arg_src),atoi(arg_dst),2);
      break;

    case GET_OBSTACLE_FRONT:
      get_obstacle_data(atoi(arg_src),atoi(arg_dst),3);
      break;

    case GET_MAG_DATA:
      get_mag_data(atoi(arg_src),atoi(arg_dst));
      break;

    case GET_RSSI:
      get_RSSI(atoi(arg_src),atoi(arg_dst));
      break;

    default:
      printf("Unknown command detected\n");
      break;

  }

}

/* Function to read and
 * parse commands from 
 * file 
 */

void read_file() {

    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;


    fp = fopen("cmd_file.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        printf("%s", line);
        if(strstr(line,"send_forward_time")) {
            printf("Forward time called\n");
            parse_cmd(MOVEFORWARD_TIME,line);
        }
        if(strstr(line,"send_forward_dist")) {
            printf("Forward distance called\n");
            parse_cmd(MOVEFORWARD_DIST,line);
        }
        if(strstr(line,"send_reverse_time")) {
            printf("Forward time called\n");
            parse_cmd(MOVE_REVERSE_TIME,line);
        }
        if(strstr(line,"send_reverse_dist")) {
            printf("Forward distance called\n");
            parse_cmd(MOVE_REVERSE_DIST,line);
        }
        if(strstr(line,"send_rotate_left")) {
            parse_cmd(ROTATE_LEFT,line);
        }
        if(strstr(line,"send_rotate_right")) {
            parse_cmd(ROTATE_RIGHT,line);
        }
        if(strstr(line,"stop_bot")) {
            parse_cmd(STOP_BOT,line);
        }
        if(strstr(line,"get_obstacle_data")) {
            parse_cmd(GET_OBSTACLE_LEFT,line);
        }
        if(strstr(line,"get_mag_data")) {
            parse_cmd(GET_MAG_DATA,line);
        }
        if(strstr(line,"get_RSSI")) {
            parse_cmd(GET_RSSI,line);
        }
        if(strstr(line,"get_botID")) {
            parse_cmd(GET_ID,line);
        }
        if(strstr(line,"set_ID")) {
            parse_cmd(SET_ID,line);
        }

    }
    fclose(fp);
    if (line)
        free(line);
}

/* Function to extract data
 * from the packet */

char *get_data(char packet[]) {

    int i;
    char len = packet[PACKET_DATA_LENGTH_LOC];
    char *data = (char *)malloc(len * sizeof(char));
    for(i = 0 ; i < len ; i++) {
        data[i] = packet[PACKET_DATA_LOC + 1 + i];
    }
    return data;
}

long get_RSSI(int src,int dst) {

    int ret = 0;
    char *value = NULL;
    char data;
    int client_index = get_index(dst_id);
    
    data = GET_RSSI;
    create_packet(src,dst,sizeof(data),&data);
    memset(client_message,'\0',1024);
    
#ifdef __DEBUG__
    printf("Receving from sockfd - %d\n",client_sock[client_index]);
#endif
    ret = recv(client_sock[client_index] , client_message ,1024, 0);
    value = get_data(client_message);
#ifdef __DEBUG__
    printf("%x %x %x %x\n",value[0],value[1],value[2],value[3]);
#endif
    
    return (value[0] << 24 ) | (value[1] << 16) | (value[2] << 8) | (value[3]);
}

/* 
 * Function used to query the ID
 * of the bot as soon as it
 * connects to the server
 */

int get_botID(int bot) {

    int ret = 0;
    unsigned char *value = NULL;
    char data;
    char src = 0x00;
    data = GET_ID;
    create_packet_ID(sizeof(data),&data , bot);
    memset(client_message,'\0',1024);
    /* !! ALERT !! Here it should be client_sock[bot] itself !!! */
    ret = recv(client_sock[bot] , client_message ,1024,0);
    value = get_data(client_message);
#ifdef __DEBUG__
    int i;
    for(i = 0 ; i < ret; i++) {
      printf("0x%x ",client_message[i]);
    }
      printf("\n");
    printf("%x %x %x\n",value[0],value[1],value[2]);
#endif
    return value[0];
}



/* Might be required in the future */

void set_botID(int src,int dst,int botID) {

    int ret = 0;
    char *value = NULL;
    char data[2];
    data[0] = SET_ID;
    data[1] = botID;
    create_packet(src,dst,sizeof(data),data);
}

/* Function used to get magnetometer data from
 * the bot. 
 */
/* Might be required in the future */

int get_mag_data(int src,int dst) {

    int ret = 0;
    char *value = NULL;
    int client_index = get_index(3);
    char data;
    data = GET_MAG_DATA;
    create_packet(src,dst,sizeof(data),&data);
    memset(client_message,'\0',1024);
    
    ret = recv(client_sock[client_index] , client_message ,1024, 0);
    value = get_data(client_message);
    return value[1];
}

/* Function used to get obstacle sensor data from
 * the bot. 
 */

int get_obstacle_data(int src,int dst,int sensor_num) {

    int ret = 0;
    unsigned char *value = NULL;

    if (sensor_num == ULTRASONIC_LEFT) {
        char data;
        data = GET_OBSTACLE_LEFT;
        create_packet(src,dst,sizeof(data),&data);
    }
    if (sensor_num == ULTRASONIC_RIGHT) {
      char data = GET_OBSTACLE_RIGHT;
      create_packet(src,dst,sizeof(data),&data);
    }
    if (sensor_num == ULTRASONIC_FRONT) {
      char data = GET_OBSTACLE_FRONT;
      create_packet(src,dst,sizeof(data),&data);

    }
    memset(client_message,'\0',1024);
    int client_index = get_index(dst_id);
    ret = recv(client_sock[client_index] , client_message ,1024, 0);
   
    value = get_data(client_message);
#ifdef __DEBUG__
    int i;
    for(i = 0 ; i < ret; i++) {
      printf("0x%x ",client_message[i]);
    }
      printf("\n");
#endif
    return value[0];
}

/* Function to send stop command to the bot */

void stop_bot(int src,int dst) {

    char data = STOP_BOT;
    create_packet(src,dst,sizeof(data),&data);
}

/* Function to send rotate left command to the bot */
/* TBD Sanity  checks */
void send_rotate_left(int src,int dst,int time) {
    char data[2];
    data[0] = ROTATE_LEFT;
    data[1] = time;
    create_packet(src,dst,sizeof(data),data);
}

/* Function to send rotate right command to the bot */
void send_rotate_right(int src,int dst,int time) {
    char data[2];
    data[0] = ROTATE_RIGHT;
    data[1] = time;
    create_packet(src,dst,sizeof(data),data);
}

/* Function to send forward command to the bot */

void send_forward_time(int src,int dst,int time) {

    if(time == 0) {
        char data = MOVEFORWARD;
        create_packet(src,dst,sizeof(data),&data);
    }
    else {
        char data[2];
        data[0] = MOVEFORWARD_TIME;
        data[1] = time;
        create_packet(src,dst,sizeof(data),data);
    }

}

/* 
 * Function to send forward for a specific distance
 * to the bot 
 * Can be useful in future when Hall effect sensor
 * is available on the bot
 */

void send_forward_dist(int src,int dst,char dist) {

    if(dist == 0) {
        char data = MOVEFORWARD;
        create_packet(src,dst,sizeof(data),&data);
    }
    else {
        char data[2];
        data[0] = MOVEFORWARD_DIST;
        data[1] = dist;
        create_packet(src,dst,sizeof(data),data);
    }

}

/* 
 * Function to send reverse for a specific distance
 * to the bot 
 * Can be useful in future when Hall effect sensor
 * is available on the bot
 */
void send_reverse_time(int src,int dst,char time) {

    if(time == 0) {
        char data = MOVE_REVERSE;
        create_packet(src,dst,sizeof(data),&data);
    }
    else {
        char data[2];
        data[0] = MOVE_REVERSE_TIME;
        data[1] = time;
        create_packet(src,dst,sizeof(data),data);
    }

}
void send_reverse_dist(int src,int dst,char dist) {

    if(dist == 0) {
        char data = MOVE_REVERSE; 
        create_packet(src,dst,sizeof(data),&data);
    }
    else {
        char data[2];
        data[0] = MOVE_REVERSE_DIST;
        data[1] = dist;
        create_packet(src,dst,sizeof(data),data);
    }

}

/*
 *  Function to extract
 * socket fd for specific ID
 */


int get_index(int val) {

    int i = 0;
    for(i = 0 ; i < NUM_CONNECTIONS; i++) {
        
        if(BOT_ID[i] == val)
            return i;
    }

    return -1;

}


/* 
 * Function used to create and send the 
 * packet from the server to the selected bot
 */

void create_packet(int src,int dst, char length,char *data)
{
    int l,i,j;
    char checksum;
    char *packet;
    int client_index = get_index(dst);
    
    if(client_index == -1) {

        printf("Bot number not present in list\n");
        return;
    }
    
    packet=(char*)calloc(10 +length,sizeof(char));
    packet[0]= START_MARKER;
    packet[PACKET_START_BYTE_LOC + 1] = 0xFF;
    packet[PACKET_SRC_DST_LOC + 1] = src << 4 | dst ;
    packet[PACKET_INTERMEDIATE_SRC_LOC + 1] = src <<4; 
    packet[PACKET_INTERNAL_CMD_LOC + 1] = 0b000 << 5|0b1 << 4|0b0 << 3|0b000; 
    /* Forward packet and TCP set */
    //packet[4] = 0x10; 
    packet[PACKET_COUNTER_HIGH_LOC + 1] = counter << 8;
    packet[PACKET_COUNTER_LOW_LOC + 1] = counter & 0xFF;
    packet[PACKET_DATA_LENGTH_LOC + 1] = length;
    
    counter++;
    //memcpy(packet+7,&data,length);
    for(l=0;l<length;l++)
    {
        packet[PACKET_DATA_LOC + 1 + l] = *(data+l);

    }

    packet[10 + length - 1]= END_MARKER;
#ifdef __DEBUG__
    printf(" Sending to sockfd %d\n",client_sock[client_index]);
#endif
    send_cmd(client_sock[client_index],packet,10 + length);

    free(packet);
}

/* 
 * Function used to create and send the 
 * packet only during initialization as ID is not yet known 
 *
 */


void create_packet_ID(char length,char *data,int client_index)
{
    int l,i,j;
    char checksum;
    char *packet;
    
    packet=(char*)calloc(10 +length,sizeof(char));


    packet[0]= START_MARKER;
    packet[PACKET_START_BYTE_LOC + 1] = 0xFF;
    packet[PACKET_SRC_DST_LOC + 1] = 0x00;
    packet[PACKET_INTERMEDIATE_SRC_LOC + 1] = 0x00;
    packet[PACKET_INTERNAL_CMD_LOC + 1] = 0b000 << 5|0b1 << 4|0b0 << 3|0b000;
    /* Forward packet and TCP set */
    //packet[4] = 0x10; 
    packet[PACKET_COUNTER_HIGH_LOC + 1] = counter << 8;
    packet[PACKET_COUNTER_LOW_LOC + 1] = counter & 0xFF;
    packet[PACKET_DATA_LENGTH_LOC + 1] = length;
    counter++;
    for(l=0;l<length;l++)
    {
        packet[8+l] = *(data+l);

    }
    packet[10 + length - 1]= END_MARKER;
    send_cmd(client_sock[client_index],packet,10 + length);
    free(packet);
}

/* 
 * A wrapper for write() system call
 * to finally send the packet to bot
 */

int send_cmd(int client_sock, char *buf,int size) {

    int ret = 0;
    ret = write(client_sock,buf,size);
    return ret;
}

/* 
 * Function prints the packet contents in
 * hex format. Can be used for debugging
 */

void print_packet(char *buf,int size) {
    int i = 0;
    for(i = 0 ; i < size ; i++) {
        printf("%x ",buf[i]);
    }
    printf("\n");
}


