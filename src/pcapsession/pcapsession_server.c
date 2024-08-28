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
 * File: pcapsession_server.c
 * Date: Feb 15, 2012
 * Author: LMI/LXR/SH Liam Fallon
 ************************************************************************/

/**
 * This module handles the server that listens for client connections for PCAP streams
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <logger.h>
#include <pcapsession.h>

// The maximum number of queued client listen requests to allow
#define MAX_CLIENT_BACKLOG 5

// Forward definition of private functions
void* pcapsession_server_run(void* pcapsession_param);
void* pcapsession_server_stop(void* pcapsession_param);

//
// This function opens a distribution server
//
// Parameters:
//  int port: The port on which PCAP stream clients connect
//
// Returns:
//  int: Returns a value of 1 if the session server is created and added to session handling
//
int pcapsession_server_open(int port)
{
    char tcp_errbuf[TCP_ERRBUF_SIZE];
	write_to_syslog( "opening server session on port %d\n", port);

	// Get and check if a new PCAP session is available
	pcapsession_t* pcapsession = pcapsession_handling_get_new();
	if (pcapsession == NULL) {
		return 0;
	}

	// Set the run and stop methods for the session
	pcapsession->runner  = pcapsession_server_run;
	pcapsession->stopper = pcapsession_server_stop;

	// Clear other fields on this session for now
	pcapsession->monitor = NULL;
	pcapsession->pcap_handle = NULL;
	pcapsession->pcap_dumper = NULL;
	pcapsession->handler = NULL;
	pcapsession->untunnel = PCAP_SESSION_UNTUNNEL_OFF;
	pcapsession->iterations = 0;

	// Try to open a socket for the server
    memset(tcp_errbuf,0,TCP_ERRBUF_SIZE);
	pcapsession->fd = open_server_socket(port,&pcapsession->address,tcp_errbuf,TCP_ERRBUF_SIZE);
	if (pcapsession->fd < 0) {
		write_to_syslog( "could not open socket:\n%s",tcp_errbuf);
        pcapsession->fd = 0;
		pcapsession_change_state(pcapsession->id, PCAP_SESSION_UNUSED);
		return 0;
	}
	pcapsession->supervise_fd = PCAP_SESSION_UNSUPERVISED_FD;

	// Set the description of the session
	sprintf(pcapsession->description, "%s:%d", inet_ntoa(pcapsession->address.sin_addr), ntohs(pcapsession->address.sin_port));
    write_to_syslog( "server session %d-%s: waiting for client connections\n", pcapsession->id, pcapsession->description);

	// Return the result of adding the new pcapsession
	return pcapsession_handling_add(pcapsession->id);
}

//
// This function kicks off the server socket acceptor thread, it sets the session to state PCAP_SESSION_RUNNING
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on thread initiation, set to a pcapsession_t* here, points at distribution server
//
void* pcapsession_server_run(void* pcapsession_param)
{
	// Dereference the pcapsession pointer
	pcapsession_t* pcapsession = pcapsession_param;
    int result=0;
    char tcp_errbuf[TCP_ERRBUF_SIZE];

	if (pcapsession == NULL) {
		write_to_syslog( "could not run server session thread, server not set\n");
		return NULL;
	}

	write_to_syslog( "server session started: %d-%s\n", pcapsession->id, pcapsession->description);

	// Set the session state to run, run has been ordered
	pcapsession_change_state(pcapsession->id, PCAP_SESSION_RUNNING);

	// Listen forever (well, until interrupted) for clients
	while (pcapsession->state == PCAP_SESSION_RUNNING) {
		// Accept the connection from the client
        memset(tcp_errbuf,0,TCP_ERRBUF_SIZE);
        result = poll_server_socket(pcapsession->fd,pcapsession_clientconn_open,tcp_errbuf,TCP_ERRBUF_SIZE);
        if(result<=0)
        {
            pcapsession_change_state(pcapsession->id, PCAP_SESSION_TERMINATE);
            if(result<0)
            {
                write_to_syslog( "server session rejected: %s\n", tcp_errbuf);
                break;
            }   
            write_to_syslog( "server session %d-%s: client connection rejected\n", pcapsession->id, pcapsession->description);
        }
        write_to_syslog( "server session %d-%s: client connection handled\n", pcapsession->id, pcapsession->description);
	}
	return NULL;
}

//
// This function stops the server, the state is reset back to PCAP_SESSION_START so that session handling will attempt to
// restart the server
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on session stop, set to a pcapsession_t* here, points at the server session
//
void* pcapsession_server_stop(void* pcapsession_param)
{
	// Dereference the pcapsession pointer
	pcapsession_t* pcapsession = pcapsession_param;
    char tcp_errbuf[TCP_ERRBUF_SIZE];

	if (pcapsession == NULL) {
		write_to_syslog( "could not stop server session, server not set\n");
		return NULL;
	}

	write_to_syslog( "server session stopping: %d-%s\n", pcapsession->id, pcapsession->description);

	// Check if the session file descriptor is set
	if (pcapsession->fd > 0) {
		// Close the descriptor and clear it
        memset(tcp_errbuf,0,TCP_ERRBUF_SIZE);
		close_server_socket(pcapsession->fd,tcp_errbuf,TCP_ERRBUF_SIZE);
		pcapsession->fd = 0;
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

	write_to_syslog( "server session stopped: %d-%s\n", pcapsession->id, pcapsession->description);
	return NULL;
}
