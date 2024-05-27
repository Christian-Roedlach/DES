#ifndef SETTINGS_H
#define SETTINGS_H

#define TIMEOUT_S   (0)
#define TIMEOUT_US  (1000)
#define NR_ELEMENTS_TO_CACHE 500000
#define CSV_SEPARATOR ','
#define CSV_FIELDS "id" "," "roundtrip_time [s]"
#define TIMESTAMP_DELAY_VALUE_TICKS (150U)
#define TIMESTAMP_MAX_DEVIATION_VALUE_TICKS (10U)
#define RECEIVE_ERROR_COUNT_MAX (5)

#define THREAD_PRIORITY_RECEIVE 50 // 0=low to 99=high
#define THREAD_PRIORITY_TIMER 99 // 0=low to 99=high

// DEBUGGING OPTIONS (PRODUCTION: SET EVERYTHING TO 0)
#define STDOUT_LOGGING_ENABLED 0
#define DEBUG_LOGGING 1
#define ERROR_LOGGING 1
#define DEBUG_SET_RCV_PORT_TO_SND_PORT 0
#define DEBUG_DONT_CHECK_ACK 0

#define CHECK_ACK_CONDITION   0 == strncmp(nw_desc->message_rcv.control, "ACK", sizeof(nw_desc->message_rcv.control))
#if DEBUG_DONT_CHECK_ACK
    #define CHECK_ACK 1
#else
    #define CHECK_ACK CHECK_ACK_CONDITION
#endif // DEBUG_DONT_CHECK_ACK

#endif // SETTINGS_H