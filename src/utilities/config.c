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
 * File: config.c
 * Date: Feb 23, 2012
 * Author: LMI/LXR/SH John Lee
 ************************************************************************/

/**
 ******************************************************************************
 * @file config.c
 * @ingroup CONFIG
 *      Source file implementation of a minimalist configuration parser using 
 *      json.org json-c library as default.
 *
 *      The json library is conducive to supporting a data agnostic
 *      approach to the contents of the json configuration file(s) provided
 *      some syntactical conventions are observed as detailed in the API.
 ******************************************************************************/

/*******************************************************************************
 * Define Constants and Macros
 *******************************************************************************/

/*******************************************************************************
 * Include public/global header files
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

/*******************************************************************************
 * Include private header files
 *******************************************************************************/
#include <config.h>     /* CONFIG API */
#include <json/json.h>  /* JSON-6 lib API */

/**
 ********************************************************************************
 * @ingroup CONFIG
 * @description
 *    Pointer to JSON object representation of the configuration table
 *    (maintained in json data struct). Not thread-safe.
 *******************************************************************************/
static struct json_object *config_def = NULL;
pthread_mutex_t gate;

/**
 ********************************************************************************
 * @ingroup CONFIG
 * @description
 *    Pointer to JSON object representation of the configuration table
 *    (maintained in json data struct). Not thread-safe.
 *******************************************************************************/
static short filter_config_read;

/**
 *******************************************************************************
 * @ingroup CONFIG
 * @description
 *    Resolve the configuration file name to a path.
 *    Not thread safe.
 *
 * @param config_file_path  IN/OUT Pointer to a string holding the configuration file path
 * @param config_file_name  IN     The name of the file
 *
 * @retval  0           File path was not resolvable
 *         >0           Success. The length of the config file path.
 ******************************************************************************/
int get_config_file_path(char* config_file_path, const char* config_file_name) {
	// Check if the config file name is an absolute path
	if (*config_file_name == '/' || access (config_file_name, F_OK) != -1) {
		// All is OK
		strcpy(config_file_path, config_file_name);
		return strlen(config_file_path);
	}

	// Get the location of relative configuration file definitions
	if (NULL == getenv("PCAP_SESSION_CONFIG_LOCATION")) {
		strcpy(config_file_path, PCAP_SESSION_CONFIG_LOCATION);
	}
	else {
		strcpy(config_file_path, getenv("PCAP_SESSION_CONFIG_LOCATION"));
	}

	// Add the relative part
	strcat(config_file_path, "/");
	strcat(config_file_path, config_file_name);
	return strlen(config_file_path);
}

/**
 * read_filter_config
 */
short read_filter_config(const char *config_filename)
{ 
	int result=0,temp_fd;;

	/* Parameter validation */
	if(config_filename == NULL)
		result=-1;
	else
	{
		if((temp_fd=open(config_filename, O_RDONLY))<0)
			return -1;
		else
			close(temp_fd);

		pthread_mutex_init(&gate,NULL);
		config_def = json_object_from_file((char *)config_filename);
		if(is_error(config_def))
			result=-2;
		else
		{
			++filter_config_read;
			result=json_object_array_length(config_def);
		}
	}
	return result;
}

/**
 * print_filter_config
 */
short print_filter_config(char *buffer)
{
	struct json_object *temp_criteria = NULL;
	const char *json_spec_string = NULL;
	int i = 0, offset = 0, result = -1;

	if ((!filter_config_read) || (NULL!=buffer))
	{
		pthread_mutex_lock(&gate);
		while(i<json_object_array_length(config_def))
		{
			temp_criteria = json_object_array_get_idx(config_def, i);
			json_spec_string = json_object_to_json_string(temp_criteria);
			sprintf(buffer+offset,"criteria:%d=%s\n\n", i, json_spec_string);
			offset += strlen(buffer);
			i++;
		}
		pthread_mutex_unlock(&gate);
		result = 0;
	}
	return result;
}

/**
 * get_property
 */
short get_property(const char *name, char *value)
{
	struct json_object *temp_criteria = NULL, *json_value = NULL;
	int element_count, result=-1;

	if((!filter_config_read) || (NULL==name) || (NULL==value))
		return -1;

	pthread_mutex_lock(&gate);
	for(element_count=0;
			element_count<json_object_array_length(config_def);
			++element_count)
	{
		temp_criteria = json_object_array_get_idx(config_def, element_count);
		json_value = json_object_object_get(temp_criteria, name);
		if(NULL!=json_value)
		{
			strcpy(value,json_object_to_json_string(json_value)+1);
			value[strlen(value)-1] = 0;
			result=0;
			break;
		}
	}
	pthread_mutex_unlock(&gate);
	return result;
}

/**
 * get_properties
 */
short get_properties(   const char *array_name, unsigned int element_size, 
		const char *prop_name, char *values)
{
	struct json_object *temp_outer=NULL, *temp_inner=NULL;
	struct json_object *json_array=NULL, *json_value=NULL;
	int element_count, result=-1, i, offset=0;
	const char *local_value=NULL;

	if((!filter_config_read) || (NULL==array_name) || (NULL==prop_name) ||(NULL==values))
		return -1;

	pthread_mutex_lock(&gate);
	for(element_count=0;
			element_count<json_object_array_length(config_def);
			++element_count)
	{
		temp_outer = json_object_array_get_idx(config_def, element_count);
		json_array = json_object_object_get(temp_outer, array_name);
		if(NULL!=json_array)
		{
			for(i=0;i<json_object_array_length(json_array);++i)
			{
				temp_inner = json_object_array_get_idx(json_array, i);
				json_value = json_object_object_get(temp_inner, prop_name);
				if(NULL!=json_value)
				{
					local_value = json_object_to_json_string(json_value)+1;
					strcpy(values+offset,local_value);
					values[offset+strlen(local_value)-1] = 0;
					offset+=element_size;
					if(result==-1)
						result=1;
					else
						++result;
				}
			}
		}
	}
	pthread_mutex_unlock(&gate);
	return result;
}
