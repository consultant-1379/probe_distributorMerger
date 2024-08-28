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
* File: util.c
* Date: Mar 6, 2012
* Author: LMI/LXR/SH Liam Fallon
************************************************************************/

#include <string.h>

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
void strip_char_from_string(char* string, const char strip_char)
{
	// Iterate over the string
	int j = 0;
	for (int i = 0; i < strlen(string); i++) {
		// Find if we have found the strip character
		if (string[i] != strip_char) {
			string[j++] = string[i];
		}
	}

	// Terminate the string
	string[j] = 0;
}
