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

	char time_string[FILENAME_MAX];

	sprintf(time_string, "%s", ctime(&header->ts.tv_sec));
	time_string[strlen(time_string)-1] = 0;

	printf("%s %09u usec:", time_string, (unsigned int)header->ts.tv_sec);

	if (gtpv1hdr == NULL) {
		printf("not-gtp\n");
		return;
	}

	if (gtpv1hdr->message_type == GTPV1_MT_G_PDU) {
		printf("gtp-u,");
	}
	else {
		printf("gtp-c,");
	}

	printf("%d,%d,", GTP_VER(gtpv1hdr->flag_typever), GTP_TYPE(gtpv1hdr->flag_typever));
	printf("%d,%d,%d,", GTP_EXT_FLAG(gtpv1hdr->flag_options), GTP_SEQ_FLAG(gtpv1hdr->flag_options), GTP_NPDU_FLAG(gtpv1hdr->flag_options));
	printf("%d[%s],%d,%x,", gtpv1hdr->message_type, gtpv1_message_types[gtpv1hdr->message_type].name, ntohs(gtpv1hdr->length), ntohl(gtpv1hdr->teid));

	// Options specified
	if (gtpv1hdr->flag_options) {
		struct gtpv1hdropt* gtpv1hdropt = (struct gtpv1hdropt*)(((char*)gtpv1hdr) + sizeof(struct gtpv1hdr));

		offset += sizeof(struct gtpv1hdr) + sizeof(struct gtpv1hdropt);
		printf("%x,%x,%x", ntohs(gtpv1hdropt->sequence_no), gtpv1hdropt->npdu_no, gtpv1hdropt->next_exthdrtype);
	}
	else {
		offset += sizeof(struct gtpv1hdr);
		printf(",,");
	}

	if (gtpv1hdr->message_type == GTPV1_MT_G_PDU) {
		struct ip* tunnelled_ip_header = (struct ip*)(((char*)gtpv1hdr) + offset);
		printf(",%s,", inet_ntoa(tunnelled_ip_header->ip_src));
		printf("%s", inet_ntoa(tunnelled_ip_header->ip_dst));
		printf("\n");
		return;
	}

	// No source or destination on GTP-C
	printf(",,");

	// Checks for IMSI, TEID Data1, TEID COntrol, and EUA
	int imsi_found = 0;
	int teidd1_found = 0;
	int teidcp_found = 0;
	int eua_found = 0;

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
			char imsistr[GTPV1_IMSI_LENGTH * 2];
			imsi2str(imsip, imsistr);
			printf ("%s,", imsistr);
			imsi_found = 1;
		}

		if (ie == GTPV1_IE_TEI_DATA_I) {
			struct gtpv1_teid* teidd1p = (struct gtpv1_teid*)(((unsigned char*)gtpv1hdr) + offset);
			printf("%x,", teid2uint(teidd1p));
			teidd1_found = 1;
		}

		if (ie == GTPV1_IE_TEI_CONTROL_PLANE) {
			struct gtpv1_teid* teidcpp = (struct gtpv1_teid*)(((unsigned char*)gtpv1hdr) + offset);
			printf("%x,", teid2uint(teidcpp));
			teidcp_found = 1;
		}

		if (ie == GTPV1_IE_END_USER_ADDRESS) {
			struct gtpv1_eua* euap = (struct gtpv1_eua*)(((unsigned char*)gtpv1hdr) + offset);

			if (GTPV1_EUA_LENGTH(euap->length) > GTPV1_EUA_NO_ADDRESS_LENGTH) {
				for (int i = 0; i < GTPV1_EUA_IPV4_LENGTH; i++) {
					if (i > 0) {
						printf(".");
					}
					printf ("%d", euap->address.ipv4.address[i]);
				}
				eua_found = 1;
			}
		}

		if (ie > GTPV1_IE_IMSI && !imsi_found) {
			printf(",");
			imsi_found = 1;
		}

		if (ie > GTPV1_IE_TEI_DATA_I && !teidd1_found) {
			printf(",");
			teidd1_found = 1;
		}

		if (ie > GTPV1_IE_TEI_CONTROL_PLANE && !teidcp_found) {
			printf(",");
			teidcp_found = 1;
		}

		offset += gtpv1_information_elements[ie].header_length + body_length;

		// We have processed all Information elements
		if (offset >= sizeof(struct gtpv1hdr) + ntohs(gtpv1hdr->length)) {
			break;
		}
	}

	printf("\n");


}

int main(int argc, char** argv)
{
	// Set up GTP V1 message types and information elements
	init_gtpv1();

	if (argc != 2) {
		fprintf(stderr, "usage: %s pcap_file_name\n", argv[0]);
		return 1;
	}

	char pcap_errbuf[PCAP_ERRBUF_SIZE];
	printf("gtp,version,type,f_exthdr,f_seqno,f_npdu,msgtype,length,teid,seqno,npdu,nexttype,src,dest\n");

	pcap_t* pcap_handle = pcap_open_offline(argv[1], pcap_errbuf);
	if (pcap_handle == NULL) {
		fprintf(stderr, "decode failed: %s\n", pcap_errbuf);
		return 2;
	}
	pcap_loop(pcap_handle, -1, decode_gtp_packet, NULL);

	return 0;
}

