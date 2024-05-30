#include "signal_pin.h"
#include <wiringPi.h>
#include <iostream>
#include "settings.h"

/* global variables have to be used as wiringPiISR does not allow arguments */
static node_state_t *node_state_ptr = nullptr;

void signal_pin_handler(void);

void signal_pin_handler(void)
{
    {
        std::lock_guard<std::mutex> lock(node_state_ptr->gpio_event_registered_mutex);
        node_state_ptr->gpio_event_registered_timestamp = node_state_ptr->timestamp;
        node_state_ptr->gpio_event_registered_sync_status = node_state_ptr->time_synced;
        node_state_ptr->gpio_event_registered = true;
    }
    /* mutex is released */
}

void thread_signal_pin(int pin, node_state_t *node_state) 
{
    int retval = EXIT_FAILURE;
    /* set global pointer to handled pointer */
    node_state_ptr = node_state;

#if (RASPBERRY_PI)
    retval = wiringPiSetup();

    if (EXIT_SUCCESS == retval)
    {
        pinMode(pin, INPUT);
        retval = wiringPiISR(pin, INT_EDGE_RISING, &signal_pin_handler);
    }
#else
    retval = EXIT_SUCCESS;
#endif // RASPBERRY_PI

    if (EXIT_SUCCESS == retval)
    {
        while (EXIT_SUCCESS == node_state->errorstate) 
        {
            #if (RASPBERRY_PI)
                sleep(1);  
            #else // RASPBERRY_PI
                /* write testdata */
                {
                    std::lock_guard<std::mutex> lock(node_state_ptr->gpio_event_registered_mutex);
                    node_state_ptr->gpio_event_registered_timestamp = node_state_ptr->timestamp;
                    node_state_ptr->gpio_event_registered_sync_status = node_state_ptr->time_synced;
                    node_state_ptr->gpio_event_registered = true;
                }
                
                /* sleep for 100 ms */
                usleep(100000);
            #endif // RASPBERRY_PI
        }

        #if DEBUG_LOGGING
            std::cout<< "THREAD: signal_pin STOPPED!"<<std::endl;
        #endif // DEBUG_LOGGING
    }
    else
    {
        #if (RASPBERRY_PI)
        node_state->errorstate = EXIT_FAILURE;
        #else  // RASPBERRY_PI
            #warning "signal pin exit program DISABLED (for PC development)"
        #endif // RASPBERRY_PI
        std::cerr << "ERROR: setting up signal_pin_header FAILED!" << std::endl;
    }
}