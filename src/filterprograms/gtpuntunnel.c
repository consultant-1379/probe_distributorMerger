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
* File: gtptunnel.c
* Date: Feb 27, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

#include <stdlib.h>
#include <pcap/pcap.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <gtpv1.h>
#include <pcapdefines.h>
#include <pcapsession.h>

// Define PCAP handles
pcap_dumper_t* pcap_dumper = NULL;

// Define old and new IP addresses
struct in_addr old_ip;
struct in_addr new_ip;

// Forward references for private functions
void pcap_packet_handler(unsigned char* pcap_param, const struct pcap_pkthdr* header, const unsigned char* data);

int main(int argc, char** argv) {
	// Check arguments
	if (argc != 3) {
		fprintf(stderr, "usage: %s in_file out_file\n", argv[0]);
		fprintf(stderr, "  if in_file or out_file are specified as -, then standard input/standard output is used\n");
		return 2;
	}

	fprintf(stderr, "starting GTP untunnelling\n");

	// Open PCAP file input from standard input
	char pcap_errbuf[PCAP_ERRBUF_SIZE];
	pcap_t* pcap_handle = pcap_open_offline(argv[1], pcap_errbuf);
	if (pcap_handle == NULL) {
		fprintf(stderr, "capture start failed on file %s: %s\n", argv[1], pcap_errbuf);
		return 4;
	}

	// Open packet dumping on the standard output
	pcap_dumper = pcap_dump_open(pcap_handle, argv[2]);
	if (pcap_dumper == NULL) {
		pcap_close(pcap_handle);
		fprintf(stderr, "dump start failed on file %s\n", argv[2]);
		return 5;
	}

	// Handle the PCAP file packets
	pcap_loop(pcap_handle, PCAP_INFINITE, pcap_packet_handler, NULL);

	// Close dumper if open
	if (pcap_dumper != NULL) {
		pcap_dump_close(pcap_dumper);
	}

	// Close pcap
	pcap_close(pcap_handle);

	fprintf(stderr, "completed GTP untunnelling\n");
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
	// Copy the header data to a new modifiable header
	struct pcap_pkthdr new_header;
	new_header.caplen = header->caplen;
	new_header.len    = header->len;
	new_header.ts     = header->ts;

	// Untunnel the packet
	pcapsession_untunnel_packet(NULL, &new_header, data);

	// Dump the packet
	pcap_dump((unsigned char*)pcap_dumper, &new_header, data);
}
