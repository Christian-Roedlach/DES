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
#define DEBUG_FIXED_ADDRESSES_PORT "12345"
#define DEBUG_FIXED_ADDRESSES_ADDR "224.0.0.1"

typedef struct {
	node_message_t message_rcv = {};
	int socket_file_descriptor = -1;
	struct sockaddr_in slave_nw_socket_addr = {};
	std::string slave_multicast_grp_addr = "";
	struct sockaddr_in recv_nw_socket_addr = {};
	struct timeval timeout = {TIMEOUT_S, TIMEOUT_US};
	uint32_t nr_of_messages = 0;
	std::string logfile_name = "";
} nw_multicast_descriptor_t;

int set_socket_address(
		char *ip_address, 
		char *port, 
        struct sockaddr_in *addr);
errorstate_t recieve_multicast(
		node_state_t *node_state, 
		nw_multicast_descriptor_t *descriptor);
int socket_slave_multicast(nw_multicast_descriptor_t *descriptor);
void thread_receive(
	node_state_t *node_state,
    nw_multicast_descriptor_t *descriptor);


#endif // NETWORK_H