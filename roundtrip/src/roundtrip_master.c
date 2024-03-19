#include <types.h>

int main (int argc, char** argv)
{
    int retval = EXIT_FAILURE;
    message_t message = {0};

    retval = clock_gettime(CLOCK_MONOTONIC, &message.timestamp);


    return retval;
}