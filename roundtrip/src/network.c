#include "network.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int socket_master(nw_descriptor_t *descriptor) 
{
    int retval = EXIT_FAILURE;
    socklen_t timeout_len = sizeof(descriptor->timeout);

    // Creating socket file descriptor
	if ( (descriptor->socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
    {
		perror("Socket creation failed");
		retval = EXIT_FAILURE;
	} 
    else if (-1 == getsockopt(descriptor->socket_file_descriptor,
            SOL_SOCKET,
            SO_RCVTIMEO,
            (char *) &descriptor->timeout,
            &timeout_len))
    {
        perror("Setting socket option SO_RCVTIMEO failed");
		retval = EXIT_FAILURE;
    } 
    else if (-1 == getsockopt(descriptor->socket_file_descriptor,
            SOL_SOCKET,
            SO_SNDTIMEO,
            (void *) &descriptor->timeout,
            &timeout_len))
    {
        perror("Setting socket option SO_SNDTIMEO failed");
		retval = EXIT_FAILURE;
    } 
    else
    {
        retval = EXIT_SUCCESS;
    }

    return retval;    
}

int set_socket_address(char *ip_address, char *port, 
        struct sockaddr_in *addr)
{
    int retval = EXIT_FAILURE;
    int64_t parsed_port = strtol(port, NULL, 10);
    uint16_t port_u16 = 0;

    if (0L != parsed_port)
    {
        if(0 <= parsed_port && UINT16_MAX >= parsed_port)
        {
            port_u16 = (uint16_t) parsed_port;
            retval = EXIT_SUCCESS;
        }    
    }

    if (EXIT_SUCCESS == retval)
    {
        memset(addr, 0,
                sizeof(*addr));

        addr->sin_family = AF_INET; 
        addr->sin_port = htons(port_u16); 
    
        if (!inet_aton(ip_address, &addr->sin_addr))
            retval = EXIT_FAILURE;
    }    

    return retval;
}

int send_and_receive_roundtrip(nw_descriptor_t *descriptor)
{
    int retval = EXIT_FAILURE;
    ssize_t len = 0;
    socklen_t sockaddr_len = sizeof(struct sockaddr_in);

    len = sendto(descriptor->socket_file_descriptor,
        	(const char *) &descriptor->message_snd,
            sizeof(descriptor->message_snd),
            0,  // flag - on reply, use MSG_CONFIRM
            (const struct sockaddr *) &descriptor->slave_nw_socket_addr,
            sockaddr_len);
    
    if (sizeof(descriptor->message_snd) == len)
    {
        retval = EXIT_SUCCESS;
        /* maximum wait time defined in descriptor->timeout */
        len = recvfrom(descriptor->socket_file_descriptor,
                (char *) &descriptor->message_rcv,
                sizeof(descriptor->message_rcv),
				MSG_WAITALL, 
                (struct sockaddr *) &descriptor->recv_nw_socket_addr,
				&sockaddr_len);

        if (sizeof(descriptor->message_rcv) == len) 
            retval = EXIT_SUCCESS;
        else
            retval = EXIT_FAILURE;
    }    

    return retval;
}