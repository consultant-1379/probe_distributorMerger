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
 * File: pcapsession_filecapture.c
 * Date: Feb 17, 2012
 * Author: LMI/LXR/SH Liam Fallon
 ************************************************************************/

/**
 * This module loops over a set of files in a directory and streams them into the server
 */

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <logger.h>
#include <monitor.h>
#include <pcapdefines.h>
#include <pcapsession.h>

// Forward definition of private functions
void* pcapsession_filecapture_run(void* pcapsession_param);
void* pcapsession_filecapture_stop(void* pcapsession_param);

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
int pcapsession_filecapture_open(char* directory_name, int iterations)
{
	write_to_syslog( "opening file capture session on directory %s\n", directory_name);

	// Get and check if a new PCAP session is available
	pcapsession_t* pcapsession = pcapsession_handling_get_new();
	if (pcapsession == NULL) {
		return 0;
	}

	// Set the run and stop methods for the session
	pcapsession->runner  = pcapsession_filecapture_run;
	pcapsession->stopper = pcapsession_filecapture_stop;

	// Save the directory name for capture as the description
	strcpy(pcapsession->description, directory_name);

	write_to_syslog( "packet capture starting on file capture session: %d-%s\n", pcapsession->id, pcapsession->description);

	// Clear other fields on this session for now
	pcapsession->fd = 0;
	pcapsession->supervise_fd = PCAP_SESSION_UNSUPERVISED_FD;
	pcapsession->monitor = NULL;
	pcapsession->pcap_handle = NULL;
	pcapsession->pcap_dumper = NULL;
	pcapsession->handler = NULL;
	pcapsession->untunnel = PCAP_SESSION_UNTUNNEL_OFF;
	pcapsession->iterations = iterations;

	// Return the result of adding the new pcapsession
	return pcapsession_handling_add(pcapsession->id);
}

//
// This function kicks off the PCAP file capture thread, it sets the session to state PCAP_SESSION_RUNNING
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on thread initiation, set to a pcapsession_t* here, points at capture session
//
void* pcapsession_filecapture_run(void* pcapsession_param)
{
	// Dereference the pcapsession pointer
	pcapsession_t* pcapsession = pcapsession_param;

	if (pcapsession == NULL) {
		write_to_syslog( "could not run packet capture, session not set\n");
		return NULL;
	}

	write_to_syslog( "packet capture session started: %d-%s\n", pcapsession->id, pcapsession->description);

	// Set the session state to run, run has been ordered
	pcapsession_change_state(pcapsession->id, PCAP_SESSION_RUNNING);

	// Set the monitor for this client
	pcapsession->monitor = monitor_open(pcapsession->id, pcapsession->description);

	// Loop continuously over the PCAP files in the directory
	int pcap_file_count;
	do {
		// Decrement the iterations if we're not looping infinitely
		if (pcapsession->iterations != PCAP_SESSION_ITERATE_INFINITY && pcapsession->iterations != 0) {
			pcapsession->iterations--;
		}

		// Clear the file count
		pcap_file_count = 0;

		// Open the directory in which the packet capture files are
		DIR* pcap_directory = opendir(pcapsession->description);
		if (pcap_directory == NULL) {
			write_to_syslog( "packet capture session %d-%s: %s\n", pcapsession->id, pcapsession->description, strerror(errno));
			pcapsession_change_state(pcapsession->id, PCAP_SESSION_TERMINATE);
			return NULL;
		}

		// Contains a directory entry
		struct dirent* pcap_dir_entry = NULL;

		// Loop over every file in the directory
		while ((pcap_dir_entry = readdir(pcap_directory)) != NULL) {
			// Check if this is a PCAP file
			char* pcap_extension = strstr(pcap_dir_entry->d_name, PCAP_FILE_TYPE);
			if (pcap_extension == NULL || strcmp(pcap_extension, PCAP_FILE_TYPE)) {
				continue;
			}

			// We found a PCAP file, count it
			pcap_file_count++;

			// Set the full path to the file
			char file_path[FILENAME_MAX];
			sprintf(file_path, "%s/%s", pcapsession->description, pcap_dir_entry->d_name);

			// Open the PCAP file
			char pcap_errbuf[PCAP_ERRBUF_SIZE];
			pcapsession->pcap_handle = pcap_open_offline(file_path, pcap_errbuf);
			if (pcapsession->pcap_handle == NULL) {
				write_to_syslog( "file capture session: %d-%s: capture start failed on file %s, %s\n",
						pcapsession->id, pcapsession->description, file_path, pcap_errbuf);
				pcapsession_change_state(pcapsession->id, PCAP_SESSION_TERMINATE);
                closedir(pcap_directory);
				return NULL;
			}

			// Set the file descriptor fields for this file
			pcapsession->fd = pcap_get_selectable_fd(pcapsession->pcap_handle);
			pcapsession->supervise_fd = PCAP_SESSION_SUPERVISED_FD;

			// Stream the PCAP file
			pcap_loop(pcapsession->pcap_handle, PCAP_INFINITE, pcapsession_clientconn_packet_handler, (void*)pcapsession);

			// Clear the file descriptor fields for this file
			pcapsession->supervise_fd = PCAP_SESSION_UNSUPERVISED_FD;
			pcapsession->fd = 0;

			// Close packet capture
			pcap_close(pcapsession->pcap_handle);
			pcapsession->pcap_handle = NULL;
		}

		// Close the directory again
		closedir(pcap_directory);
	}
	// Only loop while there are PCAP files in the directory
	while (pcapsession->iterations != 0 && pcap_file_count > 0);

	if (pcap_file_count == 0) {
		// No packet capture files found
		write_to_syslog( "no packet capture files found on session: %d-%s\n", pcapsession->id, pcapsession->description);
	}
	else {
		// Packet capture has been interrupted
		write_to_syslog( "packet capture interrupted on session: %d-%s\n", pcapsession->id, pcapsession->description);
	}

	// Set the state to terminating
	pcapsession_change_state(pcapsession->id, PCAP_SESSION_TERMINATE);

	return NULL;
}

//
// This function stops the file capture session, the state is reset back to PCAP_SESSION_START so that session handling will attempt to
// restart the server.
//
// Parameters:
//  void* pcapsession_param: A transparent parameter on session stop, set to a pcapsession_t* here, points at session
//
void* pcapsession_filecapture_stop(void* pcapsession_param)
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
	if (pcapsession->state == PCAP_SESSION_ABORTING || pcapsession->iterations == 0) {
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
