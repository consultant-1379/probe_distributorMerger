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
* File: pcapsession_livecapture.c
* Date: Feb 17, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

/**
 * This module handles PCAP live capture sessions
 */

#include <string.h>
#include <stdlib.h>

#include <logger.h>
#include <monitor.h>
#include <pcapdefines.h>
#include <pcapsession.h>

// Forward definition of private functions
void* pcapsession_livecapture_run(void* pcapsession_param);
void* pcapsession_livecapture_stop(void* pcapsession_param);

//
// This function opens a PCAP live capture session
//
// Parameters:
//  char* interface_name: The interface on which to start live capture
//
// Returns:
//  int: Returns a value of 1 if the live capture session is created and added to session handling
//
int pcapsession_livecapture_open(char* interface_name)
{
	write_to_syslog( "opening live capture session on interface %s\n", interface_name);

	// Get and check if a new PCAP session is available
	pcapsession_t* pcapsession = pcapsession_handling_get_new();
	if (pcapsession == NULL) {
		return 0;
	}

	// Set the run and stop methods for the session
	pcapsession->runner  = pcapsession_livecapture_run;
	pcapsession->stopper = pcapsession_livecapture_stop;

	// Save the interface name for capture as the description
	strcpy(pcapsession->description, interface_name);

	write_to_syslog( "packet capture starting on live capture session: %d-%s\n", pcapsession->id, pcapsession->description);

	// Start PCAP capture on the specified interface
	char pcap_errbuf[PCAP_ERRBUF_SIZE];
	pcapsession->pcap_handle = pcap_open_live(pcapsession->description, PCAP_MAX_SNAPLEN, PCAP_PROMISCUOUS, PCAP_TIMEOUT, pcap_errbuf);
	if (pcapsession->pcap_handle == NULL) {
		write_to_syslog( "packet capture start failed on live capture session: %d-%s, %s\n", pcapsession->id, pcapsession->description, pcap_errbuf);
		pcapsession_change_state(pcapsession->id, PCAP_SESSION_UNUSED);
		return 0;
	}

	// Set the file descriptor fields for this session
	pcapsession->fd = pcap_get_selectable_fd(pcapsession->pcap_handle);
	pcapsession->supervise_fd = PCAP_SESSION_SUPERVISED_FD;

	// Clear other fields on this session for now
	pcapsession->monitor = NULL;
	pcapsession->pcap_dumper = NULL;
	pcapsession->handler = NULL;
	pcapsession->untunnel = PCAP_SESSION_UNTUNNEL_OFF;
	pcapsession->iterations = 0;

	// Return the result of adding the new pcapsession
	return pcapsession_handling_add(pcapsession->id);
}

//
// This function kicks off the PCAP live capture thread, it sets the session to state PCAP_SESSION_RUNNING
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on thread initiation, set to a pcapsession_t* here, points at capture session
//
void* pcapsession_livecapture_run(void* pcapsession_param)
{
	// Dereference the pcapsession pointer
	pcapsession_t* pcapsession = pcapsession_param;

	if (pcapsession == NULL) {
		write_to_syslog( "could not run packet capture, server not set\n");
		return NULL;
	}

	write_to_syslog( "packet capture session started: %d-%s\n", pcapsession->id, pcapsession->description);

	// Set the session state to run, run has been ordered
	pcapsession_change_state(pcapsession->id, PCAP_SESSION_RUNNING);

	// Set the monitor for this client
	pcapsession->monitor = monitor_open(pcapsession->id, pcapsession->description);

	pcap_loop(pcapsession->pcap_handle, PCAP_INFINITE, pcapsession_clientconn_packet_handler, (void*)pcapsession);

	// Packet capture has been interrupted
	write_to_syslog( "packet capture interrupted on session: %d-%s\n", pcapsession->id, pcapsession->description);
    
    // Close packet capture
	pcap_close(pcapsession->pcap_handle);
	pcapsession->pcap_handle = NULL;

	// Set the state to terminating
	pcapsession_change_state(pcapsession->id, PCAP_SESSION_TERMINATE);

	return NULL;
}

//
// This function stops the live capture session, the state is reset back to PCAP_SESSION_START so that session handling will attempt to
// restart the server.
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on session stop, set to a pcapsession_t* here, points at session
//
void* pcapsession_livecapture_stop(void* pcapsession_param)
{
	// Dereference the pcapsession pointer
	pcapsession_t* pcapsession = pcapsession_param;

	if (pcapsession == NULL) {
		write_to_syslog( "could not stop packet capture session thread, client connection not set\n");
		return NULL;
	}

	write_to_syslog( "packet capture session stopping: %d-%s\n", pcapsession->id, pcapsession->description);

	// Interrupt packet reception and close the PCAP handle
	if (pcapsession->pcap_handle != NULL) {
		pcap_breakloop(pcapsession->pcap_handle);
		pcap_close(pcapsession->pcap_handle);
		pcapsession->pcap_handle = NULL;
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

	write_to_syslog( "packet capture session stopped: %d-%s\n", pcapsession->id, pcapsession->description);
	return NULL;
}
