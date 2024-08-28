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
* File: pcapsession.h
* Date: Feb 17, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

/**
 * This module manages the life cycle of PCAP sessions
 */

#ifndef PCAPSESSION_H_
#define PCAPSESSION_H_

#include <pthread.h>
#include <netinet/in.h>
#include <pcap/pcap.h>
#include <tcp.h>

#include <monitor.h>

// The maximum number of PCAP sessions allowable
#define PCAP_SESSION_MAX_SESSIONS 128

// The supervision interval for PCAP session supervision in seconds
#define PCAP_SESSION_SUPERVISION_INTERVAL   1

// The number of supervision intervals allowed for state transitions
#define PCAP_SESSION_SUPERVISION_TRANSITION_INTERVALS   10

// PCAP session states
#define PCAP_SESSION_UNUSED       0
#define PCAP_SESSION_STOPPED      1
#define PCAP_SESSION_START        2
#define PCAP_SESSION_STARTING     3
#define PCAP_SESSION_RUNNING      4
#define PCAP_SESSION_TERMINATE    5
#define PCAP_SESSION_ABORT        6
#define PCAP_SESSION_TERMINATING  7
#define PCAP_SESSION_ABORTING     8

// Used to indicate an invalid PCAP session ID
#define PCAP_SESSION_INVALID -1

// Flags for file descriptor supervision
#define PCAP_SESSION_UNSUPERVISED_FD 0
#define PCAP_SESSION_SUPERVISED_FD   1

// Flags for untunnelling
#define PCAP_SESSION_UNTUNNEL_OFF 0
#define PCAP_SESSION_UNTUNNEL_ON  1

// Flags for iterations
#define PCAP_SESSION_ITERATE_INFINITY -1

// Typedef for a pcapsession_run() method that should be implemented on all modules that implement pcapsession, it is called
// in a new thread when the session is started
typedef void* (*pcapsession_run_function)(void *pcapsession);

// Typedef for a pcapsession_stop() method that should be implemented on all modules that implement pcapsession, it is called
// when a session is closed
typedef void* (*pcapsession_stop_function)(void *pcapsession);

// Define a struct that describes a PCAP session
struct pcapsession {
	int id;                                // The ID of the session
	char description[FILENAME_MAX];        // The session description
	int state;                             // The state of the session (Starting/Running/Terminate/Terminating)
	pthread_t thread;                      // The thread in which the session is running
	int fd;                                // The file descriptor associated with this session
	int supervise_fd;                      // A flag indicating if the file descriptor should be supervised on this session
	struct sockaddr_in address;            // The address of the session
	struct monitor* monitor;               // The monitor for this server
	pcap_t* pcap_handle;                   // The PCAP handle for this session
	pcap_dumper_t* pcap_dumper;            // The PCAP dumper for this session, if applicable
	pthread_mutex_t pcap_mutex;            // esirich DEFTFTS-1634 protect pcap handle
	pcapsession_run_function runner;       // The function to run when the session starts
	pcapsession_stop_function stopper;     // The function to run when the session starts
	void* handler;                         // The function to handle a received packet, set to a merger if used
	int interval_counter;                  // An interval counter for the number of intervals taken in transitions
	int untunnel;                          // Indicates whether packets dumped on this session should be untunnelled
	int iterations;                        // The number of iterations to carry out on this session
};

// Typedef for passing sessions into and out of the functions here
typedef struct pcapsession pcapsession_t;
    
// Hold a reference to the servers
pcapsession_t session_list[PCAP_SESSION_MAX_SESSIONS];

//
// This function initializes session handling, allocating memory for the session list and kicking off
// a supervising thread
//
// Return:
//  int: 1 if session handling started successfully, 0 otherwise
//
int pcapsession_handling_init(pthread_t *thread);

//
// This function closes session handling and frees memory for the server list
//
void pcapsession_handling_close();

//
// This function checks if session handling is running
//
// Return:
//  int : One of the PCAP_SESSION_ states from pcapsession.h
//
int pcapsession_handling_state();

//
// This function gets a new PCAP session and returns its ID
//
// Return:
//  pcapsession_t* pcapsession: a pointer to a PCAP session or NULL if all sessions are used
//
pcapsession_t* pcapsession_handling_get_new(void);

//
// This function adds a PCAP session to PCAP session handling
//
// Parameters:
//  int session_id: The ID of the PCAP session to start
//
// Return:
//  int: 1 means the session was added
//
int pcapsession_handling_add(int session_id);

//
// This function opens a distribution server
//
// Parameters:
//  int port: The port on which PCAP stream clients connect
//
// Returns:
//  int: Returns a value of 1 if the session server is created and added to session handling
//
int pcapsession_server_open(int port);

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
pcapsession_t* pcapsession_merger_open(char* filename, int untunnel);

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
int pcapsession_clientconn_open(int client_socket_fd, struct sockaddr_in client_address);

//
// This function opens a PCAP live capture session
//
// Parameters:
//  char* interface_name: The interface on which to start live capture
//
// Returns:
//  int: Returns a value of 1 if the live capture session is created and added to session handling
//
int pcapsession_livecapture_open(char* interface_name);

//
// This function opens a PCAP file capture session
//
// Parameters:
//  char* directory_name: The directory containing PCAP files to be streamed in
//  int iterations: The number of iterations to use over the files
//
// Returns:
//  int: Returns a value of 1 if the file capture session is created and added to session handling
//
int pcapsession_filecapture_open(char* directory_name, int iterations);

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
int pcapsession_client_open(struct sockaddr_in server_address, pcapsession_t* pcapsession_merger);

//
// This function is a PCAP packet handler call back method for packet distribution on clients
//
// Parameters:
//  unsigned char* pcapsession_param: A pointer to user data set in the pcap_loop call, in this case it is a pcapsession_t pointer
//  const struct pcap_pkthdr* header: A pointer to the header of the packet
//  const unsigned char* data: A pointer to the packet data
//
void pcapsession_clientconn_packet_handler(unsigned char* pcapsession_param, const struct pcap_pkthdr* header, const unsigned char* data);

//
// This function is a PCAP packet handler callback method for packet merging
//
// Parameters:
//  unsigned char* pcapsession_param: A pointer to user data set in the pcap_loop call, in this case it is a pcapsession_t pointer
//  const struct pcap_pkthdr* header: A pointer to the header of the packet
//  const unsigned char* data: A pointer to the packet data
//
void pcapsession_merger_packet_handler(unsigned char* pcapsession_param, const struct pcap_pkthdr* header, const unsigned char* data);

//
// This function untunnels GTP-U packets by moving the enclosed IP header up to just under the Ethernet header
//
// Parameters:
//  pcapsession_t*: Returns a pointer to the merger or NULL if opening failed
//  struct pcap_pkthdr* header: A pointer to the header of the packet
//  const unsigned char* data: A pointer to the packet data
//
void pcapsession_untunnel_packet(pcapsession_t* pcapsession, struct pcap_pkthdr* header, const unsigned char* data);

//
// This function is used to change the state of a PCAP session
//
// Parameters:
//  int session_id: The ID of the PCAP session of which to change state
//  int nesw_state: The new state of the PCAP session
//
void pcapsession_change_state(int id, int new_state);

#endif /* PCAPSESSION_H_ */
