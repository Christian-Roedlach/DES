#include "types.h"
#include "lib.h"
#include "network.h"
#include "settings.h"
#include "lib-timer.h"
#include "signal_pin.h"
#include "logging.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <thread>


#define USAGE "Usage:  \
./exercise_slave <Multicast Address> <Port Number>\n\n"


int parse_arguments (int argc, char** argv, nw_multicast_descriptor_t *nw_desc);
void setup_thread_priority(std::thread *thread, int priority, int type);


int main (int argc, char** argv)
{
    int retval = EXIT_FAILURE;
    node_state_t node_state;
    //node_state.time_synced = 1;
    nw_multicast_descriptor_t nw_desc;

    setlinebuf(stdout);

    syslog_program_start();

    retval = parse_arguments(argc, argv, &nw_desc);
       
    if (EXIT_SUCCESS == retval)
        retval = socket_slave_multicast(&nw_desc);

    if (EXIT_SUCCESS == retval)
    {
        std::thread timer_thread(thread_timer, &node_state);
        setup_thread_priority(&timer_thread, THREAD_PRIORITY_TIMER, SCHED_FIFO);  

        std::thread receive_thread(thread_receive, &node_state, &nw_desc);
        setup_thread_priority(&receive_thread, THREAD_PRIORITY_RECEIVE, SCHED_FIFO);  
        
        std::thread signal_pin_thread(thread_signal_pin, GPIO_PIN, &node_state);
        setup_thread_priority(&signal_pin_thread, THREAD_PRIORITY_SIGNAL_PIN, SCHED_FIFO);  
        
        std::thread logging_thread(thread_logging, &node_state, &nw_desc);
        setup_thread_priority(&logging_thread, THREAD_PRIORITY_LOGGING, SCHED_FIFO);  

        logging_thread.join();
        signal_pin_thread.join();
        timer_thread.join();
        receive_thread.join();
    }
         
    if (-1 != nw_desc.socket_file_descriptor)
        retval = close(nw_desc.socket_file_descriptor);

    if (EXIT_SUCCESS != retval)
        fprintf(stderr, "Error occured: %s\n\n", strerror( errno ));

    /* set program return value to corresponding enum value */
    retval = get_errorstate(&node_state);

#if DEBUG_LOGGING
    std::cout << "Program exits with node_state.errorstate = " << retval << std::endl;
#endif // DEBUG_LOGGING
    return retval;
}

int parse_arguments (int argc, char** argv, nw_multicast_descriptor_t *nw_desc)
{
    int retval = EXIT_FAILURE;
    /* parse IP address and port */

    char all_addresses[] = "0.0.0.0";
    
    #if (!DEBUG_FIXED_ADDRESSES_ENABLED)
    if (argc == 4)
    {
        retval = set_socket_address(all_addresses, argv[2], &nw_desc->slave_nw_socket_addr);
        nw_desc->slave_multicast_grp_addr = argv[1];

        /* parse number of messages */
        if (EXIT_SUCCESS == retval)
            nw_desc->logfile_name.assign(argv[3]);   
    }
    #else // DEBUG_FIXED_ADDRESSES_ENABLED
        #warning "ATTENTION: DEBUG_FIXED_ADDRESSES_ENABLED is set!!!"
        char port[] = DEBUG_FIXED_ADDRESSES_PORT;
        char multicast_addr[] = DEBUG_FIXED_ADDRESSES_ADDR;
        retval = set_socket_address(all_addresses, port, &nw_desc->slave_nw_socket_addr);
        nw_desc->slave_multicast_grp_addr = multicast_addr;
        std::cout << "WARNING: DEBUG_FIXED_ADDRESSES_ENABLED is set: listening to multicast group on" << std::endl;
        std::cout << "\t --> " << multicast_addr << ":" << port << std::endl;
        nw_desc->logfile_name.assign(DEBUG_FIXED_LOG_FILENAME);

    #endif // DEBUG_FIXED_ADDRESSES_ENABLED
       
    if (EXIT_SUCCESS != retval)
    {
        fprintf(stderr, "Error occured: %s\n\n", strerror( errno ));
        fprintf(stderr, USAGE);
    }

    return retval;
}

void setup_thread_priority(std::thread *thread, int priority, int type)
{
    /* setting thread priority */
    sched_param sch_params;
    sch_params.sched_priority = priority; // 0=low to 99=high

    /* SHED_FIFO has priority (Real Time) - requires root priviledges!!! */
    if(pthread_setschedparam(thread->native_handle(), type, &sch_params)) 
    {
        std::cerr << "Failed to set Thread scheduling : " << strerror(errno) <<
                " - root privileges required" << std::endl;
    }
}





