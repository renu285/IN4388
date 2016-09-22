#ifndef __GAURD__
#define __GAURD__
/*
 * internal commands
 * 
 */
#define IC_1 0B001
#define ic_2 0b010
#define ic_3 0b011
#define ic_4 0b100
#define ic_5 0b101
#define ic_6 0b110
#define ic_7 0b111
#define tcp 0b0
#define adhoc 0b1
#define res1 0b000
#define fwd 0b0
#define ack 0b1
#define res2 0b000


#define ADHOC_UDP_PORT 8080
#define TCP_PORT 8089


/* Ultrasonic Sesors */

#define ULTRASONIC_LEFT 1 
#define ULTRASONIC_RIGHT 2 
#define ULTRASONIC_FRONT 3 



/* Packet format details */


#define START_BYTE 0xFF

#define PACKET_START_BYTE_LOC 0
#define PACKET_SRC_DST_LOC 1
#define PACKET_INTERMEDIATE_SRC_LOC 2
#define PACKET_INTERNAL_CMD_LOC 3
#define PACKET_COUNTER_HIGH_LOC 4
#define PACKET_COUNTER_LOW_LOC 5
#define PACKET_DATA_LENGTH_LOC 6
#define PACKET_DATA_LOC 7


/*
 * 
 * external commands
 */

#define MOVEFORWARD 0X01
#define MOVEFORWARD_TIME 0X02
#define MOVEFORWARD_DIST 0X0b
#define MOVE_REVERSE 0X03
#define MOVE_REVERSE_TIME 0X04
#define MOVE_REVERSE_DIST 0X0c
#define ROTATE_LEFT 0X05
#define ROTATE_RIGHT 0X06
#define STOP_BOT 0X07
#define GET_OBSTACLE_LEFT 0X08
#define GET_OBSTACLE_RIGHT 0X09
#define GET_OBSTACLE_FRONT 0X0A
#define GET_MAG_DATA 0x0D
#define GET_RSSI 0x0E
#define GET_ID 0x0F
#define SET_ID 0x10


/*
 * setting packet location
 * 
 */
#define START_BYTE_LOCATION 0
#define SRCDST_LOCATION 1
#define ISC_LOCATION 2
#define IC_LOCATION 3
#define CH_LOCATION 4
#define CL_LOCATION 5
#define LENGTH_LOCATION 6

#endif
