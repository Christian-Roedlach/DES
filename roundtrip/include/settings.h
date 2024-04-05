#ifndef SETTINGS_H
#define SETTINGS_H

#define TIMEOUT_S   (1)
#define TIMEOUT_US  (0)
#define NR_ELEMENTS_TO_CACHE 500000
#define CSV_SEPARATOR ','
#define CSV_FIELDS "id" "," "roundtrip_time [s]"

// DEBUGGING OPTIONS (PRODUCTION: SET EVERYTHING TO 0)
#define STDOUT_LOGGING_ENABLED 0
#define DEBUG_LOGGING 0
#define DEBUG_SET_RCV_PORT_TO_SND_PORT 0
#define DEBUG_DONT_CHECK_ACK 0

#define CHECK_ACK_CONDITION   0 == strncmp(nw_desc->message_rcv.control, "ACK", sizeof(nw_desc->message_rcv.control))
#if DEBUG_DONT_CHECK_ACK
    #define CHECK_ACK 1
#else
    #define CHECK_ACK CHECK_ACK_CONDITION
#endif // DEBUG_DONT_CHECK_ACK

#endif // SETTINGS_H