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
 * File: pcapdistributed.c
 * Date: Feb 27, 2012
 * Author: LMI/LXR/SH Liam Fallon, John Lee
 ************************************************************************/

/**
 ******************************************************************************
 * @file pcapdistributer.c
 * @defgroup PCAP_DIST pcap_distribution
 * @ingroup PCAP_DIST
 *
 * This module locks off and terminates all handling for PCAP stream 
 * distribution
 ******************************************************************************/

/*******************************************************************************
 * Include public/global header files
 *******************************************************************************/
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

/*******************************************************************************
 * Include private header files
 *******************************************************************************/
extern "C" {
#include <config.h>
#include <genutils.h>
#include <logger.h>
#include <pcapsession.h>
#include <tcp.h>
}
#include "MagicStringTester.h"


/*******************************************************************************
 * Define Constants and Macros
 *******************************************************************************/
#define USAGE(logger) \
		fprintf(logger, "usage %s: config_file_name\n", argv[0]); \
		return 1;

/**
 ********************************************************************************
 * @ingroup PCAP_DIST
 * @description
 *   Shutdown handler.
 *
 * @param sig    IN  Signal caught with this signal handler.
 *******************************************************************************/
static void sighandler(int sig);

/**
 ********************************************************************************
 * @ingroup PCAP_DIST
 * @description
 *   Register signals with shutdown handler.
 *
 * @param sig    IN  Signal caught with this signal handler.
 *******************************************************************************/
static void register_signals_with_shutdown(void(*p_pHandler)(int));

/**
 ********************************************************************************
 * @ingroup PCAP_DIST
 * @description
 *   Parse in program configuration from file.
 *
 * @retval -1           Unable to open profile configuration file
 * @retval -2           Data in event filter criteria in unexpected format
 * @retval >0           Success. Value returned is number of top-level array 
 *                      elements.
 *******************************************************************************/
static int read_pcap_dist_cfg_file();

/**
 ********************************************************************************
 * @ingroup PCAP_DIST
 * @description
 *   The configuration file.
 *******************************************************************************/
static char pcap_dist_cfg_file[FILENAME_MAX];

// Flag used to avoid multiple shutdown calls
static int shutdown_ordered = 0;

/**
 * main
 */
int main(int argc, char *argv[])
{
	pthread_t supervision_thread;
	char* capture_location;
	char live_str[FILENAME_MAX], capture_location_str[FILENAME_MAX], port_str[FILENAME_MAX], iterations_str[FILENAME_MAX];
	char config_str[MAX_MESSAGE_BODY_SIZE];
	MagicStringTester licenceTester;

	if(argc < 2 || licenceTester.testString(argv[1]))
	{
		fprintf(stderr, "Licence check failed\n");
		return(254);
	}
	
	// The configuration file name must be specified
	if (argc != 3) {
		USAGE(stderr);
	}

	// Open logging
	init_logger("PCAP_distributer:");

	// Register signals with signal handler
	register_signals_with_shutdown(sighandler);

	// Get the configuration file name
	int cfg_file_length = get_config_file_path(pcap_dist_cfg_file, argv[2]);
	write_to_syslog("configuration file name is : %s\n", pcap_dist_cfg_file);
	if (cfg_file_length <= 0 || read_pcap_dist_cfg_file() <= 0) {
		write_to_syslog("unable to process configuration, file name is : %s\n", pcap_dist_cfg_file);
		exit(1);
	}
	print_filter_config(config_str);
	write_to_syslog("config file contents: \n%s", config_str);

	get_property(CAPTURE_LOCATION_PROPERTY, capture_location_str);
	get_property(PORT_PROPERTY, port_str);
	get_property(LIVE_CAPTURE_PROPERTY, live_str);
	get_property(FILE_CAPTURE_ITERATIONS_PROPERTY, iterations_str);

	capture_location = capture_location_str;
	strip_char_from_string(capture_location, '\\');
	int live = atoi(live_str);
	int distribution_port = atoi(port_str);
	int iterations = atoi(iterations_str);

	// Check the port number is valid
	if (distribution_port < 1 || distribution_port > HIGHEST_IP_PORT) {
		write_to_syslog("port %s invalid, port must be a whole number between 1 and %d\n", argv[2], HIGHEST_IP_PORT);
		exit(1);
	}

	// Initialize PCAP session handling
	write_to_syslog("starting session handling\n");
	if (!pcapsession_handling_init(&supervision_thread)) {
		write_to_syslog( "failed to start session handling\n");
		exit(1);
	}
	write_to_syslog( "started session handling\n");

	// Kick off the server for client connections
	write_to_syslog( "starting client listening\n");
	if (!pcapsession_server_open(distribution_port)) {
		write_to_syslog( "failed to start client listening\n");
		exit(1);
	}
	write_to_syslog( "client listening started\n");

	// Wait for 10 seconds to allow any clients to connect
	write_to_syslog("waiting for 10s before starting packet capture to allow client connect\n");
	sleep(10);

	// Check if we are in live or directory mode
	if (live) {
		// Kick off live packet capture
		write_to_syslog( "starting live packet capture\n");
		if (!pcapsession_livecapture_open(capture_location)) {
			write_to_syslog( "failed to start live packet capture\n");
			exit(1);
		}
	}
	else {
		// Kick off directory packet capture
		write_to_syslog( "starting directory packet capture\n");
		if (!pcapsession_filecapture_open(capture_location, iterations)) {
			write_to_syslog( "failed to start directory packet capture\n");
			exit(1);
		}
	}

	// Check if session handling is completed
	if(pthread_join(supervision_thread, (void **) NULL) != 0)
	{
		perror("Thread join failed");
		exit(1);
	}

	write_to_syslog( "PCAP distribution finished\n");
	sighandler(0);
	exit(0);
}

/**
 * sighandler
 */
static void sighandler(int sig)
{
	// Check and set shutdown flag
	if (shutdown_ordered) {
		// Shutdown already in progress
		return;
	}
	else {
		shutdown_ordered = 1;
	}

	write_to_syslog("server termination ordered: signal=%d\n", sig);
	pcapsession_handling_close();
	closelog();
}

/**
 * register_signals_with_shutdown
 */
static void register_signals_with_shutdown(void(*p_pHandler)(int))
{
	// Set up signals to block. This applies to all running threads
	// created by this process.
	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGALRM);
	sigaddset(&sigs, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &sigs, NULL);

	// Set up signal handlers that deal with program interruption and call the
	// sighandler() function when the handled signals are received. sighandler()
	// attempts a graceful shutdown
	signal(SIGABRT, p_pHandler);
	signal(SIGTERM, p_pHandler);
	signal(SIGINT, p_pHandler);
	signal(SIGQUIT, p_pHandler);
	signal(SIGSEGV, p_pHandler);
#ifdef __linux__
	signal(SIGSTKFLT, p_pHandler);
#endif
}

/**
 * read_pcap_dist_cfg_file
 */
static int read_pcap_dist_cfg_file()
{
	short result = 0;

	/* Calling the event profile filter api */
	result = read_filter_config((const char *)pcap_dist_cfg_file);

	/* Handling return result here so clear to operator which process has
       problems reading in it's event filters. Failures are printed to terminal 
       and to log file */
	if (result > 0)
	{
		write_to_syslog("distributer successfully read in its configuration\n");
	}
	else if (result == -1)
	{
		write_to_syslog("distributer failed to read in its configuration\n");
		write_to_syslog("unable to open configuration file\n");
	}
	else
	{
		write_to_syslog("distributer failed to read in its configuration\n");
		write_to_syslog("data in configuration file is in unexpected format\n");
	}
	return result;
}
