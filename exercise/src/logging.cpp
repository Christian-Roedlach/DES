#include "logging.h"
#include <fstream>
#include <iostream>
#include <string.h>

void thread_logging(node_state_t *node_state, nw_multicast_descriptor_t *nw_desc)
{
    int retval = EXIT_FAILURE;
    timestamp_t timestamp_to_log = 0;
    int sync_status_to_log = 0;
    bool log_pending = false;
    const useconds_t thread_sleep_time_us = (MICROTICK_NS / 1000) * MAKROTICK_MULT;

    /* open file to append to */
    std::fstream logfile(nw_desc->logfile_name, std::fstream::in | std::fstream::out | std::fstream::app);
    //std::ofstream logfile(nw_desc->logfile_name, std::ios::out);

    if (logfile.is_open())
        retval = EXIT_SUCCESS;
    else
    {
        std::cerr << "ERROR: failed to open logfile: " << nw_desc->logfile_name << std::endl;
        std::cerr << "errno: " << errno << ": " << strerror( errno ) << std::endl;
        retval = EXIT_FAILURE;
    }
    
    if (EXIT_SUCCESS == retval)
    {
        while (EXIT_SUCCESS == node_state->errorstate) 
        {
            {
                std::lock_guard<std::mutex> lock(node_state->gpio_event_registered_mutex);
                
                if (node_state->gpio_event_registered)
                {
                    timestamp_to_log = node_state->gpio_event_registered_timestamp;
                    sync_status_to_log = node_state->gpio_event_registered_sync_status;
                    log_pending = true;
                    /* reset evant_registered_flag */
                    node_state->gpio_event_registered = false;
                }
            }
            /* mutex is released */

            if (log_pending)
            {
                if (logfile.is_open())
                {
                    log_pending = false;
                    logfile << std::to_string(timestamp_to_log) << "," 
                            << std::to_string(sync_status_to_log) << std::endl;
                }
                else
                {
                    node_state->errorstate = EXIT_FAILURE;
                    std::cerr << "ERROR: logfile is not open anymore! --> " << nw_desc->logfile_name << std::endl;
                    std::cerr << "errno: " << errno << ": " << strerror( errno ) << std::endl;
                }
            }

            /* sleep for one macrotick */
            usleep(thread_sleep_time_us);  
        }
        #if DEBUG_LOGGING
            std::cout<< "THREAD: logging STOPPED!"<<std::endl;
        #endif // DEBUG_LOGGING
    }
    else
    {
        node_state->errorstate = EXIT_FAILURE;
        std::cerr << "ERROR: thread_logging not initialized!" << std::endl;
    }

    logfile.close();
}