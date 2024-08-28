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
* File: pcapdefines.h
* Date: Feb 15, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

/**
 * This header file contains common defines for PCAP handling
 */

#ifndef PCAPDEFINES_H_
#define PCAPDEFINES_H_

// Defines for PCAP
#define PCAP_MAX_SNAPLEN 65535   // Maximum packet size to capture
#define PCAP_PROMISCUOUS     1   // PCAP promiscuous mode for interfaces
#define PCAP_TIMEOUT      1500   // Number of milliseconds to wait before timing out on a packet capture wait
#define PCAP_INFINITE        -1  // Loop forever on pcap_loop capturing packets
#define PCAP_FILE_TYPE   ".pcap" // The file type of PCAP files

#endif /* PCAPDEFINES_H_ */
