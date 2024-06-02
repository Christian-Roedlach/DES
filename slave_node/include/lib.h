#ifndef LIB_H
#define LIB_H

#include "types.h"

/**
 * @fn timespec_diff(struct timespec *, struct timespec *, struct timespec *)
 * @brief Compute the diff of two timespecs, that is a - b = result.
 * @param a the minuend
 * @param b the subtrahend
 * @param result a - b
 * source: https://gist.github.com/diabloneo/9619917
 */
static inline void timespec_diff(struct timespec *a, 
        struct timespec *b,
        struct timespec *result) 
{
    result->tv_sec  = a->tv_sec  - b->tv_sec;
    result->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (result->tv_nsec < 0) {
        --result->tv_sec;
        result->tv_nsec += 1000000000L;
    }
}

static inline void timespec_to_float(struct timespec *in, float *result) 
{
    // negative values of tv_nsec already handled due to math rules...
    *result = (float) in->tv_sec + in->tv_nsec / (float) 1000000000;
}

static inline void timespec_to_double(struct timespec *in, double *result) 
{
    // negative values of tv_nsec already handled due to math rules...
    *result = (double) in->tv_sec + in->tv_nsec / (double) 1000000000;
}

#endif // LIB_H