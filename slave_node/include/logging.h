#ifndef LOGGING_H
#define LOGGING_H

#include "types.h"
#include "settings.h"
#include "network.h"
#include <syslog.h>

void thread_logging(node_state_t *node_state, nw_multicast_descriptor_t *nw_desc);
void write_syslog(std::string message, int log_type = LOG_INFO);
void syslog_program_start(void);
errorstate_t check_previous_execution_state(uint32_t * failure_count);
int write_program_exit_status(errorstate_t errorstate, uint32_t failure_count, bool syslog=true);
int process_previous_execution_state(errorstate_t *errorstate, uint32_t *failure_count);

#endif // LOGGING_H