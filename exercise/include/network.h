#ifndef NETWORK_H
#define NETWORK_H

#include "types.h"
#include <netinet/in.h>
#include "time.h"
#include <sys/socket.h>
#include <sys/time.h>
#include <settings.h>

/* DEBUG option to set multicast address and port statically */
#define DEBUG_FIXED_ADDRESSES_ENABLED 1
#define DEBUG_FIXED_ADDRESSES_PORT "1234"
#define DEBUG_FIXED_ADDRESSES_ADDR "224.0.0.1"

typedef struct {
	message_t message_snd = {};
	message_t message_rcv = {};
	int socket_file_descriptor = -1;
	struct sockaddr_in master_nw_socket_addr = {};
	struct sockaddr_in slave_nw_socket_addr = {};
	std::string slave_multicast_grp_addr = "";
	struct sockaddr_in recv_nw_socket_addr = {};
	struct timeval timeout = {TIMEOUT_S, TIMEOUT_US};
	uint32_t nr_of_messages = 0;
	char *logfile_name = nullptr;
} nw_descriptor_t;

int socket_master(nw_descriptor_t *descriptor);
int set_socket_address(char *ip_address, char *port, 
        struct sockaddr_in *addr);
int send_and_receive_roundtrip(nw_descriptor_t *descriptor);
int recieve_and_send_roundtrip(nw_descriptor_t *descriptor);
int socket_slave(nw_descriptor_t *descriptor);
int socket_slave_multicast(nw_descriptor_t *descriptor);

#endif // NETWORK_H