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
* File: pcapsession_merger.c
* Date: Feb 15, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

/**
 * This module merges and dumps packets received on incoming streams onto a single
 * output stream
 */
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <tcp.h>
#include <logger.h>
#include <pcapdefines.h>
#include <pcapsession.h>

// Forward definition of private functions
void* pcapsession_merger_run(void* pcapsession_param);
void* pcapsession_merger_stop(void* pcapsession_param);

// Mutex to ensure only one client thread writes PCAP output at a time
pthread_mutex_t merge_mutex = PTHREAD_MUTEX_INITIALIZER;

//
// This function opens a PCAP merger instance
//
// Parameters:
//  char* filename: The name of the file to dump to "-" for stdout
//  addresslist_t* addresslist: The list of addresses of servers to connect to
//  int untunnel: If true, GTP-U packets should be untunnelled
//
// Returns:
//  pcapsession_t*: Returns a pointer to the merger or NULL if opening failed
//
pcapsession_t* pcapsession_merger_open(char* filename, int untunnel)
{
	write_to_syslog( "starting merging session to file %s, untunnel=%d\n", filename, untunnel);

	// Get and check if a new PCAP session is available
	pcapsession_t* pcapsession = pcapsession_handling_get_new();
	if (pcapsession == NULL) {
		return NULL;
	}

	// Set the run and stop methods for the session
	pcapsession->runner  = pcapsession_merger_run;
	pcapsession->stopper = pcapsession_merger_stop;

	// Set whether untunnelling is turned on for this session
	pcapsession->untunnel = untunnel;

	// Clear other fields on this session for now
	pcapsession->monitor = NULL;
	pcapsession->pcap_handle = NULL;
	pcapsession->pcap_dumper = NULL;
	pcapsession->handler = NULL;
	pcapsession->fd = 0;
	pcapsession->supervise_fd = PCAP_SESSION_SUPERVISED_FD;
	pcapsession->iterations = 0;

	// Set the description of the merger to be the description field
	strcpy(pcapsession->description, filename);

	// Return the result of adding the new pcapsession
	if (!pcapsession_handling_add(pcapsession->id)) {
		return NULL;
	}

	// Return the merger
	return pcapsession;
}

//
// This function kicks off the merger thread, it sets the session to state PCAP_SESSION_RUNNING
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on thread initiation, set to a pcapsession_t* here, points at merger
//
void* pcapsession_merger_run(void* pcapsession_param)
{
	// Dereference the pcapsession pointer
	pcapsession_t* pcapsession = pcapsession_param;

	if (pcapsession == NULL) {
		write_to_syslog( "could not run merger session thread, server not set\n");
		return NULL;
	}

	write_to_syslog( "merger session started: %d-%s\n", pcapsession->id, pcapsession->description);

	// Set the session state to run, run has been ordered
	pcapsession_change_state(pcapsession->id, PCAP_SESSION_RUNNING);

	// Set the monitor for this client
	pcapsession->monitor = monitor_open(pcapsession->id, pcapsession->description);

	// Start PCAP dumping on the specified file
	write_to_syslog( "merger session %d-%s: packet dumping to file starting\n", pcapsession->id, pcapsession->description);

	// We have to give pcap_dump_open an input file handle so we use a dead open with an Ethernet PCAP type
	pcap_t* pcap_dead_handle = pcap_open_dead(DLT_EN10MB, PCAP_MAX_SNAPLEN);

	// Open packet dumping on the specified file
	pcapsession->pcap_dumper = pcap_dump_open(pcap_dead_handle, pcapsession->description);
    if (pcap_dead_handle != NULL)
        pcap_close(pcap_dead_handle);

	if (pcapsession->pcap_dumper == NULL) {
		write_to_syslog( "merger session %d-%s: packet dump open failed, %s\n",
					pcapsession->id, pcapsession->description, pcap_geterr(pcap_dead_handle));
		pcapsession_change_state(pcapsession->id, PCAP_SESSION_TERMINATE);
		return NULL;
	}

	// Get the file descriptor from the PCAP dumper
	pcapsession->fd = fileno(pcap_dump_file(pcapsession->pcap_dumper));

	write_to_syslog( "merger session %d-%s: packet dumping to file started\n", pcapsession->id, pcapsession->description);

	// Happy days, merger is open
	return NULL;
}

