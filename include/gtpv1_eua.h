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
* File: gtpv1_eua.h
* Date: Mar 19, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

/**
 * This header file handles the IMSI
 */
#ifndef GTPV1_EUA_H_
#define GTPV1_EUA_H_

#include <string.h>

// PDP Type Organizations
#define GTPV1_EUA_PDP_TYPE_ORG_ETSI 0
#define GTPV1_EUA_PDP_TYPE_ORG_IETF 1

// PDP Type Numbers
#define GTPV1_EUA_PDP_TYPENUMBER_IPV4   0x21
#define GTPV1_EUA_PDP_TYPENUMBER_IPV6   0x57
#define GTPV1_EUA_PDP_TYPENUMBER_IPV4V6 0x8d

// Address Lengths
#define GTPV1_EUA_NO_ADDRESS_LENGTH  2
#define GTPV1_EUA_IPV4_LENGTH        4
#define GTPV1_EUA_IPV6_LENGTH       16

// IPV4 Address
struct eua_ipv4 {
	u_char address[GTPV1_EUA_IPV4_LENGTH];
};

// IPV6 Address
struct eua_ipv6 {
	u_char address[GTPV1_EUA_IPV6_LENGTH];
};

// IPV4V6 Address
struct eua_ipv4v6 {
	struct eua_ipv4 ipv4_address;
	struct eua_ipv6 ipv6_address;
};

// The EUA Information Element
struct gtpv1_eua {
	u_char  ie_type;
	u_char  length[2];
	u_char	spare:4,         // Spare, always 0x1111
			pdp_type_org:4;  // PDP Type Organisations
	u_char  pdp_type_number; // PDP Type Number indicates IPv4, IPv6, or IPv4v6
	union pdp_address {
		struct eua_ipv4   ipv4;
		struct eua_ipv6   ipv6;
		struct eua_ipv4v6 ipv4v6;
	} address;
};

// Get the length as a short
#define GTPV1_EUA_LENGTH(length)	(((length)[0] << 8) + (length)[1])

#endif /* GTPV1_EUA_H_ */
