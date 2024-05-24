#include "network.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include "settings.h"
#include "stdio.h"

int socket_master(nw_descriptor_t *descriptor) 
{
    int retval = EXIT_FAILURE;

    // Creating socket file descriptor
	if ( (descriptor->socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) 
    {
		perror("Socket creation failed");
		retval = EXIT_FAILURE;
	} 
    else if (bind(descriptor->socket_file_descriptor, 
            (struct sockaddr *) &descriptor->master_nw_socket_addr,
            sizeof (descriptor->master_nw_socket_addr)))
    {
        perror("Socket binding failed");
		retval = EXIT_FAILURE;
    }
    else if (-1 == setsockopt(descriptor->socket_file_descriptor,
            SOL_SOCKET,
            SO_RCVTIMEO,
            (void *) &descriptor->timeout,
            sizeof(descriptor->timeout)))
    {
        perror("Setting socket option SO_RCVTIMEO failed");
		retval = EXIT_FAILURE;
    } 
    else if (-1 == setsockopt(descriptor->socket_file_descriptor,
            SOL_SOCKET,
            SO_SNDTIMEO,
            (void *) &descriptor->timeout,
            sizeof(descriptor->timeout)))
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
    /* handle case of intentionally setting port to 0 (refer to man strtol) */
    else if (strncmp(port, "0", 2) == 0)
    {
        port_u16 = 0;
        retval = EXIT_SUCCESS;
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
            MSG_CONFIRM,  // flag - on reply, use MSG_CONFIRM
            (const struct sockaddr *) &descriptor->slave_nw_socket_addr,
            sockaddr_len);

    #if (DEBUG_LOGGING)
    fprintf(stdout, "Master transmitted: ID=%d control=%s ts_sec=%ld ts_nsec=%ld\n",
            descriptor->message_snd.id,
            descriptor->message_snd.control,
            descriptor->message_snd.timestamp_sec,
            descriptor->message_snd.timestamp_nsec);
    #endif // DEBUG_LOGGING
    
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

        #if (DEBUG_LOGGING)
        fprintf(stdout, "Master received:    ID=%d control=%s ts_sec=%ld ts_nsec=%ld\n",
            descriptor->message_snd.id,
            descriptor->message_snd.control,
            descriptor->message_snd.timestamp_sec,
            descriptor->message_snd.timestamp_nsec);
        #endif // DEBUG_LOGGING

        if (sizeof(descriptor->message_rcv) == len) 
            retval = EXIT_SUCCESS;
        else
            retval = EXIT_FAILURE;
    }    

    return retval;
}

int recieve_and_send_roundtrip(nw_descriptor_t *descriptor)
{
    int retval = EXIT_FAILURE;
    
    socklen_t sockaddr_len = sizeof(struct sockaddr_in);

	retval = EXIT_SUCCESS;
        
    recvfrom(descriptor->socket_file_descriptor,
            (char *) &descriptor->message_rcv,
            sizeof(descriptor->message_rcv),
			MSG_WAITALL, 
            (struct sockaddr *) &descriptor->recv_nw_socket_addr,
			&sockaddr_len);

    #if (DEBUG_LOGGING)
    fprintf(stdout, "Slave received:    ID=%d control=%s ts_sec=%ld ts_nsec=%ld\n",
            descriptor->message_rcv.id,
            descriptor->message_rcv.control,
            descriptor->message_rcv.timestamp_sec,
            descriptor->message_rcv.timestamp_nsec);
    #endif // DEBUG_LOGGING
	
	strncpy(descriptor->message_rcv.control, "ACK", sizeof(descriptor->message_rcv.control));
				
    sendto(descriptor->socket_file_descriptor,
        	(const char *) &descriptor->message_rcv,
            sizeof(descriptor->message_rcv),
            MSG_CONFIRM,  // flag - on reply, use MSG_CONFIRM
            (const struct sockaddr *) &descriptor->recv_nw_socket_addr,
            sockaddr_len);
    
    #if (DEBUG_LOGGING)
    fprintf(stdout, "Slave transmitted: ID=%d control=%s ts_sec=%ld ts_nsec=%ld\n",
            descriptor->message_rcv.id,
            descriptor->message_rcv.control,
            descriptor->message_rcv.timestamp_sec,
            descriptor->message_rcv.timestamp_nsec);
    #endif // DEBUG_LOGGING

    return retval;
}


