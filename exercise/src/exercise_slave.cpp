#include "types.h"
#include "lib.h"
#include "network.h"
#include "settings.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define USAGE "Usage:  \
./exercise_slave <Multicast Address> <Port Number>\n\n"


int parse_arguments (int argc, char** argv, nw_descriptor_t *nw_desc);


int main (int argc, char** argv)
{
    int retval = EXIT_FAILURE;
    node_state_t node_state;
    nw_descriptor_t nw_desc;

    setlinebuf(stdout);

    retval = parse_arguments(argc, argv, &nw_desc);
       
    if (EXIT_SUCCESS == retval)
        retval = socket_slave_multicast(&nw_desc);
/*
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

    if (argc == 3)
    {
        // char all_addresses[] = "0.0.0.0";
        /* parse IP address and port */
        retval = set_socket_address(argv[1], argv[2], &nw_desc->slave_nw_socket_addr);
    }    
       
    if (EXIT_SUCCESS != retval)
    {
        fprintf(stderr, "Error occured: %s\n\n", strerror( errno ));
        fprintf(stderr, USAGE);
    }

    return retval;
}





