#include "network.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include "settings.h"
#include "stdio.h"
#include <iostream>
#include "timesync.h"
#include "lib.h"

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

int recieve_multicast(
        node_state_t *node_state,
        nw_multicast_descriptor_t *descriptor)
{
    using namespace drs_timesync;

    int retval = EXIT_FAILURE;
    size_t message_lenght = 0;
    static const size_t message_length_expected = sizeof(node_message_t);
    socklen_t sockaddr_len = sizeof(struct sockaddr_in);
    

    /* Receive multicast messages */        
    message_lenght = recvfrom(
            descriptor->socket_file_descriptor,
            (char *) &descriptor->message_rcv,
            sizeof(descriptor->message_rcv),
			MSG_WAITALL, 
            (struct sockaddr *) &descriptor->recv_nw_socket_addr,
			&sockaddr_len);

    if (-1 == static_cast<long int>(message_lenght))
    {
        #if ERROR_LOGGING
        std::cerr << "ERROR: recvfrom FAILED: " << strerror(errno); // endl is done by thread
        #endif // ERROR_LOGGING
        retval = EXIT_FAILURE;
    } 
    else if (message_length_expected != message_lenght)
    {
        #if ERROR_LOGGING
        std::cerr << "ERROR: recvfrom FAILED: mismatch of message lenght: expected " << 
                message_length_expected << 
                ", received: "<< message_lenght; // endl is done by thread
        #endif // ERROR_LOGGING
        retval = EXIT_FAILURE;
    } 
    else
        retval = EXIT_SUCCESS;

    
    if (EXIT_SUCCESS == retval)
    {
        sync_local_time(node_state, &descriptor->message_rcv);
    #if (DEBUG_LOGGING)
        std:: cout << 
                "Received message: CRC=" << descriptor->message_rcv.crc <<
                ", MSG_CNT = " << descriptor->message_rcv.msg_cnt << 
                ", TIMESTAMP = " << descriptor->message_rcv.timestamp << std::endl;
    #endif // DEBUG_LOGGING
    }
    
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

int socket_slave_multicast(nw_multicast_descriptor_t *descriptor) 
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
        if (-1 == setsockopt(descriptor->socket_file_descriptor,
            SOL_SOCKET,
            SO_RCVTIMEO,
            (void *) &descriptor->timeout,
            sizeof(descriptor->timeout)))
        {
            perror("Setting socket option SO_RCVTIMEO failed");
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
        std::cout << "Slave relay listening to multicast messages on " <<
                std::endl << "\t --> " <<
                descriptor->slave_multicast_grp_addr <<
                ":" <<
                ntohs(descriptor->slave_nw_socket_addr.sin_port) <<
                std::endl;
    }

    return retval;    
}

void thread_receive(
	node_state_t *node_state,
    nw_multicast_descriptor_t *descriptor)
{
    uint32_t error_count = 0;
    int retval = EXIT_FAILURE;

#if DEBUG_LOGGING
    struct timespec timestamp_start = {};
    struct timespec timestamp_end = {};
    struct timespec timestamp_diff = {};
    double timestamp_diff_double = 0;

    retval = clock_gettime(CLOCK_MONOTONIC, &timestamp_start);
#endif // DEBUG_LOGGING

    while (RECEIVE_ERROR_COUNT_MAX > error_count)
    {
        retval = recieve_multicast(node_state, descriptor);
        if (EXIT_SUCCESS == retval)
            error_count = 0;
        else
        {
            error_count++;
            #if ERROR_LOGGING
            std::cerr << ", error_count: " << error_count << std::endl;
            #endif // ERROR_LOGGING
        }
    }

    node_state->errorstate = EXIT_FAILURE;

#if DEBUG_LOGGING
    retval = clock_gettime(CLOCK_MONOTONIC, &timestamp_end);

    if (EXIT_SUCCESS == retval)
    {
        timespec_diff(&timestamp_end, &timestamp_start, &timestamp_diff);
        timespec_to_double(&timestamp_diff, &timestamp_diff_double);
    }

    std::cout << "Thread execution time: " << timestamp_diff_double << " s" << std::endl;
#endif // DEBUG_LOGGING

}

