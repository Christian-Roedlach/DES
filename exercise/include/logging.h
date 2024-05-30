#ifndef LOGGING_H
#define LOGGING_H

#include "types.h"
#include "settings.h"
#include "network.h"

void thread_logging(node_state_t *node_state, nw_multicast_descriptor_t *nw_desc);

#endif // LOGGING_H