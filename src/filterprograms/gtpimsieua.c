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
 * File: decodegtp.c
 * Date: Feb 20, 2012
 * Author: LMI/LXR/SH Liam Fallon
 ************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <pcap/pcap.h>

#include <gtpv1.h>
#include <pcapsession.h>
#include <pcapdefines.h>

#define IMSI_MAP_SIZE 10000000
#define IMSI_TIMEOUT  300

int    imsi_map  [IMSI_MAP_SIZE];
time_t imsi_time [IMSI_MAP_SIZE];
char   imsi_array[IMSI_MAP_SIZE][GTPV1_IMSI_LENGTH*2];

//
// This function decodes a GTP packet
//
// Parameters:
//  pcapsession_t*: Returns a pointer to the merger or NULL if opening failed
//  const struct pcap_pkthdr* header: A pointer to the header of the packet
//  const unsigned char* data: A pointer to the packet data
//
void decode_gtp_packet(unsigned char* notused, const struct pcap_pkthdr* header, const unsigned char* data)
{
	struct gtpv1hdr* gtpv1hdr = gtpv1_get_header(header->caplen, data);
	int offset = 0;

	if (gtpv1hdr == NULL) {
		return;
	}

	if (gtpv1hdr->message_type != GTPV1_MT_CREATE_PDP_CONTEXT_REQUEST && gtpv1hdr->message_type != GTPV1_MT_CREATE_PDP_CONTEXT_RESPONSE) {
		return;
	}

	// Options specified
	if (gtpv1hdr->flag_options) {
		offset += sizeof(struct gtpv1hdr) + sizeof(struct gtpv1hdropt);
	}
	else {
		offset += sizeof(struct gtpv1hdr);
	}

	// Hold the IMSI value
	char imsistr[GTPV1_IMSI_LENGTH * 2];
	strcpy(imsistr, "");

	time_t time_now;
	time(&time_now);

	// Get the next information element in the GTP-C message
	while (offset < header->caplen) {
		// Get the Information Element number
		int ie = *(((unsigned char*)gtpv1hdr) + offset);

		// Check if there is enough data to read the header
		if (offset + gtpv1_information_elements[ie].header_length >= header->caplen) {
			// Not enough data remaining to read header, break
			break;
		}

		// Check if this is a TLV IE
		int body_length = gtpv1_information_elements[ie].body_length;
		if (ie >= GTPV1_FIRST_TLV_IE) {
			body_length = ntohs(*(unsigned short*)(((unsigned char*)gtpv1hdr) + offset + 1));
		}

		// Check if there is enough data to read the header
		if (offset + gtpv1_information_elements[ie].header_length + body_length >= header->caplen) {
			// Not enough data remaining to read body, break
			break;
		}

		if (ie == GTPV1_IE_IMSI) {
			struct gtpv1_imsi* imsip = (struct gtpv1_imsi*)(((unsigned char*)gtpv1hdr) + offset);
			imsi2str(imsip, imsistr);
		}

		if (ie == GTPV1_IE_TEI_CONTROL_PLANE) {
			struct gtpv1_teid* teidcpp = (struct gtpv1_teid*)(((unsigned char*)gtpv1hdr) + offset);
			if (gtpv1hdr->message_type == GTPV1_MT_CREATE_PDP_CONTEXT_REQUEST) {

				for (int i = 0; i < IMSI_MAP_SIZE; i++) {
					if (imsi_map[i] == 0 || imsi_time[i] + IMSI_TIMEOUT < time_now) {
						imsi_map[i] = teid2uint(teidcpp);
						imsi_time[i] = time_now;
						strcpy(imsi_array[i], imsistr);
						break;
					}
				}
			}
		}

		if (ie == GTPV1_IE_END_USER_ADDRESS) {
			struct gtpv1_eua* euap = (struct gtpv1_eua*)(((unsigned char*)gtpv1hdr) + offset);

			if (GTPV1_EUA_LENGTH(euap->length) > GTPV1_EUA_NO_ADDRESS_LENGTH) {
				printf("%lu,%u,", header->ts.tv_sec, header->ts.tv_usec);

				for (int i = 0; i < GTPV1_EUA_IPV4_LENGTH; i++) {
					if (i > 0) {
						printf(".");
					}
					printf ("%d", euap->address.ipv4.address[i]);
				}

				if (gtpv1hdr->message_type == GTPV1_MT_CREATE_PDP_CONTEXT_RESPONSE) {
					for (int i = 0; i < IMSI_MAP_SIZE; i++) {
						if (imsi_map[i] == ntohl(gtpv1hdr->teid)) {
							printf(",%s", imsi_array[i]);
							imsi_map[i] = 0;
							break;
						}
					}
				}
				printf("\n");
			}
		}

		offset += gtpv1_information_elements[ie].header_length + body_length;

		// We have processed all Information elements
		if (offset >= sizeof(struct gtpv1hdr) + ntohs(gtpv1hdr->length)) {
			break;
		}
	}
}

int main(int argc, char** argv)
{
	// Set up GTP V1 message types and information elements
	init_gtpv1();

	// Initialize the IMSI array
	for (unsigned long long i = 0; i < IMSI_MAP_SIZE; i++) {
		imsi_map[i] = 0;
		imsi_array[i][0] = '\0';
	}

	if (argc != 3) {
		fprintf(stderr, "usage: %s -i|-f pcap_file_name\n", argv[0]);
		return 1;
	}

	char pcap_errbuf[PCAP_ERRBUF_SIZE];
	pcap_t* pcap_handle = NULL;

	if (!strcmp(argv[1], "-i")) {
		pcap_handle = pcap_open_live(argv[2], PCAP_MAX_SNAPLEN, PCAP_PROMISCUOUS, PCAP_TIMEOUT, pcap_errbuf);
	}
	else if (!strcmp(argv[1], "-f")) {
		pcap_handle = pcap_open_offline(argv[2], pcap_errbuf);
	}
	else {
		fprintf(stderr, "invalid argument %s\n", argv[1]);
		return 1;
	}

	if (pcap_handle == NULL) {
		fprintf(stderr, "decode failed: %s\n", pcap_errbuf);
		return 2;
	}

	pcap_loop(pcap_handle, -1, decode_gtp_packet, NULL);

	return 0;
}

