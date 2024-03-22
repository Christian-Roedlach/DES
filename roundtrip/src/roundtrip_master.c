#include "types.h"
#include "lib.h"
#include <stdio.h>
#include <string.h>

int test_gettime (void);

int main (int argc, char** argv)
{
    int retval = EXIT_FAILURE;
    message_t message = {0};
    
    retval = test_gettime();
    
    return retval;
}

int test_gettime (void)
{
    int retval = EXIT_FAILURE;
    struct timespec timestamp_start, timestamp_stop, timestamp_diff;
    struct timespec sleeptime = {.tv_sec = 1, .tv_nsec = 1000000};
    struct timespec sleeptime_readback;
    float timediff_sec_float;
    double timediff_sec_double;

    retval = clock_gettime(CLOCK_MONOTONIC, &timestamp_start);
   

    if (EXIT_SUCCESS == retval)
    {
        retval = nanosleep(&sleeptime, &sleeptime_readback);
    }

    if (EXIT_SUCCESS == retval)
    {
        retval = clock_gettime(CLOCK_MONOTONIC, &timestamp_stop);
    }

    if (EXIT_SUCCESS == retval)
    {
        timespec_diff(&timestamp_stop, &timestamp_start, &timestamp_diff);
        timespec_to_float(&timestamp_diff, &timediff_sec_float);
        timespec_to_double(&timestamp_diff, &timediff_sec_double);
        fprintf(stdout, "Timedifference (timespec): %ld s and %ld ns\n", timestamp_diff.tv_sec, timestamp_diff.tv_nsec);
        fprintf(stdout, "Timedifference (float):    %11.9f s\n", timediff_sec_float);
        fprintf(stdout, "Timedifference (double):   %11.9lf s\n", timediff_sec_double);
    }
    else
    {
        fprintf(stderr, "Error occured: %s\n", strerror( errno ));
    }

    return retval;
}
