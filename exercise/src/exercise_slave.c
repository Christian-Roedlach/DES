#include "types.h"
#include "lib.h"
#include "network.h"
#include "settings.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define USAGE "Usage:  \
./slave <Slave Port Number>\n\n"


int parse_arguments (int argc, char** argv, nw_descriptor_t *nw_desc);


int main (int argc, char** argv)
{
    int retval = EXIT_FAILURE;
    /*
    nw_descriptor_t nw_desc = {
        .message_snd = {0},
        .message_rcv = {0},
        .socket_file_descriptor = -1,
        .master_nw_socket_addr = {0},
        .slave_nw_socket_addr = {0},
        .recv_nw_socket_addr = {0},
        .timeout = {TIMEOUT_S, TIMEOUT_US}
    };
    setlinebuf(stdout);

    retval = parse_arguments(argc, argv, &nw_desc);
       
    if (EXIT_SUCCESS == retval)
        retval = socket_slave(&nw_desc);

    if (EXIT_SUCCESS == retval)
    {
        while(1)
		{
			retval = recieve_and_send_roundtrip(&nw_desc);
			
		}
    }
         
    //retval = test_gettime();
    if (-1 != nw_desc.socket_file_descriptor)
        retval = close(nw_desc.socket_file_descriptor);

    if (EXIT_SUCCESS != retval)
        fprintf(stderr, "Error occured: %s\n\n", strerror( errno ));
    */
    return retval;
}

int parse_arguments (int argc, char** argv, nw_descriptor_t *nw_desc)
{
    int retval = EXIT_FAILURE;
    //int64_t parsed_numbers; 

    if (argc == 2)
    {
        /* parse IP address and port */
        retval = set_socket_address("0.0.0.0", argv[1], &nw_desc->slave_nw_socket_addr);
    }    
       
    if (EXIT_SUCCESS != retval)
    {
        fprintf(stderr, "Error occured: %s\n\n", strerror( errno ));
        fprintf(stderr, USAGE);
    }

    return retval;
}





