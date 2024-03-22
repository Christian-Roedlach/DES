#ifndef NETWORK_H
#define NETWORK_H

#include "types.h"
#include <netinet/in.h>
#include "time.h"
#include <sys/socket.h>
#include <sys/time.h>

typedef struct {
	message_t message_snd;
	message_t message_rcv;
	int socket_file_descriptor;
	struct sockaddr_in master_nw_socket_addr;
	struct sockaddr_in slave_nw_socket_addr;
	struct sockaddr_in recv_nw_socket_addr;
	struct timeval timeout;
	uint32_t nr_of_messages;
} nw_descriptor_t;

int socket_master(nw_descriptor_t *descriptor);
int set_socket_address(char *ip_address, char *port, 
        struct sockaddr_in *addr);
int send_and_receive_roundtrip(nw_descriptor_t *descriptor);

#endif // NETWORK_H