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


static int port = 12345;

struct message {
    uint64_t timestamp;
    uint16_t msg_cnt;
    uint16_t crc;
};

int main (int argc, char** argv)
{
    int retval = EXIT_FAILURE;
    int socket_descriptor;
    struct sockaddr_in address;
    struct message msg;
    uint16_t message_id = 0;
    struct timespec req = {0};
    req.tv_sec = 0;
    req.tv_nsec = 5 * 1000000L; // 5 ms

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
        // Get current timestamp
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        msg.timestamp = (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

        msg.msg_cnt = message_id++;

        // Calc CRC

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
        std::cout<< "msg"<<std::endl;
        // Sleep for 5 ms
        nanosleep(&req, NULL);
    }

    return retval;
}