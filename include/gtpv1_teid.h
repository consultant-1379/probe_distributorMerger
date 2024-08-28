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
* File: gtpv1_teid.h
* Date: Mar 19, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

/**
 * This header file handles TEIDs
 */

#ifndef GTPV1_TEID_H_
#define GTPV1_TEID_H_

// The length of an IMSI
#define GTPV1_TEID_LENGTH 4

struct gtpv1_teid {
	u_char ie_type;
	u_char teid[GTPV1_TEID_LENGTH];
};

//
// This method converts a TEID array to an unsigned Integer
//
// Parameters:
//  const struct gtpv1_teid* teidp: The TEID to convert
//
// Return:
//  unsigned int: The converted TEID
//
static inline unsigned int teid2uint(struct gtpv1_teid* teidp)
{
	// Declare the return value
	unsigned int teid_int =
			(teidp->teid[0] << 24) +
			(teidp->teid[1] << 16) +
			(teidp->teid[2] <<  8) +
			(teidp->teid[3]);

	return teid_int;
}

#endif /* GTPV1_TEID_H_ */
