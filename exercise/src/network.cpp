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
#include "logging.h"
#include <error.h>

static inline errorstate_t recvfrom_error_handling(node_state_t *node_state);

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

errorstate_t recieve_multicast(
        node_state_t *node_state,
        nw_multicast_descriptor_t *descriptor)
{
    using namespace drs_timesync;

    errorstate_t retval = errSt_undefined;
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

    /* casting required as message length is size_t aka uint64_t, but error value of recvfrom is -1 */
    if (-1 == static_cast<long int>(message_lenght))
    {
        #if ERROR_LOGGING
        std::cerr << "ERROR: recvfrom FAILED: " << strerror(errno); // endl is done by thread
        #endif // ERROR_LOGGING
        retval = recvfrom_error_handling(node_state);
    } 
    else if (message_length_expected != message_lenght)
    {
        #if ERROR_LOGGING
        std::cerr << "ERROR: recvfrom FAILED: mismatch of message lenght: expected " << 
                message_length_expected << 
                ", received: "<< message_lenght; // endl is done by thread
        #endif // ERROR_LOGGING
        retval = errSt_retry;
    } 
    else
        retval = errSt_running;

    if (errSt_running == retval)
    {
        if (EXIT_SUCCESS == sync_local_time(node_state, &descriptor->message_rcv))
            retval = errSt_running;
        else
            retval = errSt_retry;

    #if (DEBUG_LOGGING)
        std:: cout << 
                "Received message: CRC=" << descriptor->message_rcv.crc <<
                ", MSG_CNT = " << descriptor->message_rcv.msg_cnt << 
                ", TIMESTAMP = " << descriptor->message_rcv.timestamp << std::endl;
    #endif // DEBUG_LOGGING
    }
    
    return retval;
}

void thread_receive(
	node_state_t *node_state,
    nw_multicast_descriptor_t *descriptor)
{
    uint32_t error_count = 0;
    errorstate_t retval = errSt_undefined;
    const uint32_t error_count_out_of_sync = (TIMEOUT_OUT_OF_SYNC_MS * 1000) / TIMEOUT_US;

    while(errSt_restart > get_errorstate(node_state))
    {
        if (RECEIVE_ERROR_COUNT_MAX > error_count)
        {
            retval = recieve_multicast(node_state, descriptor);
            if (errSt_running == retval)
            {
                /* reset error_count and program restart count on successful sync */
                error_count = 0;
                node_state->restart_error_count = 0;    
            }
            else
            {
                error_count++;
                #if ERROR_LOGGING
                std::cerr << ", error_count: " << error_count;
                #endif // ERROR_LOGGING
                if(error_count >= error_count_out_of_sync)
                {
                    {
                        std::lock_guard<std::mutex> lock(node_state->timestamp_mutex);
                        node_state->time_synced = 0;
                    }
                    /* mutex is released */
                    #if ERROR_LOGGING
                        std::cerr << ", TIME sync LOST!";
                    #endif // ERROR_LOGGING
                }
                #if ERROR_LOGGING
                std::cerr << std::endl;
                #endif // ERROR_LOGGING
            }
        } 
        else 
        {
            write_syslog("network: maximum number of receive timeout errors reached - restarting...", LOG_ERR);
            set_errorstate(node_state, errSt_restart);
            break;
        }
    }
}

static inline errorstate_t recvfrom_error_handling(node_state_t *node_state)
{
    std::string message = "recvfrom: STOP + disable service; errno: ";
    bool write_to_syslog = false;
    errorstate_t errorstate = errSt_undefined;

    message.append(std::to_string(errno));
    message.append(": ");
    message.append(strerror(errno));

    switch (errno)
    {
        /* A - STOP + syslog + deaktivieren von Service */
        case EBADF:
        case ECONNREFUSED:
        case EFAULT:
        case EINVAL:
        case ENOTSOCK:
        case EACCES:
        /* case BLOCK - unknown errorcode */

            message.append(" --> STOP + disable service");
            errorstate = srrSt_stop_disable_service;
            write_to_syslog = true;

            break;

        /* B - SEGFAULT + Erkennung von Restart und deaktivieren von Service */
        case ECONNABORTED:
        case EPERM:

            message.append(" --> SEGFAULT: disable service after restart");
            errorstate = errSt_segfault;
            write_to_syslog = true;

            break;

        /* Retry (in code) + delay + counter mit STOP nach zu vielen Versuchen */
        case EAGAIN:  /* timeout */
            errorstate = errSt_retry;
            write_to_syslog = false;
            break;

        /* E STOP + syslog + ohne deaktivieren von Service */
        case EBUSY:
        case EINTR: // os restart

            message.append(" --> STOP: leave service enabled");
            errorstate = errSt_stop_leave_service;

            break;

        /* D - Restart (durch Service) + counter mit STOP nach zu vielen Versuchen */
        /* wrong sender - how is that possible with broadcast? ERRNO not found... */
        default:
             message.append(" --> RESTART (by service)");  
            
    }

    if (errorstate > get_errorstate(node_state))
        set_errorstate(node_state, errorstate);

    if (write_to_syslog)
        write_syslog("recvfrom: STOP + disable service; errno: ", LOG_CRIT);
    
    return errorstate;
}

