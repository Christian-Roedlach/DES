#include "logging.h"
#include <fstream>
#include <iostream>
#include <string.h>

void thread_logging(node_state_t *node_state, nw_multicast_descriptor_t *nw_desc)
{
    int retval = EXIT_FAILURE;
    timestamp_t timestamp_to_log = 0;
    int sync_status_to_log = 0;
    bool log_pending = false;
    const useconds_t thread_sleep_time_us = (MICROTICK_NS / 1000) * MAKROTICK_MULT;

    /* open file to append to */
    std::fstream logfile(nw_desc->logfile_name, /*std::fstream::in |*/ std::fstream::out | std::fstream::app);

    if (logfile.is_open())
        retval = EXIT_SUCCESS;
    else
    {
        std::cerr << "ERROR: failed to open logfile: " << nw_desc->logfile_name << std::endl;
        std::cerr << "errno: " << errno << ": " << strerror( errno ) << std::endl;
        retval = EXIT_FAILURE;
    }
    
    if (EXIT_SUCCESS == retval)
    {
        /* allow errSt_running and errSt_retry to continue thread */
        while (errSt_restart > get_errorstate(node_state)) 
        {
            {
                std::lock_guard<std::mutex> lock(node_state->gpio_event_registered_mutex);
                
                if (node_state->gpio_event_registered)
                {
                    timestamp_to_log = node_state->gpio_event_registered_timestamp;
                    sync_status_to_log = node_state->gpio_event_registered_sync_status;
                    log_pending = true;
                    /* reset evant_registered_flag */
                    node_state->gpio_event_registered = false;
                }
            }
            /* mutex is released */

            if (log_pending)
            {
                if (logfile.is_open())
                {
                    log_pending = false;
                    logfile << std::to_string(timestamp_to_log) << "," 
                            << std::to_string(sync_status_to_log) << std::endl;
                }
                else
                {
                    set_errorstate(node_state, errSt_restart);
                    std::cerr << "ERROR: logfile is not open anymore! --> " << nw_desc->logfile_name << std::endl;
                    std::cerr << "errno: " << errno << ": " << strerror( errno ) << std::endl;
                }
            }

            /* sleep for one macrotick */
            usleep(thread_sleep_time_us);  
        }
        #if DEBUG_LOGGING
            std::cout<< "THREAD: logging STOPPED!"<<std::endl;
        #endif // DEBUG_LOGGING
    }
    else
    {
        set_errorstate(node_state, errSt_restart);
        std::cerr << "ERROR: thread_logging not initialized!" << std::endl;
    }

    logfile.close();
}

void write_syslog(std::string message, int log_type)
{
    setlogmask (LOG_UPTO (LOG_INFO));
    openlog (SYSLOG_NAME, LOG_PERROR | LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog (log_type, "%s", message.c_str());
    closelog ();
}

void syslog_program_start(void)
{
    setlogmask (LOG_UPTO (LOG_INFO));
    openlog (SYSLOG_NAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog (LOG_NOTICE, "Program started by User %d", getuid ());
    closelog ();
}

errorstate_t check_previous_execution_state(uint32_t * failure_count)
{
    /* open file to read from */
    errorstate_t retval = errSt_undefined;
    std::fstream exec_state_file(EXECUTION_STATE_FILENAME, std::fstream::in);

    if(exec_state_file.is_open())
    {
        std::string line;
        int errorstate_temp = -1;

        if(getline(exec_state_file, line)) {
            auto delimiterPos = line.find("=");
            auto name = line.substr(0, delimiterPos);
            auto value = line.substr(delimiterPos + 1);
            if (EXECUTION_STATE_STRING == name) {
                errorstate_temp = std::stoi(value);
                if (0 <= errorstate_temp && errSt_count > errorstate_temp)
                    retval = static_cast<errorstate_t>(errorstate_temp);
            } 
            else 
            {
                std::cerr << "ERROR: parsing " << EXECUTION_STATE_FILENAME << ":" << EXECUTION_STATE_STRING << " failed!" << std::endl;
            }    
        }
        if(getline(exec_state_file, line)) {
            auto delimiterPos = line.find("=");
            auto name = line.substr(0, delimiterPos);
            auto value = line.substr(delimiterPos + 1);
            if (EXECUTION_STATE_FAILURE_COUNT == name) {
                *failure_count = std::stoi(value);
            } 
            else 
            {
                std::cerr << "ERROR: parsing " << EXECUTION_STATE_FILENAME << ":" << EXECUTION_STATE_FAILURE_COUNT << " failed!" << std::endl;
            }    
        }

        exec_state_file.close();
    }
    else
        retval = errSt_undefined;

    return retval;
}

int write_program_exit_status(errorstate_t errorstate, uint32_t failure_count, bool syslog)
{
    int retval = EXIT_FAILURE;
    std::string syslog_message = "";

    std::fstream exec_state_file(EXECUTION_STATE_FILENAME, std::fstream::out);
    if (exec_state_file.is_open())
    {
        exec_state_file << EXECUTION_STATE_STRING << "=" << static_cast<int>(errorstate) << std::endl;
        exec_state_file << EXECUTION_STATE_FAILURE_COUNT << "=" << std::to_string(failure_count) << std::endl;

        exec_state_file.close();
        syslog_message.append("exit state written to " 
                EXECUTION_STATE_FILENAME
                ", state=");
        syslog_message.append(std::to_string(errorstate));
        syslog_message.append(", failure_count=");
        syslog_message.append(std::to_string(failure_count));

        if (syslog)
        {
            write_syslog(syslog_message, LOG_NOTICE);
        }
        retval = EXIT_SUCCESS;
    }
    else 
    {
        write_syslog("ERROR: writing to " EXECUTION_STATE_FILENAME " failed!", LOG_ERR);
        retval = EXIT_FAILURE;
    }    

    return retval;
}

int process_previous_execution_state(errorstate_t *errorstate, uint32_t *failure_count)
{
    int retval = EXIT_FAILURE;
    std::string log_message = "";

    switch (*errorstate)
    {
        case errSt_terminatedFromOutside:
        case errSt_undefined:
        case errSt_running:
        case errSt_retry:

            /* reset failure_count */
            *failure_count = 0;
            retval = EXIT_SUCCESS;
            break;

        case errSt_restart:

            if (RESTART_COUNTER_MAX >= *failure_count)
            {
                log_message = "processing restart number: ";
                log_message.append(std::to_string(*failure_count));
                write_syslog(log_message, LOG_CRIT);
                retval = EXIT_SUCCESS;
            }
            else
            {
                log_message = "too many restarts -> STOP + DEACTIVATING... count: ";
                log_message.append(std::to_string(*failure_count));
                write_syslog(log_message, LOG_CRIT);
                retval = EXIT_FAILURE;
            }
            break;

        case errSt_stop_leave_service:
        case srrSt_stop_disable_service:
            retval = EXIT_FAILURE;
            write_syslog("program started although previous execution state was STOP!!!", LOG_CRIT);
            break;

        case errSt_segfault:

            retval = EXIT_FAILURE;
            *errorstate = srrSt_stop_disable_service;
            write_syslog("previous SEGFAULT registered - STOP + DEACTIVATING...", LOG_CRIT);
            break;

        default:
            retval = EXIT_FAILURE;
    }

    return retval;
}
    