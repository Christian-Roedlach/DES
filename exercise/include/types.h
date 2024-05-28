#ifndef TYPES_H
#define TYPES_H

//#ifndef __USE_POSIX199309
//    #define __USE_POSIX199309
//#endif //__USE_POSIX199309

#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <mutex>

typedef struct  __attribute__ ((__packed__)) {
    int64_t timestamp_sec;
    int64_t timestamp_nsec;
    uint32_t id;
    char control[4];
} message_t;

typedef uint64_t timestamp_t;

typedef struct  __attribute__ ((__packed__)) {
    timestamp_t timestamp;
    uint16_t msg_cnt;
    uint16_t crc;
} node_message_t;

typedef struct {
    timestamp_t timestamp = 0;
    std::mutex timestamp_mutex;
    // always lock all gpio_event variables !!
    bool gpio_event_registered = false;
    timestamp_t gpio_event_registered_timestamp = 0;
    std::mutex gpio_event_registered_mutex;
    bool time_synced = false;
    int errorstate = EXIT_SUCCESS;
} node_state_t;

typedef struct {
    uint32_t id;
    double roundtrip_time;
} data_cache_t;

#endif // TYPES_H
