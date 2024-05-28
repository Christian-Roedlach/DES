#include <signal.h>
#include <time.h>
#include <iostream>
#include <unistd.h>
#include <settings.h>
#include <lib.h>
#include <types.h>
#include <lib-timer.h>

// Global counter variable
volatile int counter = 0;

// Timer handler function
void timer_handler(union sigval sv) {
    #if DEBUG_LOGGING
    static struct timespec timestamp_last = {};
    static struct timespec timestamp_current = {};
    static struct timespec timestamp_diff = {};  
    static float diff_sec = 0;
    //double timestamp_diff_double = 0;

    clock_gettime(CLOCK_MONOTONIC, &timestamp_current);
    timespec_diff(&timestamp_current, &timestamp_last, &timestamp_diff);
    clock_gettime(CLOCK_MONOTONIC, &timestamp_last);

    timespec_to_float(&timestamp_diff, &diff_sec);
    std::cout << "Counter: " << counter << " , timediff: " << diff_sec << " s" << std::endl;
    #endif // DEBUG_LOGGING


    node_state_t *node_state = static_cast<node_state_t*>(sv.sival_ptr);
    {
        std::lock_guard<std::mutex> lock(node_state->timestamp_mutex);
        node_state->timestamp++;
       
    }
}

int thread_timer(node_state_t *node_state) {
    
    timer_t timerid;
    
    if (start_timer(node_state,&timerid) != 0) {
        return 1;
    }


    while (EXIT_SUCCESS == node_state->errorstate) {
        sleep(1);
          
    }

    stop_timer(timerid);

    


    return 0;
}

int start_timer(node_state_t *node_state,timer_t *timerid){

    struct sigevent sev;
    struct itimerspec its;
    
    int ret;

    // Create the timer
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_value.sival_ptr = node_state;
    sev.sigev_notify_function = timer_handler;
    sev.sigev_notify_attributes = NULL;

    ret = timer_create(CLOCK_MONOTONIC, &sev, timerid);
    if (ret == -1) {
        perror("timer_create");
        return 1;
    }

    // Start the timer
    its.it_value.tv_sec = 0;  // Initial delay, nanoseconds
    its.it_value.tv_nsec = 500000; //nanoseconds --> 500us
    its.it_interval.tv_sec = 0;  // Interval delay, seconds
    its.it_interval.tv_nsec = 500050;//nanoseconds --> 500us

    ret = timer_settime(timerid, 0, &its, NULL);
    if (-1 == ret) {
        perror("timer_settime");
        return 1;
    }

    return 0;
}

void stop_timer(timer_t timerid){

    int ret = timer_delete(timerid);
    if(ret == -1){
        perror("timer_delete");
    } else{

        #if DEBUG_LOGGING
        std::cout<< "Timer stopped!"<<std::endl;
        #endif
    }
}



