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
* File: gtpv1_imsi.h
* Date: Mar 19, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

/**
 * This header file handles the IMSI
 */
#ifndef GTPV1_IMSI_H_
#define GTPV1_IMSI_H_

#include <string.h>

// The length of an IMSI
#define GTPV1_IMSI_LENGTH 8

struct gtpv1_imsi {
	u_char ie_type;
	u_char imsi[GTPV1_IMSI_LENGTH];
};

//
// This method converts an IMSI to a string
//
// Parameters:
//  const struct gtpv1_imsi* imsip: The IMSI to convert
//  char* string: The string into which to write the IMSI
//
static inline void imsi2str(struct gtpv1_imsi* imsip, char* string)
{
	// Intialise the string
	strcpy(string, "");

	// Store the IMSI into the string
	for (int i = 0; i < GTPV1_IMSI_LENGTH; i++) {
		sprintf(string, "%s%d", string, (imsip->imsi[i] & 0x0f));

		// Check for fillers
		if ((imsip->imsi[i] >> 4) < 10) {
			sprintf(string, "%s%d", string, (imsip->imsi[i] >> 4));
		}
	}
}

// This method converts an IMSI to an unsigned long long
//
// Parameters:
//  const struct gtpv1_imsi* imsip: The IMSI to convert
//
// Return:
//  unsigned long long: The IMSI as a long long value
//
static inline unsigned long long imsi2longlong(struct gtpv1_imsi* imsip)
{
	// Initialize the output IMSI
	unsigned long long imsilong = 0;

	// Store the IMSI into the string
	for (int i = 0; i < GTPV1_IMSI_LENGTH; i++) {
		// Shift right the last processed part
		imsilong *= 10;
		imsilong += (imsip->imsi[i] & 0x0f);

		// Check for fillers
		if ((imsip->imsi[i] >> 4) < 10) {
			imsilong *= 10;
			imsilong += imsip->imsi[i] >> 4;
		}
	}

	return imsilong;
}


#endif /* GTPV1_IMSI_H_ */