int socket_slave(nw_descriptor_t *descriptor) 
{
    int retval = EXIT_FAILURE;

    // Creating socket file descriptor
	if ( (descriptor->socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) 
    {
		perror("Socket creation failed");
		retval = EXIT_FAILURE;
	} 
    else if (bind(descriptor->socket_file_descriptor, 
            (struct sockaddr *) &descriptor->slave_nw_socket_addr,
            sizeof (descriptor->slave_nw_socket_addr)))
    {
        perror("Socket binding failed");
		retval = EXIT_FAILURE;
    }
    else
    {
        fprintf(stdout, "Slave relay listening on port %d\n", 
                ntohs(descriptor->slave_nw_socket_addr.sin_port));
        retval = EXIT_SUCCESS;
    }

    return retval;    
}

int socket_slave_multicast(nw_descriptor_t *descriptor) 
{
    int retval = EXIT_FAILURE;
    struct ip_mreq multicast_request;

    // Creating socket file descriptor
	if ( (descriptor->socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0 ) 
    {
		perror("Socket creation failed");
		retval = EXIT_FAILURE;
	} 
    else 
        retval = EXIT_SUCCESS;
    
    if (EXIT_SUCCESS == retval)
    {
        /* Allow multiple processes to use selected port (avoid restart issues) */
        int value = 1;
        if (setsockopt ( 
                descriptor->socket_file_descriptor,
                SOL_SOCKET,
                SO_REUSEADDR,
                &value, sizeof (value)) < 0) 
        {
            perror ("setsockopt:SO_REUSEADDR");
            retval = EXIT_FAILURE;
        }
        else 
            retval = EXIT_SUCCESS;
    }

    if (EXIT_SUCCESS == retval)
    {
        if (bind(descriptor->socket_file_descriptor, 
                (struct sockaddr *) &descriptor->slave_nw_socket_addr,
                sizeof (descriptor->slave_nw_socket_addr)) < 0)
        {
            perror("Socket binding failed");
            retval = EXIT_FAILURE;
        }
        else
            retval = EXIT_SUCCESS;
    }

    if (EXIT_SUCCESS == retval)
    {
        /* Allow broadcasting messages on this machine */
        int value = 1;
        if (setsockopt ( 
                descriptor->socket_file_descriptor,
                IPPROTO_IP,
                IP_MULTICAST_LOOP,
                &value, sizeof (value)) < 0) 
        {
            perror ("setsockopt:IP_MULTICAST_LOOP");
            retval = EXIT_FAILURE;
        }
        else 
            retval = EXIT_SUCCESS;
    }

    if (EXIT_SUCCESS == retval)
    {
        /* Set up multicast address */
        multicast_request.imr_multiaddr.s_addr = inet_addr(descriptor->slave_multicast_grp_addr.c_str());
        multicast_request.imr_interface = descriptor->slave_nw_socket_addr.sin_addr;

        /* Join broadcast group */
        if (setsockopt ( 
                descriptor->socket_file_descriptor,
                IPPROTO_IP,
                IP_ADD_MEMBERSHIP,
                &multicast_request, sizeof (multicast_request)) < 0) 
        {
            perror ("setsockopt:IP_ADD_MEMBERSHIP");
            retval = EXIT_FAILURE;
        }
        else 
            retval = EXIT_SUCCESS;
    }

    if (EXIT_SUCCESS == retval)    
    {
        fprintf(stdout, "Slave relay listening on port %d\n", 
                ntohs(descriptor->slave_nw_socket_addr.sin_port));
    }

    return retval;    
}
