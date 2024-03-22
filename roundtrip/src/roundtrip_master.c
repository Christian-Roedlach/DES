#include "types.h"
#include "lib.h"
#include "network.h"
#include "settings.h"
#include <stdio.h>
#include <string.h>

int test_gettime (void);
int parse_arguments (int argc, char** argv, nw_descriptor_t *nw_desc);
int do_roundtrip_measurement (nw_descriptor_t *nw_desc, double *result_sec);
int do_roundtrip_sequence (nw_descriptor_t *nw_desc);

int main (int argc, char** argv)
{
    int retval = EXIT_FAILURE;
    nw_descriptor_t nw_desc = {
        .message_snd = {{0}},
        .message_rcv = {{0}},
        .socket_file_descriptor = -1,
        .master_nw_socket_addr = {0},
        .slave_nw_socket_addr = {0},
        .recv_nw_socket_addr = {0},
        .timeout = {TIMEOUT_S, TIMEOUT_US}
    };

    retval = parse_arguments(argc, argv, &nw_desc);
       
    if (EXIT_SUCCESS == retval)
        retval = socket_master(&nw_desc);

    if (EXIT_SUCCESS == retval)
        retval = do_roundtrip_sequence (&nw_desc);
    
    //retval = test_gettime();

    return retval;
}

int parse_arguments (int argc, char** argv, nw_descriptor_t *nw_desc)
{
    int retval = EXIT_FAILURE;
    int64_t parsed_numbers; 

    if (argc == 4)
    {
        /* parse IP address and port */
        retval = set_socket_address(argv[1], argv[2], &nw_desc->slave_nw_socket_addr);

        /* parse number of messages */
        if (EXIT_SUCCESS == retval)
        {
            /* reset return value */
            retval = EXIT_FAILURE;
            parsed_numbers = strtol(argv[3], NULL, 10);

            if (0L != parsed_numbers)
            {
                if(0 <= parsed_numbers && UINT32_MAX >= parsed_numbers)
                {
                    nw_desc->nr_of_messages = (uint32_t) parsed_numbers;
                    retval = EXIT_SUCCESS;
                }    
            }
        }        
    }

    if (EXIT_SUCCESS != retval)
    {
        fprintf(stderr, "Error occured: %s\n\n", strerror( errno ));
        fprintf(stderr, "Usage:  ./master <Slave IPv4 Address> <Slave Port Number> <Nr. of messages>\n\n");
    }

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
        fprintf(stderr, "Error occured: %s\n", strerror(errno));
    }

    return retval;
}

int do_roundtrip_measurement (nw_descriptor_t *nw_desc, double *result_sec)
{
    int retval = EXIT_FAILURE;
    struct timespec timestamp_end = {0};
    struct timespec timestamp_diff = {0};

    strncpy(nw_desc->message_snd.control, "REQ", sizeof(nw_desc->message_snd.control));
    retval = clock_gettime(CLOCK_MONOTONIC, &nw_desc->message_snd.timestamp);
    
    /* message number has to be set in calling function ! */
    if (EXIT_SUCCESS == retval)
        retval = send_and_receive_roundtrip(nw_desc);

    if (EXIT_SUCCESS == retval)
    {
        timespec_diff(&timestamp_end, &nw_desc->message_snd.timestamp, &timestamp_diff);
        timespec_to_double(&timestamp_diff, result_sec);
    }

    return retval;
}

int do_roundtrip_sequence (nw_descriptor_t *nw_desc)
{
    int retval = EXIT_FAILURE;
    size_t i = 0;
    double time_measured = 0;

    for (i = 0; i < nw_desc->nr_of_messages; i++)
    {
        nw_desc->message_snd.id = i;

        retval = do_roundtrip_measurement(nw_desc, &time_measured);

        if (EXIT_SUCCESS == retval)
        {
            /* ToDo: check message */
            fprintf(stdout, "Msg %6d: Measurment result: %11.9lf s\n", 
                    nw_desc->message_rcv.id, time_measured);
        }
        else
        /* break if not successful - might be removed */
        {
            fprintf(stderr, "Error executing measurement: %s\n", strerror(errno));
            break;
        }
    }

    return retval;
}
