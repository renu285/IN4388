#ifndef __HEADER__
#define __HEADER__ 

/* Number of bots */
unsigned char NUM_CONNECTIONS;

extern int bot_num;
extern int src_id;
extern int dst_id;

/* Global packet counter value */
extern uint16_t counter;

/* To maintain mapping between sockfd and bot ID */
char *BOT_ID;

/* To keep track of number of connections */
extern unsigned int con_count;

int *client_sock;

char cmd[32];
char client_message[1024];

#define START_MARKER 0x80
#define END_MARKER 0x81



/* Declerations */

char *get_data(char packet[]);
long get_RSSI(int src,int dst);
int get_botID(int bot);
void set_botID(int src,int dst,int botID);
int get_mag_data(int src,int dst);
int get_obstacle_data(int src,int dst,int sensor_num);
void stop_bot(int src,int dst);
void send_rotate_left(int src,int dst,int time);
void send_rotate_right(int src,int dst,int time);
void send_forward_time(int src,int dst,int time);
void send_forward_dist(int src,int dst,char dist);
void send_reverse_time(int src,int dst,char time);
void send_reverse_dist(int src,int dst,char dist);
int get_index(int val);
void create_packet(int src,int dst, char length,char *data);
void create_packet_ID(char length,char *data,int client_index);
int send_cmd(int client_sock, char *buf,int size);
void print_packet(char *buf,int size);
#endif
