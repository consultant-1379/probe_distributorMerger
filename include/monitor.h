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
* File: config.h
* Date: Feb 23, 2012
* Author: LMI/LXR/SH Liam Fallon, John Lee
************************************************************************/

/**
 *******************************************************************************
 * @file monitor.h
 * @defgroup MONITOR monitor
 *
 * @lld_start
 * @lld_overview
 * 
 * This API monitors statistics on PCAP streams.
 *
 * @lld_end
 ******************************************************************************/
#ifndef MONITOR_H_
#define MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include public/global header files
*******************************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*******************************************************************************
* Include private header files
*******************************************************************************/
#include <gtpv1.h>
#include <logger.h>

/*******************************************************************************
* Define Constants and Macros
*******************************************************************************/
#define MONITOR_OUTPUT_INTERVAL 10 /* In seconds */
#define MAX_MONITORS 128

/**
 *******************************************************************************
 * @ingroup MONITOR
 * @description
 *    Last output time.
 ******************************************************************************/
static time_t monitor_last_output_time;

/**
 *******************************************************************************
 * @ingroup MONITOR
 * @description
 *    Monitor structure.
 ******************************************************************************/
struct monitor {
	int id;                              // The ID of the monitor
	char description[FILENAME_MAX];      // The monitor description
	long long packets;                   // The number of packets monitored
	long long bytes;                     // The number of bytes monitored
	long long gtp_packets;               // The number of bytes monitored
	long long gtp_bytes;                 // The number of bytes monitored
	long long gtp_ext;                   // The number of packets with GTP extension header optional fields
	long long gtp_seqno;                 // The number of packets with GTP sequence number fields
	long long gtp_npdu;                  // The number of packets with GTP sequence N-PDU fields
	time_t last_output_time;             // The time of the last output
};

/**
 *******************************************************************************
 * @ingroup MONITOR
 * @description
 *    This function opens a monitor.
 *
 * @param id            IN      The ID of this monitor
 * @param description   IN      The description of the monitor
 *
 * @retval monitor      OUT     Pointer to the monitor structure.
 ******************************************************************************/
static inline struct monitor* monitor_open(int id, char* description)
{
	// Allocate memory for the monitor struct
	struct monitor* monitor = (struct monitor *) calloc(1, sizeof(struct monitor));

	// Set the initial data for this monitor
	monitor->id = id;

	// Set the description of the monitor, set last character as zero to avoid overruns on strings
	strncpy(monitor->description, description, FILENAME_MAX);
	monitor->description[FILENAME_MAX-1] = 0;

	// If overall last output time is zero, set it to now
	if (monitor_last_output_time == 0) {
		monitor_last_output_time = time(NULL);
	}

	// Set the last output time of this monitor to the overall last output time
	monitor->last_output_time = monitor_last_output_time;

	// Return the monitor
	return monitor;
};

/**
 *******************************************************************************
 * @ingroup MONITOR
 * @description
 *    This function closes a monitor.
 *
 * @param monitor      OUT     Pointer to the monitor structure.
 ******************************************************************************/
static inline void monitor_close(struct monitor* monitor)
{
	// Clear the monitor memory and set the monitor as null
    if(monitor!=NULL)
    {
	    free(monitor);
	    monitor = NULL;
    }
};

/**
 *******************************************************************************
 * @ingroup MONITOR
 * @description
 *    This function increments the monitored variables for a monitor.
 *
 * @param monitor       IN     Pointer to the monitor structure.
 * @param packets       IN      The number of packets
 * @param bytes         IN      The number of bytes
 ******************************************************************************/
static inline void monitor_increment(struct monitor* monitor, long long packets, long long bytes)
{
	// Sanity check the monitor pointer
	if (monitor == NULL) {
		return;
	}

	monitor->packets += packets;
	monitor->bytes += bytes;
};

/**
 *******************************************************************************
 * @ingroup MONITOR
 * @description
 *    This function increments the monitored gtp variables for a monitor.
 *
 * @param monitor       IN      Pointer to the monitor structure.
 * @param gtp_packets   IN      The number of packets
 * @param gtp_bytes     IN      The number of bytes
 * @param gtp_options   IN      The gtp options
 ******************************************************************************/
static inline void monitor_increment_gtp(struct monitor* monitor, long long gtp_packets, 
    long long gtp_bytes, int gtp_options)
{
	// Sanity check the monitor pointer
	if (monitor == NULL) {
		return;
	}

	monitor->gtp_packets += gtp_packets;
	monitor->gtp_bytes   += gtp_bytes;
	monitor->gtp_ext     += GTP_EXT_FLAG(gtp_options);
	monitor->gtp_seqno   += GTP_SEQ_FLAG(gtp_options);
	monitor->gtp_npdu    += GTP_NPDU_FLAG(gtp_options);
};

/**
 *******************************************************************************
 * @ingroup MONITOR
 * @description
 *  This function handles a single monitor, outputs its data and clears 
 *  its variables.
 *
 * @param monitor       IN      Pointer to the monitor structure.
 ******************************************************************************/
static inline void handle_monitor(struct monitor* monitor)
{
	// Check if monitoring should be output or not at an overall level
	int current_time = time(NULL);
	if (current_time - monitor_last_output_time >= MONITOR_OUTPUT_INTERVAL) {
		// Set the overall last monitor time to now
		monitor_last_output_time = current_time;
	}

	// Now check if this monitor should be output
	if (monitor_last_output_time <= monitor->last_output_time) {
		// Monitor should not be output
		return;
	}

	int time_since_output = current_time - monitor->last_output_time;

	// Calculate some variables from the monitor
	double packets_per_second = (monitor->packets > 0 ? (double)(monitor->packets / time_since_output) : 0.0);
	double bytes_per_second = (monitor->bytes > 0 ? (double)(monitor->bytes / time_since_output) : 0.0);
	double bits_per_second = bytes_per_second * 8;

	write_to_syslog("monitor %d-%s: pkts=%lld, pkts/s=%f, bytes=%lld, bytes/s=%f, Mbits/s=%f",
			monitor->id, monitor->description, monitor->packets, packets_per_second, monitor->bytes, bytes_per_second, bits_per_second / 1000000);
	write_to_syslog(" gtppkts=%lld, gtpbytes=%lld, gtpext=%lld, gtpseqno=%lld, gtpnpdu=%lld\n",
			monitor->gtp_packets, monitor->gtp_bytes, monitor->gtp_ext, monitor->gtp_seqno, monitor->gtp_npdu);

	// Record output time
	monitor->last_output_time = monitor_last_output_time;

	// Clear all counters
	monitor->bytes = 0;
	monitor->packets = 0;
	monitor->gtp_bytes = 0;
	monitor->gtp_packets = 0;
	monitor->gtp_ext = 0;
	monitor->gtp_seqno = 0;
	monitor->gtp_npdu = 0;
};

#ifdef __cplusplus
}
#endif 
#endif /* MONITOR_H */
