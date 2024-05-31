#ifndef LOGGING_H
#define LOGGING_H

#include "types.h"
#include "settings.h"
#include "network.h"
#include <syslog.h>

void thread_logging(node_state_t *node_state, nw_multicast_descriptor_t *nw_desc);
void write_syslog(std::string message, int log_type = LOG_INFO);
void syslog_program_start(void);

#endif // LOGGING_H