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
 * File: pcapmerger.c
 * Date: Feb 27, 2012
 * Author: LMI/LXR/SH Liam Fallon, John Lee
 ************************************************************************/

/**
 ******************************************************************************
 * @file pcapmerger.c
 * @defgroup PCAP_MERGE pcap_merge
 * @ingroup PCAP_MERGE
 *
 * This program merges PCAP streams.
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
#define PROPERTY_PER_SERVER 2
#define MAX_VALUES MAX_ADDRESSES*PROPERTY_PER_SERVER

#define USAGE(logger) \
		fprintf(logger, "usage %s: config_file_name\n", argv[0]); \
		return 1;

/**
 ********************************************************************************
 * @ingroup PCAP_MERGE
 * @description
 *   Shutdown handler.
 *
 * @param sig    IN  Signal caught with this signal handler.
 *******************************************************************************/
static void sighandler(int sig);

/**
 ********************************************************************************
 * @ingroup PCAP_MERGE
 * @description
 *   Register signals with shutdown handler.
 *
 * @param sig    IN  Signal caught with this signal handler.
 *******************************************************************************/
static void register_signals_with_shutdown(void(*p_pHandler)(int));

/**
 ********************************************************************************
 * @ingroup PCAP_MERGE
 * @description
 *   Parse in program configuration from file.
 *
 * @retval -1           Unable to open profile configuration file
 * @retval -2           Data in event filter criteria in unexpected format
 * @retval >0           Success. Value returned is number of top-level array 
 *                      elements.
 *******************************************************************************/
static int read_pcap_merge_cfg_file();

/**
 ********************************************************************************
 * @ingroup PCAP_MERGE
 * @description
 *   The configuration file.
 *******************************************************************************/
static char pcap_merge_cfg_file[FILENAME_MAX];

// Flag used to avoid multiple shutdown calls
static int shutdown_ordered = 0;

/**
 * main
 */
int main(int argc, char *argv[])
{
	pthread_t supervision_thread;
	char untunnel_str[FILENAME_MAX], output_path_str[FILENAME_MAX];
	char config_str[MAX_MESSAGE_BODY_SIZE];
	char host_values[MAX_ADDRESSES][FILENAME_MAX];
	char port_values[MAX_ADDRESSES][FILENAME_MAX];
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
	init_logger("PCAP_merger:");

	// Register signals with signal handler
	register_signals_with_shutdown(sighandler);

	// Get the configuration file name
	int cfg_file_length = get_config_file_path(pcap_merge_cfg_file, argv[2]);
	write_to_syslog("configuration file name is : %s\n", pcap_merge_cfg_file);
	if (cfg_file_length <= 0 || read_pcap_merge_cfg_file() <= 0) {
		write_to_syslog("unable to process configuration, file name is : %s\n", pcap_merge_cfg_file);
		exit(1);
	}

	// Print the properties that were read
	print_filter_config(config_str);
	write_to_syslog("config file contents: \n%s", config_str);

	// Get properties
	get_property(UNTUNNEL_GTP_PROPERTY, untunnel_str);
	get_property(OUTPUT_PATH_PROPERTY, output_path_str);

	int untunnel = atoi(untunnel_str);
	strip_char_from_string(output_path_str, '\\');

	// Get the host and port properties
	size_t host_length = get_properties(DISTRIBUTION_SERVER_ARRAY_PROPERTY, FILENAME_MAX, HOST_PROPERTY, (char *)host_values);
	size_t port_length = get_properties(DISTRIBUTION_SERVER_ARRAY_PROPERTY, FILENAME_MAX, PORT_PROPERTY, (char *)port_values);

	// Sanity check that host and port counts are the same
	if (host_length != port_length) {
		write_to_syslog( "unmatched number of distribution server hosts %d and ports %d\n", host_length, port_length);
		exit(1);
	}

	// Print host values
	for (int i = 0; i < host_length; i++)
	{
		write_to_syslog("distribution server %d: %s %s\n", i, host_values[i], port_values[i]);
	}

	// Set the list of server addresses
	int result = address_list_from_arguments(host_length, host_values, port_values, servers);
	if ((result <= 0) || (servers == NULL)) {
		write_to_syslog( "invalid addresses specified with result: %d\n", result);
		exit(1);
	}

	// Initialize PCAP session handling
	write_to_syslog( "starting session handling\n");
	if (!pcapsession_handling_init(&supervision_thread)) {
		write_to_syslog( "failed to start session handling\n");
		exit(1);
	}
	write_to_syslog( "started session handling\n");

	// Kick off packet merging and dumping to standard output
	write_to_syslog( "starting packet merging and dumping\n");
	pcapsession_t* pcap_merger = pcapsession_merger_open(output_path_str, untunnel);
	if (pcap_merger == NULL) {
		write_to_syslog( "failed to start packet merging and dumping\n");
		exit(1);
	}

	// Wait for 10 seconds to allow any clients to connect
	write_to_syslog("waiting for 10s to allow merge dumping to start before connecting clients\n");
	sleep(10);

	// Iterate over each address and start a server connection for each one
	for (int i = 0; i < result; i++) {
		// Open a server connection for this address
		pcapsession_client_open(servers[i].address, pcap_merger);
	}

	// Check if session handling is completed
	if (pthread_join(supervision_thread, (void **) NULL) != 0)
	{
		perror("Thread join failed");
		exit(1);
	}

	write_to_syslog("PCAP merging finished\n");
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
	//signal(SIGSEGV, p_pHandler);
#ifdef __linux__
	signal(SIGSTKFLT, p_pHandler);
#endif
}

/**
 * read_pcap_merge_cfg_file
 */
static int read_pcap_merge_cfg_file()
{
	short result = 0;

	/* Calling the event profile filter api */
	result = read_filter_config((const char *)pcap_merge_cfg_file);

	/* Handling return result here so clear to operator which process has
       problems reading in it's event filters. Failures are printed to terminal 
       and to log file */
	if(result > 0)
	{
		write_to_syslog("merger successfully read in its configuration\n");
	}
	else if (result == -1)
	{
		write_to_syslog("merger failed to read in its configuration\n");
		write_to_syslog("unable to open configuration file\n");
	}
	else
	{
		write_to_syslog("merger failed to read in its configuration\n");
		write_to_syslog("data in configuration file is in unexpected format\n");
	}
	return result;
}
