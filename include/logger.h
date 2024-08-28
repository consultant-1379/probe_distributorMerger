/************************************************************************
* COPYRIGHT (C) Ericsson 2012                                           *
* The copyright to the computer program(s) herein is the property       *
* of Telefonaktiebolaget LM Ericsson.                                   *
* The program(s) may be used and/or copied only with the written        *
* permission from Telefonaktiebolaget LM Ericsson or in accordance with *
* the terms and conditions stipulated in the agreement/contract         *
* under which the program(s) have been supplied.                        *
*************************************************************************
*************************************************************************
* File: logger.h
* Date: Feb 28, 2012
* Author: LMI/LXR/SH John Lee
************************************************************************/

/**
 *******************************************************************************
 * @file logger.h
 * @defgroup LOGGER logger
 *
 * @lld_start
 * @lld_overview
 * 
 * This API respresents wraps the syslogd API and provides utilities for logging.
 *
 * @lld_end
 ******************************************************************************/

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>

#define MAX_MESSAGE_HEADER_SIZE 256
#define MAX_MESSAGE_BODY_SIZE 65536

#ifdef __cplusplus
extern "C" {
#endif 

/**
 *******************************************************************************
 * @ingroup COMMS
 * @description
 *    Establish a connection to the syslogd daemon.
 *
 * @param prog_name     IN      Identifier for the program in the log file.
 ******************************************************************************/
static inline void init_logger(const char *prog_name)
{
    static int init_done = 0;
    if(init_done == 0)
    {
        init_done++;
		openlog(prog_name, LOG_NDELAY|LOG_PID, LOG_LOCAL2);
    }
};

/**
 *******************************************************************************
 * @ingroup COMMS
 * @description
 *    Write the message to the syslog daemon.
 *
 * @param message     IN      The contents of the message to write to file.
 ******************************************************************************/
static inline void write_to_syslog(char * message,...)
{
    char header[MAX_MESSAGE_HEADER_SIZE];
    char body[MAX_MESSAGE_BODY_SIZE];

    va_list arguments;
    struct timeval tv;
    char time_arr[16];  

    time_t current_time = time(NULL);
    struct tm* local_time = localtime(&current_time);
    gettimeofday(&tv, NULL);
    
    sprintf(time_arr, "%02d:%02d:%02d.%03d", (int)local_time->tm_hour,
        (int)local_time->tm_min, (int)local_time->tm_sec, (int)tv.tv_usec/1000);
    
    sprintf(header, "eniq> %s ? %d:", time_arr, LOG_NOTICE);

    memset(header,0,MAX_MESSAGE_HEADER_SIZE);
    memset(body,0,MAX_MESSAGE_BODY_SIZE);


    if (strlen(message) < sizeof(body))
    {
        va_start(arguments, message);
        vsprintf(body, message, arguments);
        va_end(arguments);
        syslog(LOG_NOTICE|LOG_LOCAL2,"%s %s", header, body);
    }
};

#ifdef __cplusplus
}
#endif 
#endif /* LOGGER_H_ */
