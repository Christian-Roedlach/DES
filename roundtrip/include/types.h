#ifndef TYPES_H
#define TYPES_H

#ifndef __USE_POSIX199309
    #define __USE_POSIX199309
#endif //__USE_POSIX199309

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    struct timespec timestamp;
    uint32_t id;
    char* control[4];
} message_t;

#endif // TYPES_H