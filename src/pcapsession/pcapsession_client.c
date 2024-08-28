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
 * File: pcapsession_client.c
 * Date: Feb 16, 2012
 * Author: LMI/LXR/SH Liam Fallon
 ************************************************************************/

/**
 * This module receives packets on incoming streams and forwards them for output onto
 * a single merger
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <tcp.h>
#include <logger.h>
#include <pcapdefines.h>
#include <pcapsession.h>

// Forward definition of private functions
void* pcapsession_client_run(void* pcapsession_param);
void* pcapsession_client_stop(void* pcapsession_param);

//
// This function opens a new server socket connection
//
// Parameters:
//  struct sockaddr_in server_address: The address information for the server to connect to
//  pcapsession_t* pcapsession_merger: The merger to merge packet sessions onto
//
// Return:
//  int: 1 if the server connection was completed, 0 otherwise
//
int pcapsession_client_open(struct sockaddr_in server_address, pcapsession_t* pcapsession_merger)
{
	write_to_syslog( "opening server connection session %s:%d\n",
			inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));

	// Get and check if a new PCAP session is available
	pcapsession_t* pcapsession = pcapsession_handling_get_new();
	if (pcapsession == NULL) {
		return 0;
	}

	// Set the run and stop methods for the session
	pcapsession->runner  = pcapsession_client_run;
	pcapsession->stopper = pcapsession_client_stop;

	// Set the merger for this client to use
	pcapsession->handler = pcapsession_merger;

	// Set the connection fd and address
	pcapsession->address = server_address;

	// Set the description of the session
	sprintf(pcapsession->description, "%s:%d", inet_ntoa(pcapsession->address.sin_addr), ntohs(pcapsession->address.sin_port));

	// Clear other fields on this session for now
	pcapsession->fd = 0;
	pcapsession->supervise_fd = PCAP_SESSION_SUPERVISED_FD;
	pcapsession->monitor = NULL;
	pcapsession->pcap_handle = NULL;
	pcapsession->pcap_dumper = NULL;
	pcapsession->untunnel = PCAP_SESSION_UNTUNNEL_OFF;
	pcapsession->iterations = 0;

	// Return the result of adding the new pcapsession
	return pcapsession_handling_add(pcapsession->id);
}

//
// This function kicks off packet reception from a server in a new thread
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on thread initiation, set to a pcapsession_t* here, points at a server connection
//
void* pcapsession_client_run(void* pcapsession_param)
{
	// Dereference the pcapsession pointer
	pcapsession_t* pcapsession = pcapsession_param;

	if (pcapsession == NULL) {
		write_to_syslog( "could not run server connection session, server connection not set\n");
		return NULL;
	}

	write_to_syslog( "server connection session started: %d-%s\n", pcapsession->id, pcapsession->description);

	// Set the session state to run, run has been ordered
	pcapsession_change_state(pcapsession->id, PCAP_SESSION_RUNNING);

	write_to_syslog( "server connection session %d-%s: opening connection to server\n", pcapsession->id, pcapsession->description);

	// Get the socket for connecting to the server
	pcapsession->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (pcapsession->fd < 0) {
		write_to_syslog( "server connection session %d-%s: connection to socket failed\n", pcapsession->id, pcapsession->description);
		pcapsession_change_state(pcapsession->id, PCAP_SESSION_TERMINATE);
		return NULL;
	}

	// Connect to the remote server
	if (connect(pcapsession->fd, (struct sockaddr*)&pcapsession->address, sizeof(struct sockaddr_in)) < 0) {
		// Close the descriptor
		write_to_syslog( "server connection session %d-%s: connection to server failed\n", pcapsession->id, pcapsession->description);
		pcapsession_change_state(pcapsession->id, PCAP_SESSION_TERMINATE);
		return NULL;
	}

	write_to_syslog( "server connection session %d-%s: connected to server\n", pcapsession->id, pcapsession->description);

	// Set the monitor for this server session
	pcapsession->monitor = monitor_open(pcapsession->id, pcapsession->description);

	// Open packet capture on the server connection
	write_to_syslog( "server connection session %d-%s: opening packet capture\n", pcapsession->id, pcapsession->description);

	// Open and save a PCAP handle
	const char* mode = "rb";
	FILE *server_fd = fdopen(pcapsession->fd, mode);
	char pcap_errbuf[PCAP_ERRBUF_SIZE];
	pcapsession->pcap_handle = pcap_fopen_offline(server_fd, pcap_errbuf);
	if (pcapsession->pcap_handle == NULL) {
		write_to_syslog( "server connection session %d-%s: packet capture open failed, %s\n",
				pcapsession->id, pcapsession->description, pcap_errbuf);
		pcapsession_change_state(pcapsession->id, PCAP_SESSION_TERMINATE);
		return NULL;
	}

	write_to_syslog( "server connection session %d-%s: packet capture opened\n", pcapsession->id, pcapsession->description);

	// Loop forever (or until interrupted) on server connection
	pcap_loop(pcapsession->pcap_handle, PCAP_INFINITE, pcapsession_merger_packet_handler, (void*)pcapsession);

	// Packet capture has been interrupted
	write_to_syslog( "server connection session %d-%s: packet capture interrupted\n", pcapsession->id, pcapsession->description);
	pcapsession_change_state(pcapsession->id, PCAP_SESSION_TERMINATE);

	return NULL;
}

//
// This function stops packet capture from a server, the state is reset back to PCAP_SESSION_START so that session handling will attempt to
// restart packet capture from the server when the server recovers
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on session stop, set to a pcapsession_t* here, points at live capture session
//
void* pcapsession_client_stop(void* pcapsession_param)
{
	// Dereference the pcapsession pointer
	pcapsession_t* pcapsession = pcapsession_param;

	if (pcapsession == NULL) {
		write_to_syslog( "could not stop server connection session, server connection not set\n");
		return NULL;
	}

	write_to_syslog( "server connection session %d-%s: stopping\n", pcapsession->id, pcapsession->description);

	// Interrupt packet reception and close the PCAP handle
	if (pcapsession->pcap_handle != NULL) {
		pcap_breakloop(pcapsession->pcap_handle);
		pcap_close(pcapsession->pcap_handle);
		pcapsession->pcap_handle = NULL;
	}

	// Close the client file descriptor, make sure it is open first
	if (pcapsession->fd > 0 && iotests_fd_open(pcapsession->fd)) {
		close(pcapsession->fd);
	}
	pcapsession->fd = 0;

	// Close monitoring
	if (pcapsession->monitor != NULL) {
		monitor_close(pcapsession->monitor);
		pcapsession->monitor = NULL;
	}

	// Set the session state as appropriate, client connections that stop will be retried unless aborted
	if (pcapsession->state == PCAP_SESSION_ABORTING) {
		// On abort, always stop
		pcapsession_change_state(pcapsession->id, PCAP_SESSION_STOPPED);
	}
	else {
		// Try to restart the server
		pcapsession_change_state(pcapsession->id, PCAP_SESSION_START);
	}


	write_to_syslog( "server connection session stopped: %d-%s\n", pcapsession->id, pcapsession->description);
	return NULL;
}
