#include "signal_pin.h"
#include "types.h"
#include <wiringPi.h>
#include <iostream>

static volatile bool interrupt_triggered = false;

void signal_pin_handler(void)
{
    interrupt_triggered = true;
}

void signal_pin_thread(int pin, node_state_t *node_state) 
{
    int retval = EXIT_FAILURE;

    retval = wiringPiSetup();

    if (EXIT_SUCCESS == retval)
    {
        pinMode(pin, INPUT);
        retval = wiringPiISR(pin, INT_EDGE_RISING, &signal_pin_handler);
    }

    if (0 == EXIT_SUCCESS)
    {
        if (interrupt_triggered)
        {
            interrupt_triggered = false;

        }
    }
    else
    {
        std::cerr << "ERROR: setting up signal_pin_header FAILED!" << std::endl;
    }
}