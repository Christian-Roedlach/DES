#include "types.h"
#include "network.h"
#include "lib.h"

int thread_timer(node_state_t *node_state);
void timer_handler(union sigval sv);
int start_timer(node_state_t *node_state,timer_t *timerid);
void stop_timer(timer_t timerid);