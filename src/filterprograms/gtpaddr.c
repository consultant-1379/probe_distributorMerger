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
* File: gtpaddr.c
* Date: Feb 27, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

#include <stdlib.h>
#include <pcap/pcap.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <gtpv1.h>
#include <pcapdefines.h>


// Define PCAP handles
pcap_dumper_t* pcap_dumper = NULL;

// Define old and new IP addresses
struct in_addr old_ip;
struct in_addr new_ip;

// Forward references for private functions
void pcap_packet_handler(unsigned char* pcap_param, const struct pcap_pkthdr* header, const unsigned char* data);

int main(int argc, char** argv) {
	// Check arguments
	if (argc != 5) {
		fprintf(stderr, "usage: %s in_file out_file old_ip new_ip\n", argv[0]);
		fprintf(stderr, "  if in_file or out_file are specified as -, then standard input/standard output is used\n");
		return 2;
	}

	// Save old and new IP addresses
	if (!inet_aton(argv[3], &old_ip)) {
		fprintf(stderr, "old Internet address %s is not valid\n", argv[3]);
		return 3;
	}

	if (!inet_aton(argv[4], &new_ip)) {
		fprintf(stderr, "new Internet address %s is not valid\n", argv[4]);
		return 3;
	}

	fprintf(stderr, "starting modification of packet IP addresses\n");

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

	fprintf(stderr, "completed modification of packet IP addresses\n");
}

//
// This function is a PCAP packet handler callback method for packets, it modifies the IP address on the inner GTP-U tunnelled packet
//
// Parameters:
//  unsigned char* pcap_param: A pointer to user data set in the pcap_loop call, in this case it is not used
//  const struct pcap_pkthdr* header: A pointer to the header of the packet
//  const unsigned char* data: A pointer to the packet data
//
void pcap_packet_handler(unsigned char* pcap_param, const struct pcap_pkthdr* header, const unsigned char* data)
{
	// Search for a GTP v1 header in the packet
	struct gtpv1hdr* gtpv1hdr = gtpv1_get_header(header->caplen, data);

	// Check if the header was found or if extension header found
	if (gtpv1hdr == NULL || GTP_EXT_FLAG(gtpv1hdr->flag_options)) {
		// Dump the packet and return
		pcap_dump((unsigned char*)pcap_dumper, header, data);

		return;
	}

	struct ip* inner_ip_header = NULL;
	// Options specified
	if (gtpv1hdr->flag_options) {
		struct gtpv1hdropt* gtpv1hdropt = (struct gtpv1hdropt*)(((char*)gtpv1hdr) + sizeof(struct gtpv1hdr));

		inner_ip_header = (struct ip*)(((char*)gtpv1hdropt) + sizeof(struct gtpv1hdropt));
	}
	else {
		inner_ip_header = (struct ip*)(((char*)gtpv1hdr) + sizeof(struct gtpv1hdr));
	}

	// Change the IP address to the new address
	if (gtpv1hdr->message_type == GTPV1_MT_G_PDU) {
		if (inner_ip_header->ip_src.s_addr == old_ip.s_addr) {
			inner_ip_header->ip_src = new_ip;
		}
		else if (inner_ip_header->ip_dst.s_addr == old_ip.s_addr) {
			inner_ip_header->ip_dst = new_ip;
		}
	}

	pcap_dump((unsigned char*)pcap_dumper, header, data);
}
