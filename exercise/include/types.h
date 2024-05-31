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

typedef uint64_t timestamp_t;

typedef struct  __attribute__ ((__packed__)) {
    timestamp_t timestamp;
    uint16_t msg_cnt;
    uint16_t crc;
} node_message_t;

typedef enum {
    errSt_running = 0,
    errSt_retry,
    errSt_restart,
    errSt_stop_leave_service,
    srrSt_stop_disable_service,
    errSt_segfault,
    errSt_undefined = -1,
} errorstate_t;

typedef struct {
    timestamp_t timestamp = 0;
    std::mutex timestamp_mutex;
    // always lock all gpio_event variables !!
    bool gpio_event_registered = false;
    timestamp_t gpio_event_registered_timestamp = 0;
    int gpio_event_registered_sync_status = 0;
    std::mutex gpio_event_registered_mutex;
    int time_synced = 0;
    errorstate_t errorstate = errSt_running;
} node_state_t;

typedef struct {
    uint32_t id;
    double roundtrip_time;
} data_cache_t;

#endif // TYPES_H
