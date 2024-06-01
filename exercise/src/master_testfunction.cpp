#include "types.h"
#include "network.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <types.h>


static int port = 12345;


int main (int argc, char** argv)
{
    int retval = EXIT_FAILURE;
    int socket_descriptor;
    struct sockaddr_in address;
    node_message_t msg;
    uint16_t message_id = 0;

    socket_descriptor = socket (AF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor == -1) {
        perror ("socket()");
        exit (EXIT_FAILURE);
    }
    
    memset (&address, 0, sizeof (address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr ("224.0.0.1");
    address.sin_port = htons (port);
    printf ("Server ist bereit ...\n");
    /* Broadcasting beginnen */

    while (1) {
        // Calc CRC

        



        if(0 == (msg.timestamp%10) )
        {

            msg.msg_cnt = message_id++;

            uint16_t crc = CRC_INIT ; // Initial value
            uint8_t *data = (uint8_t*) &msg; 

            for (size_t i = 0; i < sizeof(node_message_t)-2; i++) {
            crc ^= (uint16_t)data[i] << 8;
            for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC_POLY;
            } else {
                crc = crc << 1;
            }
            }
            }
            msg.crc = crc;
            // Send the message
            if (sendto(socket_descriptor,
                   &msg,
                   sizeof(msg),
                   0,
                   (struct sockaddr *)&address,
                   sizeof(address)) < 0) {
            perror("sendto()");
            exit(EXIT_FAILURE);
            }
            std::cout << "msg_ID=" << msg.msg_cnt << ", Timestamp=" << msg.timestamp << ", CRC=" << msg.crc << std::endl;
            

        }
        // Sleep for 5 ms
        usleep(5000);
        msg.timestamp = msg.timestamp+1;
    }

    return retval;
}