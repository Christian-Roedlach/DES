#include <signal.h>
#include <time.h>
#include <iostream>
#include <unistd.h>

// Global counter variable
volatile int counter = 0;

// Timer handler function
void timer_handler(union sigval sv) {
    counter++;
    std::cout << "Counter: " << counter << std::endl;
}

int main() {
    struct sigevent sev;
    struct itimerspec its;
    timer_t timerid;
    int ret;

    // Create the timer
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_value.sival_ptr = NULL;
    sev.sigev_notify_function = timer_handler;
    sev.sigev_notify_attributes = NULL;

    ret = timer_create(CLOCK_REALTIME, &sev, &timerid);
    if (ret == -1) {
        perror("timer_create");
        return 1;
    }

    // Start the timer
    its.it_value.tv_sec = 0;  // Initial delay, nanoseconds
    its.it_value.tv_nsec = 1000000; //nanoseconds
    its.it_interval.tv_sec = 0;  // Interval delay, seconds
    its.it_interval.tv_nsec = 1000000;//nanoseconds

    ret = timer_settime(timerid, 0, &its, NULL);
    if (ret == -1) {
        perror("timer_settime");
        return 1;
    }

    
    while (1) {
        sleep(1);  
    }

    return 0;
}