//
// This function stops the merger, the state is reset back to PCAP_SESSION_START so that session handling will attempt to
// restart the merger
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on session stop, set to a pcapsession_t* here, points at the merger session
//
void* pcapsession_merger_stop(void* pcapsession_param)
{
	// Dereference the pcapsession pointer
	pcapsession_t* pcapsession = pcapsession_param;

	if (pcapsession == NULL) {
		write_to_syslog( "could not stop merger session, merger not set\n");
		return NULL;
	}

	write_to_syslog( "merger session stopping: %d-%s\n", pcapsession->id, pcapsession->description);

	// Turn off packet dumping on this merger, make sure the PCAP dump file is open before attempting close
	if (pcapsession->pcap_dumper != NULL && iotests_fd_open(fileno(pcap_dump_file(pcapsession->pcap_dumper)))) {
		// Close the dumper
		if (iotests_fd_open(fileno(pcap_dump_file(pcapsession->pcap_dumper)))) {
			pcap_dump_close(pcapsession->pcap_dumper);
		}
		pcapsession->pcap_dumper = NULL;
	}
	pcapsession->pcap_dumper = NULL;

	// Close the PCAP handle
	if (pcapsession->pcap_handle != NULL) {
		pcap_close(pcapsession->pcap_handle);
		pcapsession->pcap_handle = NULL;
	}

	// Close the merger file descriptor, make sure it is open first
	if (pcapsession->fd > 0 && iotests_fd_open(pcapsession->fd)) {
		close(pcapsession->fd);
	}
	pcapsession->fd = 0;

	// Check if the session file descriptor is set
	if (pcapsession->fd > 0) {
		// Close the descriptor and clear it
		close(pcapsession->fd);
		pcapsession->fd = 0;
	}

	// Close monitoring
	if (pcapsession->monitor != NULL) {
		monitor_close(pcapsession->monitor);
		pcapsession->monitor = NULL;
	}

	// Set the session state as appropriate
	if (pcapsession->state == PCAP_SESSION_ABORTING) {
		// On abort, always stop
		pcapsession_change_state(pcapsession->id, PCAP_SESSION_STOPPED);
	}
	else {
		// Try to restart the server
		pcapsession_change_state(pcapsession->id, PCAP_SESSION_START);
	}

	write_to_syslog( "merger session stopped: %d-%s\n", pcapsession->id, pcapsession->description);
	return NULL;
}

//
// This function is a PCAP packet handler callback method for packet merging
//
// Parameters:
//  unsigned char* pcapsession_param: A pointer to user data set in the pcap_loop call, in this case it is a pcapsession_t pointer
//  const struct pcap_pkthdr* header: A pointer to the header of the packet
//  const unsigned char* data: A pointer to the packet data
//
void pcapsession_merger_packet_handler(unsigned char* pcapsession_param, const struct pcap_pkthdr* header, const unsigned char* data)
{
	// Sanity check for initiation
	if (pcapsession_param == NULL) {
		return;
	}

	// Dereference the pcapsession_t pointer
	pcapsession_t* pcapsession = (pcapsession_t*)pcapsession_param;

	// Get and check the merger from the session handler
	pcapsession_t* pcapsession_merger = pcapsession->handler;
	if (pcapsession_merger == NULL) {
		return;
	}

	// CRITICAL SECTION, lock writing of packet to the merged PCAP file
	pthread_mutex_lock(&merge_mutex);

	// Copy the header data to a new modifiable header
	struct pcap_pkthdr new_header;
	new_header.caplen = header->caplen;
	new_header.len    = header->len;
	new_header.ts     = header->ts;

	// Check if this packet should be untunnelled
	if (pcapsession_merger->untunnel == PCAP_SESSION_UNTUNNEL_ON) {
		// Untunnel the packet
		pcapsession_untunnel_packet(pcapsession_merger, &new_header, data);
	}

	// Dump the packet
	// esirich DEFTFTS-1634 lock the pcap_dump so the monitor can flush
	pthread_mutex_lock(&(pcapsession_merger->pcap_mutex));
	pcap_dump((unsigned char*)pcapsession_merger->pcap_dumper, &new_header, data);
	pthread_mutex_unlock(&(pcapsession_merger->pcap_mutex));

	// Add a packet and the number of bytes to the monitor for the server connection
	monitor_increment(pcapsession->monitor, 1, header->len);

	// Add a packet and the number of bytes to the monitor for the merger
	monitor_increment(pcapsession_merger->monitor, 1, header->len);

	// CRITICAL SECTION OVER, packet written to merged PCAP file
	pthread_mutex_unlock(&merge_mutex);
}
