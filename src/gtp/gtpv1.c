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
* File: gtpuv1.c
* Date: Feb 20, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

/**
 * This module handles the GTP-U V1 protocol
 */

#include <stdio.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#define __USE_BSD
#define __FAVOR_BSD
#include <netinet/udp.h>

#include <gtpv1.h>

//
// This function initializes static data for GTP V1 handling
//
void init_gtpv1()
{
	// Initialize message types and information elements
	init_gtpv1_message_types();
	init_gtpv1_information_elements();
}

//
// This function returns a pointer to a GTPv1 header in a packet in a data buffer
//
// Parameters:
//  const unsigned int length: The amount of data present in the data buffer
//  const unsigned char* data: A pointer to the packet data
//
// Return:
//  gtpv1hdr*: A pointer to a GTP V1 header if a GTP V1 packet is present, NULL otherwise
//
struct gtpv1hdr* gtpv1_get_header(const unsigned int length, const unsigned char* data)
{
	// Keep track of the current position to avoid complex casts
	unsigned int offset = 0;

	// Check if there is enough data for the Ethernet header
	if (length - offset < sizeof(struct ether_header)) {
		return NULL;
	}

	// Set the Ethernet header pointer
	struct ether_header* ether_header = (struct ether_header*)(data + offset);

	// Check if there is a VLAN specified in the Ethernet header
	int ether_type = ntohs(ether_header->ether_type);
	if (ether_type == ETHERTYPE_VLAN) {
		// In this case, we must skip past the 802.1Q tag and find the Ethernet type in the next 2 octets
		u_short* ether_typep = (u_short*)(((char*)&ether_header->ether_type) + TAG_ETHER_802_1_Q_LENGTH);
		ether_type = ntohs(*ether_typep);

		// Set the IP header pointer to the end of the Ethernet header, this is the outer IP header
		offset +=  sizeof(struct ether_header) + TAG_ETHER_802_1_Q_LENGTH;
	}
	else {
		// Set the IP header pointer to the end of the Ethernet header, this is the outer IP header
		offset +=  sizeof(struct ether_header);
	}

	// Check if this is an IPv4 packet, if not, return because for now we only support IPV4
	// TODO: Implement IPv6
	if (ether_type != ETHERTYPE_IP) {
		return NULL;
	}


	// Check if there is enough data for the IP header
	if (length - offset < sizeof(struct ip)) {
		return NULL;
	}

	// Set the IP header pointer
	struct ip* ip_header = (struct ip*)(data + offset);

	// Check if the IP header version is IPv4, for now we only support IPv4
	// TODO: Implement IPv6
	if (ip_header->ip_v != IPVERSION) {
		return NULL;
	}

	// Check if the enclosing protocol is UDP, GTP is carried in UDP
	if (ip_header->ip_p != IPPROTO_UDP) {
		return NULL;
	}

	// Set the UDP header pointer to the end of the IP header, The IP header length is in units of 4 octets
	offset +=  ip_header->ip_hl * 4;

	// Check if there is enough data for the UDP header
	if (length - offset < sizeof(struct udphdr)) {
		return NULL;
	}

	// Set the UDP header pointer
	struct udphdr* udp_header = (struct udphdr*)(data + offset);

	// Check for GTP-U or GTP-C
	if (ntohs(udp_header->uh_sport) != GTP_U_UDP_PORT && ntohs(udp_header->uh_dport) != GTP_U_UDP_PORT &&
		ntohs(udp_header->uh_sport) != GTP_C_UDP_PORT && ntohs(udp_header->uh_dport) != GTP_C_UDP_PORT) {
		return NULL;
	}

	// Set the GTP V1 header pointer to the end of the UDP header
	offset +=  sizeof(struct udphdr);

	// Check if there is enough data for the GTP v1 header
	if (length - offset < sizeof(struct gtpv1hdr)) {
		return NULL;
	}

	// Set the UDP header pointer
	struct gtpv1hdr* gtpv1_header = (struct gtpv1hdr*)(data + offset);

	// Check for GTP Version 1
	if (gtpv1_header->flag_typever != GTP_V1_TYPEVER) {
		return NULL;
	}

	// OK, we have now got to the GTP v1 header
	return gtpv1_header;
}



