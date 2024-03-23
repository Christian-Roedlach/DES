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

typedef struct {
    struct timespec timestamp;
    uint32_t id;
    char control[4];
} message_t;

typedef struct {
    uint32_t id;
    double roundtrip_time;
} data_cache_t;

#endif // TYPES_H