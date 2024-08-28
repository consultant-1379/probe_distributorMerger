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
 * File: pcapfilestats.c
 * Date: May 29, 2012
 * Author: LMI/LXR/SH Liam Fallon
 ************************************************************************/

#include <stdlib.h>
#include <pcap/pcap.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <gtpv1.h>
#include <pcapdefines.h>
#include <pcapsession.h>

// Forward references for private functions
void pcap_packet_handler(unsigned char* pcap_param, const struct pcap_pkthdr* header, const unsigned char* data);
void print_pcap_stats();

// Hold the file name
char * file_name;

// Variables to hold statistics
unsigned long long packet_count = 0;
unsigned long long byte_count   = 0;
struct timeval first_timestamp;
struct timeval last_timestamp;

int main(int argc, char** argv) {
	// Check arguments
	if (argc != 2) {
		fprintf(stderr, "usage: %s in_file\n", argv[0]);
		fprintf(stderr, "  if in_file is specified as -, then standard input/standard output is used\n");
		return 1;
	}

	file_name = argv[1];

	// Open PCAP file input from standard input
	char pcap_errbuf[PCAP_ERRBUF_SIZE];
	pcap_t* pcap_handle = pcap_open_offline(file_name, pcap_errbuf);
	if (pcap_handle == NULL) {
		fprintf(stderr, "capture start failed on file %s: %s\n", file_name, pcap_errbuf);
		return 2;
	}

	// Handle the PCAP file packets
	pcap_loop(pcap_handle, PCAP_INFINITE, pcap_packet_handler, NULL);

	// Close pcap
	pcap_close(pcap_handle);

	print_pcap_stats();
}

//
// This function is a PCAP packet handler callback method for packet
//
// Parameters:
//  unsigned char* pcap_param: A pointer to user data set in the pcap_loop call, in this case it is not used
//  const struct pcap_pkthdr* header: A pointer to the header of the packet
//  const unsigned char* data: A pointer to the packet data
//
void pcap_packet_handler(unsigned char* pcap_param, const struct pcap_pkthdr* header, const unsigned char* data)
{
	// This is the first packet
	if (packet_count == 0) {
		first_timestamp.tv_sec  = header->ts.tv_sec;
		first_timestamp.tv_usec = header->ts.tv_usec;
	}

	// Increment statistics
	packet_count++;
	byte_count += header->len;

	// Get last time stamp
	last_timestamp.tv_sec  = header->ts.tv_sec;
	last_timestamp.tv_usec = header->ts.tv_usec;

	// Print statistics every 1,000,000 packets
	if (packet_count % 1000000 == 0) {
		print_pcap_stats();
	}
}

//
// This function prints packet capture statistics
//
void print_pcap_stats()
{
	// Format start and end times to strings
	char first_timestamp_string[256];
	char last_timestamp_string[256];

	struct tm* timestamp_tm = gmtime(&first_timestamp.tv_sec);
	strftime(first_timestamp_string, 256, "%s_%Y-%m-%d_%H:%M:%S", timestamp_tm);

	timestamp_tm  = gmtime(&last_timestamp.tv_sec);
	strftime(last_timestamp_string,  256, "%s_%Y-%m-%d_%H:%M:%S", timestamp_tm);

	fprintf(stderr, "%s,pkts:%llu,bytes:%llu,start:%s.%06lu,end:%s.%06lu\n", file_name, packet_count, byte_count, first_timestamp_string, first_timestamp.tv_usec, last_timestamp_string, last_timestamp.tv_usec);
}
