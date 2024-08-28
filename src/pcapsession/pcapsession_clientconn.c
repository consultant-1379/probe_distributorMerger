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
 * File: pcapsession_clientconn.c
 * Date: Feb 15, 2012
 * Author: LMI/LXR/SH Liam Fallon
 ************************************************************************/

/**
 * This module handles client connections to the distribution server, and dumps captured
 * PCAP streams to each client
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <logger.h>
#include <monitor.h>
#include <tcp.h>
#include <pcapdefines.h>
#include <pcapsession.h>

// Hold a reference to the clients
pcapsession_t* clientconnlist[PCAP_SESSION_MAX_SESSIONS];

// Flag indicating if the client connection list has been initialized
int clientconnlist_initialized = 0;

// Forward definition of private functions
void* pcapsession_clientconn_run(void* pcapsession_param);
void* pcapsession_clientconn_stop(void* pcapsession_param);
void pcapsession_clientconn_client_handle_packet(pcapsession_t* client, const struct pcap_pkthdr* header, const unsigned char* data);

//
// This function handles a new client connection accepted on the server socket
//
// Parameters:
//  int client_socket_fd: The file descriptor of the client socket
//  struct sockaddr_in* client_address: The address information for the client
//
// Return:
//  int: 1 if the client connection was accepted, 0 otherwise
//
int pcapsession_clientconn_open(int client_socket_fd, struct sockaddr_in client_address)
{
	write_to_syslog( "opening client connection session %s:%d\n",
			inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

	// Check if the list has been initialized
	if (!clientconnlist_initialized) {
		// Clear all the client connection pointers
		for (int i = 0; i < PCAP_SESSION_MAX_SESSIONS; i++) {
			clientconnlist[i] = NULL;
		}

		// List has now been initialized
		clientconnlist_initialized = 1;
	}

	// Get and check if a new PCAP session is available
	pcapsession_t* pcapsession = pcapsession_handling_get_new();
	if (pcapsession == NULL) {
		return 0;
	}

	// Set the run and stop methods for the session
	pcapsession->runner  = pcapsession_clientconn_run;
	pcapsession->stopper = pcapsession_clientconn_stop;

	// Set the connection fd and address
	pcapsession->fd = client_socket_fd;
	pcapsession->supervise_fd = PCAP_SESSION_SUPERVISED_FD;
	pcapsession->address = client_address;

	// Set the description of the session
	sprintf(pcapsession->description, "%s:%d", inet_ntoa(pcapsession->address.sin_addr), ntohs(pcapsession->address.sin_port));

	// Clear other fields on this session for now
	pcapsession->monitor = NULL;
	pcapsession->pcap_handle = NULL;
	pcapsession->pcap_dumper = NULL;
	pcapsession->handler = NULL;
	pcapsession->untunnel = PCAP_SESSION_UNTUNNEL_OFF;
	pcapsession->iterations = 0;

	// Return the result of adding the new pcapsession
	return pcapsession_handling_add(pcapsession->id);
}

//
// This function kicks off packet dumping onto a client connection in a new thread
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on thread initiation, set to a pcapsession_t* here, points at a client connection
//
void* pcapsession_clientconn_run(void* pcapsession_param)
{
	// Dereference the pcapsession pointer
	pcapsession_t* pcapsession = pcapsession_param;

	if (pcapsession == NULL) {
		write_to_syslog( "could not run client connection session, client connection not set\n");
		return NULL;
	}

	write_to_syslog( "client connection session started: %d-%s\n", pcapsession->id, pcapsession->description);

	// Set the session state to run, run has been ordered
	pcapsession_change_state(pcapsession->id, PCAP_SESSION_RUNNING);

	// Set the monitor for this client
	pcapsession->monitor = monitor_open(pcapsession->id, pcapsession->description);

	// We have to give pcap_dump_open an input file handle so we use a dead open with an Ethernet PCAP type
	pcap_t* pcap_dead_handle = pcap_open_dead(DLT_EN10MB, PCAP_MAX_SNAPLEN);

	// Get a FILE handle from the open socket file descriptor and open packet dumping to the client
	FILE *client_fd = fdopen(pcapsession->fd, "wb");
	pcapsession->pcap_dumper = pcap_dump_fopen(pcap_dead_handle, client_fd);
    if(pcap_dead_handle != NULL)
        pcap_close(pcap_dead_handle);

	if (pcapsession->pcap_dumper == NULL) {
		write_to_syslog( "client connection on session %d-%s: packet dump open failed, %s\n",
					pcapsession->id, pcapsession->description, pcap_geterr(pcap_dead_handle));
		pcapsession_change_state(pcapsession->id, PCAP_SESSION_TERMINATE);
	}
	else {
		// Set the pcapsession in the list of client connections that are open
		clientconnlist[pcapsession->id] = pcapsession;

		write_to_syslog( "client connection on session connected: %d-%s\n", pcapsession->id, pcapsession->description);
	}

	// Happy days, client is open
	return NULL;
}

//
// This function stops packet dumping to a client, the state is reset back to PCAP_SESSION_STOP, the client side is responsible
// for ensuring the client comes up again
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on session stop, set to a pcapsession_t* here, points at live capture session
//

void* pcapsession_clientconn_stop(void* pcapsession_param)
{
	// Dereference the pcapsession pointer
	pcapsession_t* pcapsession = pcapsession_param;

	if (pcapsession == NULL) {
		write_to_syslog( "could not stop client connection session thread, client connection not set\n");
		return NULL;
	}

	write_to_syslog( "client connection session stopping: %d-%s\n", pcapsession->id, pcapsession->description);

	// Remove this session pointer from the client list
	clientconnlist[pcapsession->id] = NULL;

	// Turn off packet dumping on this client, make sure the PCAP dump file is open before attempting close
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

	// Set the session state as stopped, the client side must recover connections
	pcapsession_change_state(pcapsession->id, PCAP_SESSION_STOPPED);

	write_to_syslog( "client connection session stopped: %d-%s\n", pcapsession->id, pcapsession->description);
	return NULL;
}

//
// This function is a PCAP packet handler call back method for packet distribution on clients
//
// Parameters:
//  unsigned char* distserver_param: A pointer to user data set in the pcap_loop call, in this case it is a pcapsession_t pointer
//  const struct pcap_pkthdr* header: A pointer to the header of the packet
//  const unsigned char* data: A pointer to the packet data
//
void pcapsession_clientconn_packet_handler(unsigned char* pcapsession_param, const struct pcap_pkthdr* header, const unsigned char* data)
{
    // Sanity check for client initiation
	if (clientconnlist == NULL) {
		return;
	}
    
	// Sanity check for initiation
	if (pcapsession_param == NULL) {
		return;
	}

	// Dereference the pcapsession_t pointer
	pcapsession_t* pcapsession = (pcapsession_t*)pcapsession_param;

	// Add a packet and the number of bytes to the monitor for the server
	monitor_increment(pcapsession->monitor, 1, header->len);

	// Iterate over each client and send the packet to each one
	for (int i = 0; i < PCAP_SESSION_MAX_SESSIONS; i++) {
		// Check if the client is used
		if (clientconnlist[i] != NULL) {
			pcapsession_clientconn_client_handle_packet(clientconnlist[i], header, data);
		}
	}
}

//
// This function is a PCAP packet handler for an individual client
//
// Parameters:
//  distserverclient_t* client: The client for which data is being handled
//  const struct pcap_pkthdr* header: A pointer to the header of the packet
//  const unsigned char* data: A pointer to the packet data
//
void pcapsession_clientconn_client_handle_packet(pcapsession_t* client, const struct pcap_pkthdr* header, const unsigned char* data)
{
	// Check if the client handle is set
	if (client == NULL || client->state != PCAP_SESSION_RUNNING) {
		return;
	}

	// Dump the packet to the client in question
	if (iotests_fd_open(client->fd)) {
		
		// esirich DEFTFTS-1634 lock the pcap_dump so the monitor can flush
		pthread_mutex_lock(&(client->pcap_mutex));
		pcap_dump((unsigned char*)client->pcap_dumper, header, data);
		pthread_mutex_unlock(&(client->pcap_mutex));

        // Add a packet and the number of bytes to the monitor for the client
        monitor_increment(client->monitor, 1, header->len);
	}
	else {
		// Get client off list for packet dumping ASAP
		clientconnlist[client->id] = NULL;
		// Client disconnect detected, terminate
		// write_to_syslog( "client connection session file descriptor is lost: %d-%s\n", client->id, client->description);
	}
}
