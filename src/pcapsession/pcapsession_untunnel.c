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
* File: pcapsession_untunnel.c
* Date: Feb 21, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

/**
 * This module handles untunnelling of a GTP packet
 */

#include <string.h>
#include <stdlib.h>
#include <net/ethernet.h>

#include <gtpv1.h>
#include <pcapsession.h>

//
// This function untunnels GTP-U packets by moving the enclosed IP header up to just under the Ethernet header
//
// Parameters:
//  pcapsession_t*: Returns a pointer to the merger or NULL if opening failed
//  const unsigned char* data: A pointer to the packet data
//
void pcapsession_untunnel_packet(pcapsession_t* pcapsession, struct pcap_pkthdr* header, const unsigned char* data)
{
	// Search for a GTP v1 header in the packet
	struct gtpv1hdr* gtpv1hdr = gtpv1_get_header(header->caplen, data);

	// Check if the header was found
	if (gtpv1hdr == NULL) {
		return;
	}

	// Increment the GTP counters in the monitor, only use monitor if we're using a PCAP session
	if (pcapsession != NULL) {
		monitor_increment_gtp(pcapsession->monitor, 1, header->len, gtpv1hdr->flag_options);
	}

	// Check if this packet has extension headers, if so do not untunnel it because there are very few
	// of these packets and implementation is complex
	// TODO: Implement extension header handling
	if (GTP_EXT_FLAG(gtpv1hdr->flag_options)) {
		return;
	}

	// Find the end of the GTP header
	char* source_ptr = ((char*)gtpv1hdr) + sizeof(struct gtpv1hdr);

	// Find if there are optional fields
	if (gtpv1hdr->flag_options) {
		// There are optional fields
		source_ptr += sizeof(struct gtpv1hdropt);
	}

	// Find the target pointer, it's just after the Ethernet header
	char* target_ptr = (char*)data + sizeof(struct ether_header);

	// Check if the Ethernet header includes an optional 802.1Q tag
	struct ether_header* ether_header = (struct ether_header*)(data);
	if (ntohs(ether_header->ether_type) == ETHERTYPE_VLAN) {
		// In this case, we must add the length of the 802.1Q tag to the Ethernet header length
		target_ptr += TAG_ETHER_802_1_Q_LENGTH;
	}

	// Find out how much data is being dropped
	int dropped = source_ptr - target_ptr;

	// Now copy up the packet to just under the Ethernet header
	memmove(target_ptr, source_ptr, header->caplen - dropped);

	// Adjust the packet header fields
	header->caplen -= dropped;
	header->len -= dropped;
}

