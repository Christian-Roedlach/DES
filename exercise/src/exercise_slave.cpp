#include "types.h"
#include "lib.h"
#include "network.h"
#include "settings.h"
#include "lib-timer.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <thread>

#define USAGE "Usage:  \
./exercise_slave <Multicast Address> <Port Number>\n\n"


int parse_arguments (int argc, char** argv, nw_multicast_descriptor_t *nw_desc);


int main (int argc, char** argv)
{
    int retval = EXIT_FAILURE;
    node_state_t node_state;
    nw_multicast_descriptor_t nw_desc;

    setlinebuf(stdout);

    retval = parse_arguments(argc, argv, &nw_desc);
       
    if (EXIT_SUCCESS == retval)
        retval = socket_slave_multicast(&nw_desc);

    if (EXIT_SUCCESS == retval)
    {
        std::thread receive_thread(thread_receive, &node_state, &nw_desc);
        
        /* setting thread priority */
        sched_param sch_params;
        sch_params.sched_priority = THREAD_PRIORITY_RECEIVE; // 0=low to 99=high

        /* SHED_FIFO has priority (Real Time) - requires root priviledges!!! */
        if(pthread_setschedparam(receive_thread.native_handle(), SCHED_FIFO, &sch_params)) 
        {
            std::cerr << "Failed to set Thread scheduling : " << strerror(errno) << std::endl;
        }



        std::thread timer_thread(thread_timer, &node_state);
        
        /* setting thread priority */
        sched_param sch_params_timer;
        sch_params_timer.sched_priority = THREAD_PRIORITY_TIMER; // 0=low to 99=high

        /* SHED_FIFO has priority (Real Time) - requires root priviledges!!! */
        if(pthread_setschedparam(timer_thread.native_handle(), SCHED_FIFO, &sch_params_timer)) 
        {
            std::cerr << "Failed to set Thread scheduling : " << strerror(errno) << std::endl;
        }

        receive_thread.join();
    }
         
    if (-1 != nw_desc.socket_file_descriptor)
        retval = close(nw_desc.socket_file_descriptor);

    if (EXIT_SUCCESS != retval)
        fprintf(stderr, "Error occured: %s\n\n", strerror( errno ));

#if DEBUG_LOGGING
    std::cout << "Program exits with retval = " << retval << std::endl;
#endif // DEBUG_LOGGING
    return retval;
}

int parse_arguments (int argc, char** argv, nw_multicast_descriptor_t *nw_desc)
{
    int retval = EXIT_FAILURE;
    /* parse IP address and port */

    char all_addresses[] = "0.0.0.0";
    
    #if (!DEBUG_FIXED_ADDRESSES_ENABLED)
    if (argc == 3)
    {
        retval = set_socket_address(all_addresses, argv[2], &nw_desc->slave_nw_socket_addr);
        nw_desc->slave_multicast_grp_addr = argv[1];
    }
    #else // DEBUG_FIXED_ADDRESSES_ENABLED
        #warning "ATTENTION: DEBUG_FIXED_ADDRESSES_ENABLED is set!!!"
        char port[] = DEBUG_FIXED_ADDRESSES_PORT;
        char multicast_addr[] = DEBUG_FIXED_ADDRESSES_ADDR;
        retval = set_socket_address(all_addresses, port, &nw_desc->slave_nw_socket_addr);
        nw_desc->slave_multicast_grp_addr = multicast_addr;
        std::cout << "WARNING: DEBUG_FIXED_ADDRESSES_ENABLED is set: listening to multicast group on" << std::endl;
        std::cout << "\t --> " << multicast_addr << ":" << port << std::endl;
    #endif // DEBUG_FIXED_ADDRESSES_ENABLED
       
    if (EXIT_SUCCESS != retval)
    {
        fprintf(stderr, "Error occured: %s\n\n", strerror( errno ));
        fprintf(stderr, USAGE);
    }

    return retval;
}





