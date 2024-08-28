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
* File: chunkpcapfile.c
* Date: Mar 7, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

#include <stdlib.h>
#include <pcap/pcap.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <pcapdefines.h>
#include <pcapsession.h>

// Constants
#define MIN_CHUNK_SIZE 0x100000

// Define PCAP handles
pcap_t* pcap_handle = NULL;
pcap_dumper_t* pcap_dumper = NULL;

// Variables for chunk handling
unsigned long chunk_max_size = 0;  // Maximum chunk size in a PCAP file
unsigned long chunk_no       = 0;  // Current chunk number
unsigned long chunk_used     = 0;  // Amount of data output on current chunk

// Variables for file handling
char* out_file_name = NULL;

// Forward references for private functions
void pcap_packet_handler(unsigned char* pcap_param, const struct pcap_pkthdr* header, const unsigned char* data);

int main(int argc, char** argv) {
	// Check arguments
	if (argc != 4) {
		fprintf(stderr, "usage: %s in_file out_file_name chunk_max_size\n", argv[0]);
		fprintf(stderr, "  if in_file is specified as -, then standard input is used\n");
		fprintf(stderr, "  chunk_max_size is the maximum size of an output PCAP file\n");
		return 2;
	}

	// Save arguments
	out_file_name = argv[2];
	chunk_max_size = atol(argv[3]);

	// check chunk size
	if (chunk_max_size < MIN_CHUNK_SIZE) {
		fprintf(stderr, "  chunk_max_size must be at least %d\n", MIN_CHUNK_SIZE);
		return 3;

	}

	fprintf(stderr, "starting chunking\n");

	// Open PCAP file input from standard input
	char pcap_errbuf[PCAP_ERRBUF_SIZE];
	pcap_handle = pcap_open_offline(argv[1], pcap_errbuf);
	if (pcap_handle == NULL) {
		fprintf(stderr, "capture start failed on file %s: %s\n", argv[1], pcap_errbuf);
		return 4;
	}

	// Handle the PCAP file packets
	pcap_loop(pcap_handle, PCAP_INFINITE, pcap_packet_handler, NULL);

	// Close dumper if open
	if (pcap_dumper != NULL) {
		pcap_dump_close(pcap_dumper);
	}

	// Close pcap
	pcap_close(pcap_handle);

	fprintf(stderr, "completed chunking\n");
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
	// Check if we need to close the current dumper
	if ((chunk_used + sizeof(struct pcap_pkthdr) + header->caplen) > chunk_max_size) {
		// Close dumping
		pcap_dump_close(pcap_dumper);
		pcap_dumper = NULL;
		chunk_used = 0;
	}

	// Check if we need to open the dumper
	if (pcap_dumper == NULL) {
		// Generate the chunk file name
		char chunk_dump_file[FILENAME_MAX];
		sprintf(chunk_dump_file, "%s_%06lu.pcap", out_file_name, chunk_no++);

		// Open the dump
		pcap_dumper = pcap_dump_open(pcap_handle, chunk_dump_file);
		if (pcap_dumper == NULL) {
			pcap_close(pcap_handle);
			fprintf(stderr, "dump start failed on file %s\n", out_file_name);
			exit(1);
		}
		fprintf(stderr, "dumping to %s . . .\n", chunk_dump_file);
	}

	// Dump the packet
	pcap_dump((unsigned char*)pcap_dumper, header, data);

	// Increment chunks used
	chunk_used += sizeof(struct pcap_pkthdr) + header->caplen;
}


