/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>   
#include "AdhocServer.h"
#include "CommandList.h"


//#define __DEBUG__ 1

unsigned int con_count = 0;

int main(int argc , char *argv[])
{
    int socket_desc , c , read_size;
    struct sockaddr_in server , client;
    int cmd_val= 0; 
    int val;
    int  ret = 0;
    int i = 0;
    if (argc != 2) {
        printf("Please enter %s  <NUMBER OF BOTS>\n",argv[0]);
        exit(0);
    }
    NUM_CONNECTIONS = atoi(argv[1]);

    BOT_ID = (char *)malloc(NUM_CONNECTIONS * sizeof(char));
    memset(BOT_ID,0,NUM_CONNECTIONS);
    client_sock = (int *)malloc(NUM_CONNECTIONS * sizeof(int));
    memset(BOT_ID,0,NUM_CONNECTIONS * sizeof(int));

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("\n");
    puts("******************* Server control program (Adhoc networking course)  ******************************");
    puts("\n");

    puts("* Socket created");

    int option = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(TCP_PORT);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("* Binding done");


    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection

    while(con_count != NUM_CONNECTIONS) {

        c = sizeof(struct sockaddr_in);
        puts("* Waiting for bots to connect");

        //accept connection from an incoming client
        client_sock[con_count] = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_sock[con_count] < 0)
        {
            perror("accept failed");
            return 1;
        }
        printf("  - Accepted connection\n");
#ifdef __DEBUG__
        printf("Got new  sockfd %d\n",client_sock[con_count]);
#endif
        BOT_ID[con_count] = get_botID(con_count);
        printf("  - Bot with ID : <%d> Connected\n",BOT_ID[con_count]);
        con_count++;
    }
    printf("-----------------------------------------------------------\n");

    while(1) {

        printf("Enter Bot ID to send the packet\n");
        scanf("%d",&dst_id);
    
        printf("Enter the command(1-12) to the bot-%d : \n",dst_id);


        printf("  1. Move forward \n"); 
        printf("  2. Move forward for time in seconds \n"); 
        printf("  3. Move reverse \n"); 
        printf("  4. Move reverse for time in seconds \n"); 
        printf("  5. Move left time\n"); 
        printf("  6. Move right time\n"); 
        printf("  7. Stop the bot\n"); 
        printf("  8. Get obstacle distance left \n"); 
        printf("  9. Get obstacle distance right\n"); 
        printf(" 10. Get obstacle distance front\n"); 
        printf(" 11. Get RSSI value\n");
        printf(" 12. Get ID\n");
        printf(" 13. Execute commands from file (cmd_file.txt)\n");
        printf(" Waiting for user input : "); 

        scanf("%d",&cmd_val);        

        switch(cmd_val) {

            case 1:
                send_forward_time(src_id, dst_id,0);
                break;
            case 2:
                printf("Enter the time in seconds : \n");
                scanf("%d",&val);
                send_forward_time(src_id, dst_id, val);
                break;
            case 3:
                send_reverse_time(src_id,dst_id,0);
                break;
            case 4:
                printf("Enter the time in seconds : \n");
                scanf("%d",&val);
                send_reverse_time(src_id,dst_id,val);
                break;
            case 5:
                printf("Enter the time for left turn in seconds : \n");
                scanf("%d",&val);
                send_rotate_left(src_id,dst_id,val);
                break;
            case 6:
                printf("Enter the time for right movement : \n");
                scanf("%d",&val);
                send_rotate_right(src_id,dst_id,val);
                break;
            case 7:
                printf("Sending command to stop the bot\n");
                stop_bot(src_id,dst_id);   
                break;
            case 8:
                printf("Fetchng left obstacle sensor information  \n");
                printf("Left Obtacle sensor reading : %d\n",get_obstacle_data(src_id,dst_id,ULTRASONIC_LEFT));
                break;
            case 9:
                printf("Fetchng right  obstacle sensor information \n");
                printf("Right Obtacle sensor reading : %d\n",get_obstacle_data(src_id,dst_id,ULTRASONIC_RIGHT));
                break;
            case 10:
                printf("Fetchng front obstacle sensor information \n");
                printf("Front Obtacle sensor reading : %d\n",get_obstacle_data(src_id,dst_id,ULTRASONIC_FRONT));
                break;
            case 11:
                printf("Fetchng RSSI information \n");
                printf("RSSI reading : %ld\n",get_RSSI(src_id,dst_id));
                break;
            case 12:
                printf("Fetchng ID\n");
                printf("ID of the bot : %d\n",BOT_ID[bot_num]);
                break;
            case 13:
                read_file();
                break;
            default:
                printf("Unknown command received\n");
                break;

        }
        system("clear");
    }
    return 0;
}
