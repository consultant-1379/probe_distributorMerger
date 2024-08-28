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
* File: genutils.h
* Date: Mar 6, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

#ifndef GENUTILS_H_
#define GENUTILS_H_

/**
 * This module contains utility functions
 */

//
// This function strips a character from a string
//
// Parameters:
//  char* string: The string from which to strip the character
//  char strip_char: The character to strip
//
void strip_char_from_string(char* string, const char strip_char);

#endif /* GENUTILS_H_ */
