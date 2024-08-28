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
 * File: pcapsession.c
 * Date: Feb 17, 2012
 * Author: LMI/LXR/SH Liam Fallon
 ************************************************************************/

/**
 * This module manages the life cycle of PCAP sessions, and supervises all active sessions. it
 * also handles startup and shutdown of sessions.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <tcp.h>
#include <logger.h>
#include <monitor.h>
#include <pcapsession.h>

// A flag indicating the state of session handling
static int session_handling_state = PCAP_SESSION_STOPPED;

// Forward references of internal functions
void* pcapsession_supervision_run(void* notused_param);
void pcapsession_transition_session(int session_no);

// Mutex associated with changing state
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;

//
// This function initializes session handling and kicks off
// a supervising thread
//
// Return:
//  int: 1 if session handling started successfully, 0 otherwise
//
int pcapsession_handling_init(pthread_t *supervision_thread)
{
    if(session_handling_state==PCAP_SESSION_STOPPED)
    {
        session_handling_state = PCAP_SESSION_STARTING;
        
        pthread_mutex_init(&state_mutex,NULL);
        
	    // Clear the data of the server list
	    memset(session_list, 0, sizeof(pcapsession_t) * PCAP_SESSION_MAX_SESSIONS);

	    // Set the state of all sessions as unused
	    for (int session_id = 0; session_id < PCAP_SESSION_MAX_SESSIONS; session_id++) {
		    pcapsession_change_state(session_id, PCAP_SESSION_UNUSED);
	    }
	    write_to_syslog( "spawning new supervision thread for session handling\n");

	    // The supervision thread accepts and administers sessions
	    if (pthread_create(supervision_thread, NULL, pcapsession_supervision_run, NULL) != 0) {
		    write_to_syslog( "failed to spawn new supervision thread for session handling\n");
		    return 0;
	    }

	    // Wait for the session to change from state STARTING
	    while (session_handling_state == PCAP_SESSION_STARTING)
		    sleep(1); // Wait 1 second

	    write_to_syslog( "spawned new supervision thread for session handling\n");
    }
    else
        write_to_syslog( "supervision thread has already been spawned\n");
    
	return 1;
}

//
// This function cleans up on shutdown
//
void pcapsession_handling_close()
{
    session_handling_state=PCAP_SESSION_TERMINATING;
}

//
// This function gets a new PCAP session and returns its ID
//
// Return:
//  int: the number of a new PCAP session or PCAP_SESSION_INVALID if all sessions are used
//
pcapsession_t* pcapsession_handling_get_new(void)
{
	pthread_mutex_t mutex_init = PTHREAD_MUTEX_INITIALIZER;

	// Add the session
	int session_id;
	for (session_id = 0; session_id < PCAP_SESSION_MAX_SESSIONS; session_id++) {
		// Check for an empty session slot
		if (session_list[session_id].state == PCAP_SESSION_UNUSED) {
			break;
		}
	}

	// Check if a slot was found
	if (session_id >= PCAP_SESSION_MAX_SESSIONS) {
		write_to_syslog( "PCAP session not added, max number %d sessions running\n", PCAP_SESSION_MAX_SESSIONS);
		return NULL;
	}

	// Clear the new PCAP session struct
	memset(&session_list[session_id], 0 , sizeof(pcapsession_t));

	// Set the session ID and clear its description
	session_list[session_id].id = session_id;
	session_list[session_id].description[0] = 0;
	memcpy(&(session_list[session_id].pcap_mutex), &mutex_init, sizeof(mutex_init));  // esirich DEFTFTS-1634

	// Set the state of this session
	pcapsession_change_state(session_id, PCAP_SESSION_STOPPED);

	return &session_list[session_id];
}

//
// This function adds a PCAP session to PCAP session handling
//
// Parameters:
//  int session_id: The ID of the PCAP session to start
//
// Return:
//  int: 1 means the session was added
//
int pcapsession_handling_add(int session_id)
{
	// Check the PCAP session pointer passed in
	if (session_id < 0 || session_id >= PCAP_SESSION_MAX_SESSIONS) {
		write_to_syslog( "PCAP session not added, invalid session id %d specified\n", session_id);
		return 0;
	}

	// Add the session
	pcapsession_change_state(session_id, PCAP_SESSION_START);
	session_list[session_id].id = session_id;
	write_to_syslog( "PCAP session %s added to session handling as session %d\n",
			session_list[session_id].description, session_id);

	return 1;
}

//
// This function starts a PCAP session in a new thread, it calls the pcapsession_run() function in the
// PCAP session
//
// Parameters:
//  int session_id: The ID of the PCAP session to start
//
// Return:
//  int: 1 if the session start was ordered, 0 otherwise
//
int pcapsession_start(int session_id)
{
	// Check the PCAP session pointer passed in
	if (session_list[session_id].state != PCAP_SESSION_STARTING) {
		write_to_syslog( "PCAP session %d in invalid state %d, could not spawn new session\n", session_id, session_list[session_id].state);
		return 0;
	}

	write_to_syslog( "spawning new thread for PCAP session: %d-%s\n", session_id, session_list[session_id].description);

	// The socket server thread accepts and administers connections from clients
	if (pthread_create(&session_list[session_id].thread, NULL, session_list[session_id].runner, &session_list[session_id]) != 0) {
		write_to_syslog( "failed to spawn new thread for PCAP session: %d-%s\n", session_id, session_list[session_id].description);
		return 0;
	}

	write_to_syslog( "spawned new thread for PCAP session: %d-%s\n", session_id, session_list[session_id].description);
	return 1;
}

//
// This function closes the distribution server
//
// Parameters:
//  int session_id: The ID of the PCAP session to start
//
void pcapsession_stop(int session_id)
{
	write_to_syslog( "closing PCAP session: %d-%s\n", session_id, session_list[session_id].description);

	// Check if the socket thread is still running
	if (session_list[session_id].thread != 0) {
		write_to_syslog( "cancelling thread for PCAP session: %d-%s\n", session_id, session_list[session_id].description);

		// Cancel the thread, then join the thread in order to wait until it finishes
		pthread_cancel(session_list[session_id].thread);
		pthread_join(session_list[session_id].thread, NULL);

		// Clear the thread variable
		session_list[session_id].thread = 0;

		write_to_syslog( "cancelled thread for PCAP session: %d-%s\n", session_id, session_list[session_id].description);
	}

	// Call the session stop function
	session_list[session_id].stopper(&session_list[session_id]);

	// Clear this session and set it as unused
	if (session_list[session_id].state == PCAP_SESSION_STOPPED) {
		pcapsession_change_state(session_id, PCAP_SESSION_UNUSED);
	}

	// Session is closed
	write_to_syslog( "closed PCAP session: %d-%s\n", session_id, session_list[session_id].description);
}

//
// This function kicks off the server socket acceptor thread
//
// Parameters:
//  void* notused_param: A transparent parameter on thread initiation, not used here
//
void* pcapsession_supervision_run(void* notused_param)
{
	// Set the state as running
	session_handling_state = PCAP_SESSION_RUNNING;

	// Supervise forever (well, until interrupted)
	while (1) {
		// Iterate over each session and trigger handling of sessions that are open
		int sessioncount = 0;
		for (int i = 0; i < PCAP_SESSION_MAX_SESSIONS; i++) {
			// Check if this session is initiated
			if (session_list[i].state == PCAP_SESSION_UNUSED) {
				continue;
			}

			// Iterate the number of running sessions
			sessioncount++;

			// Check if session handling is terminating
			if (session_handling_state != PCAP_SESSION_RUNNING) {
				// Check if the session is already aborting or stopped
				if (session_list[i].state != PCAP_SESSION_STOPPED && session_list[i].state != PCAP_SESSION_ABORTING) {
					pcapsession_change_state(i, PCAP_SESSION_ABORT);
				}
			}

			// Check if this FD should be supervised; do not supervise fd 0-2 (Standard fds)
			if (session_list[i].supervise_fd == PCAP_SESSION_SUPERVISED_FD && session_list[i].fd > 2) {
				// Check if the file descriptor is open
				if (!iotests_fd_open(session_list[i].fd)) {
					write_to_syslog( "session %d-%s: file descriptor is not open\n", i, session_list[i].description);
					pcapsession_change_state(i, PCAP_SESSION_TERMINATE);
				}
			}

			// Run state transitions for the session
			pcapsession_transition_session(i);

			// Only allow running sessions to carry on
			if (session_list[i].state != PCAP_SESSION_RUNNING) {
				continue;
			}

			// esirich DEFTFTS-1634 if the output is idle, flush it
			if (session_list[i].monitor != NULL
			&& session_list[i].pcap_dumper 
			&& session_list[i].monitor->bytes == 0) {
				pthread_mutex_lock(&(session_list[i].pcap_mutex));
				pcap_dump_flush(session_list[i].pcap_dumper);
				pthread_mutex_unlock(&(session_list[i].pcap_mutex));
			}

			// Check if a monitor should be output
			if (session_list[i].monitor != NULL) {
				handle_monitor(session_list[i].monitor);
			}

		}

		// Check if there are any sessions running
		if (sessioncount == 0 && session_handling_state != PCAP_SESSION_RUNNING) {
			// No sessions running and session handling state is not running, return
			break;
		}

		// All sessions have been supervised, sleep for the next period
		sleep(PCAP_SESSION_SUPERVISION_INTERVAL);
	}

	// Set the state as stopped
	session_handling_state = PCAP_SESSION_STOPPED;

	return NULL;
}

//
// This function handles state transitions for PCAP sessions
//
// Parameters:
//  int session_no: The position of the session on the session list
//
void pcapsession_transition_session(int session_id)
{
	// Transitions for all pcapsession states
	switch (session_list[session_id].state) {

	// Session is unused or stopped, ignore
	case PCAP_SESSION_UNUSED:
	case PCAP_SESSION_STOPPED: {
		return;
	}

	// Session is awaiting start, start it
	case PCAP_SESSION_START: {
		pcapsession_change_state(session_id, PCAP_SESSION_STARTING);
		session_list[session_id].interval_counter = 0;
		pcapsession_start(session_id);
		return;
	}

	// Session is still starting
	case PCAP_SESSION_STARTING: {
		// Check for transition timeout
		if (session_list[session_id].interval_counter >= PCAP_SESSION_SUPERVISION_TRANSITION_INTERVALS) {
			write_to_syslog( "session %d-%s: start timed out\n", session_id, session_list[session_id].description);
			// Transition has timed out, terminate the session
			pcapsession_change_state(session_id, PCAP_SESSION_TERMINATE);
		}
		else {
			// Session start has not timed out yet, wait longer
			session_list[session_id].interval_counter++;
		}
		return;
	}

	// Everything is good, do nothing
	case PCAP_SESSION_RUNNING: {
		return;
	}

	// Normal termination of a session
	case PCAP_SESSION_TERMINATE: {
		pcapsession_change_state(session_id, PCAP_SESSION_TERMINATING);
		session_list[session_id].interval_counter = 0;
		pcapsession_stop(session_id);
		return;
	}

	// Abort a session
	case PCAP_SESSION_ABORT: {
		pcapsession_change_state(session_id, PCAP_SESSION_ABORTING);
		session_list[session_id].interval_counter = 0;
		pcapsession_stop(session_id);
		return;
	}

	// Session is still terminating or aborting
	case PCAP_SESSION_TERMINATING:
	case PCAP_SESSION_ABORTING:
	{
		// Check for transition timeout
		if (session_list[session_id].interval_counter >= PCAP_SESSION_SUPERVISION_TRANSITION_INTERVALS) {
			write_to_syslog( "session %d-%s: terminate/abort timed out\n", session_id, session_list[session_id].description);
			// Transition has timed out, stop the session
			// This does not take care of case where sessions should be restarted
			pcapsession_change_state(session_id, PCAP_SESSION_STOPPED);
			pcapsession_stop(session_id);
		}
		else {
			// Session start has not timed out yet, wait longer
			session_list[session_id].interval_counter++;
		}
		return;
	}

	default: {
		// Stop the session
		pcapsession_change_state(session_id, PCAP_SESSION_STOPPED);
		pcapsession_stop(session_id);
		return;
	}
	}
}

//
// This function is used to change the state of a PCAP session
//
// Parameters:
//  int session_id: The ID of the PCAP session of which to change state
//  int nesw_state: The new state of the PCAP session
//
void pcapsession_change_state(int session_id, int new_state)
{
	//write_to_syslog( "session %d-%s: state %d->%d\n", session_id, session_list[session_id].description, session_list[session_id].state, new_state);
    pthread_mutex_lock(&state_mutex);
	session_list[session_id].state = new_state;
    pthread_mutex_unlock(&state_mutex);
}
