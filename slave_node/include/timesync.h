#ifndef TIMESYNC_H
#define TIMESYNC_H

#include "types.h"
#include "network.h"
#include <cmath>
#include <iostream>
#include "logging.h"

namespace drs_timesync
{

template<typename T>
T abs_diff(T a, T b) 
{
    return a > b ? a - b : b - a;
}



static inline int crc_check(node_message_t *message)
{
    int retval = EXIT_FAILURE;
    uint16_t crc = CRC_INIT ; // Initial value
    uint8_t *data = (uint8_t*) message; // 8 bytes timestamp + 2 bytes msc_cnt
    
    /* little endian conversion
    // Convert timestamp to little-endian byte order
    for (int i = 0; i < 8; i++) {
        data[i] = (message->timestamp >> (i * 8)) & 0xFF;
    }
    // Convert msc_cnt to little-endian byte order
    data[8] = message->msg_cnt & 0xFF;
    data[9] = (message->msg_cnt >> 8) & 0xFF;
    */

    /*
    // Directly copy timestamp and msc_cnt into the byte array
    for (int i = 0; i < 8; i++) {
        data[i] = ((uint8_t*)&message->timestamp)[i];

    }
    


    data[8] = ((uint8_t*)&message->msg_cnt)[0];
    data[9] = ((uint8_t*)&message->msg_cnt)[1];
    */
    
    // Calc CRC16
    for (size_t i = 0; i < sizeof(node_message_t)-2; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC_POLY;
            } else {
                crc = crc << 1;
            }
        }
    }
    
    // return 0 if checksumms match
    if(message->crc == crc)
    {
        #if DEBUG_LOGGING
                std::cout << "CRC check passed" << std::endl;
        #endif // DEBUG_LOGGING
       
        retval = EXIT_SUCCESS;
    }else
    {
        retval = EXIT_FAILURE;
        #if ERROR_LOGGING
                std::cerr << "CRC check failed" << std::endl;    
        #endif // ERROR_LOGGING
    }


    return retval;
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

            #if SET_MICROTICK_ON_MSG_RECEIVE
            {
                std::lock_guard<std::mutex> lock(node_state->microtick_mutex);
                node_state->microtick = SET_MICROTICK_ON_MSG_RECEIVE_TO;
            }
            /* mutex is released */
            #endif // SET_MICROTICK_ON_MSG_RECEIVE

            if (TIMESTAMP_MAX_DEVIATION_VALUE_TICKS < abs_diff(new_timestamp, previous_timestamp))
            {
                std::string message = "WARNING: deviation was too big: system - master = ";
                message.append(std::to_string(
                        static_cast<int64_t>(static_cast<__int128_t>(previous_timestamp) - new_timestamp)));
                write_syslog(message, LOG_WARNING);
                
                #if ERROR_LOGGING
                std::cout << "WARNING: deviation was too big: " << abs_diff(new_timestamp, previous_timestamp) << std::endl;
                #endif // ERROR_LOGGING
            }
            #if DEBUG_LOGGING
                std::cout << "INFO: time deviation was (system - master): " << 
                        static_cast<int64_t>(static_cast<__int128_t>(previous_timestamp) - new_timestamp) << std::endl;
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

            #if SET_MICROTICK_ON_MSG_RECEIVE
            {
                std::lock_guard<std::mutex> lock(node_state->microtick_mutex);
                node_state->microtick = SET_MICROTICK_ON_MSG_RECEIVE_TO;
            }
            /* mutex is released */
            #endif // SET_MICROTICK_ON_MSG_RECEIVE
        }
    }

    return retval;
}

} // namespace drs_timesync

#endif // TIMESYNC_H