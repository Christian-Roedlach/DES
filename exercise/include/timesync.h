#ifndef TIMESYNC_H
#define TIMESYNC_H

#include "types.h"
#include "network.h"
#include <cmath>
#include <iostream>

namespace drs_timesync
{

template<typename T>
T abs_diff(T a, T b) 
{
    return a > b ? a - b : b - a;
}

static inline int crc_check(node_message_t *message)
{
    #warning "TODO: to be implemented"
    return EXIT_SUCCESS;
}

static inline int sync_local_time(
        node_state_t *node_state,
        node_message_t *message)
{
    int retval = EXIT_FAILURE;
    timestamp_t previous_timestamp = 0;
    timestamp_t new_timestamp = 0;
    
    retval = crc_check(message);
    
    if (EXIT_SUCCESS == retval)
    {
        new_timestamp = message->timestamp + TIMESTAMP_DELAY_VALUE_TICKS;

        if (1 == node_state->time_synced)
        {   
            {
                std::lock_guard<std::mutex> lock(node_state->timestamp_mutex);
                previous_timestamp = node_state->timestamp;
                node_state->timestamp = new_timestamp;
            }
            /* mutex is released */
            if (TIMESTAMP_MAX_DEVIATION_VALUE_TICKS < abs_diff(new_timestamp, previous_timestamp))
            {
                #warning "TODO: Error handling has to be implemented";
                
                #if ERROR_LOGGING
                std::cout << "WARNING: deviation was too big: " << abs_diff(new_timestamp, previous_timestamp) << std::endl;
                #endif // ERROR_LOGGING
            }
            #if DEBUG_LOGGING
                std::cout << "INFO: time deviation was (system - master): " << previous_timestamp - new_timestamp << std::endl;
            #endif // DEBUG_LOGGING
        } 
        else
        {   
            {
                std::lock_guard<std::mutex> lock(node_state->timestamp_mutex);
                node_state->timestamp = new_timestamp;
                node_state->time_synced = 1;
            }
            /* mutex is released */
        }
    }

    return retval;
}

} // namespace drs_timesync

#endif // TIMESYNC_H