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
* File: config.h
* Date: Feb 23, 2012
* Author: LMI/LXR/SH John Lee
************************************************************************/

/**
 *******************************************************************************
 * @file config.h
 * @defgroup CONFIG config
 *
 * @lld_start
 * @lld_overview
 * 
 * Application Programming Interface for the Config component.
 *
 * <p>The configuration file must obey the following json (json.org) syntax:
 * 1)   The file contains a top level array of elements.
 * 2)   An element can contain top level name/value pairs, where a value can
 *      also be an array of name/value pairs.
 * 3)   All names must be strings.
 * 4)   All values must be strings or arrays.
 *
 * Current Additional constraints:
 * 1) Configuration file cannot be greater than 4kbytes.
 * 2) Only supports nested arrays to a depth of 2.
 *
 * @dependencies
 *     json.org JSON parser library
 * @lld_end
 ******************************************************************************/
#ifndef FILTER_CONFIG_H
#define FILTER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
    
// Location of PCAP filter config files
#define PCAP_SESSION_CONFIG_LOCATION "/opt/ericsson/eniq-analysis/eniq-analysis/etc/filter"

// Property strings
#define CAPTURE_LOCATION_PROPERTY          "capture_location"
#define HOST_PROPERTY                      "host"
#define PORT_PROPERTY                      "port"
#define LIVE_CAPTURE_PROPERTY              "live"
#define FILE_CAPTURE_ITERATIONS_PROPERTY   "file_capture_iterations"
#define UNTUNNEL_GTP_PROPERTY              "untunnel_gtp"
#define OUTPUT_PATH_PROPERTY               "output_path"
#define DISTRIBUTION_SERVER_ARRAY_PROPERTY "distribution_server_array"

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
int get_config_file_path(char* config_file_path, const char* config_file_name);

/**
 *******************************************************************************
 * @ingroup CONFIG
 * @description
 *    Read in properties from configuration file and store. 
 *    Not thread safe.
 *
 * @param config_filename  IN    Pointer to the string identifying absolute 
 *                               configuration file path for using application 
 *                               process 
 *
 * @retval -1           Unable to open profile configuration file
 *         -2           Data in event filter criteria in unexpected format
 *         >0           Success. Value returned is number of top-level array 
 *                      elements.
 ******************************************************************************/
short read_filter_config(const char *config_filename);
    
/**
 *******************************************************************************
 * @ingroup CONFIG
 * @description 
 *    Print config to client provided buffer. It is the responsibility of the
 *    client to allocate sufficient memory to contain the parsed json structure.
 *    If not enough space is allocated the client shall core dump
 *    Thread safe.
 *
 * @param buffer  OUT      Target for parsed configuration
 *
 * @retval -1           Failure.
 *         =0           Success.
 ******************************************************************************/
short print_filter_config(char *buffer);
    
/**
 *******************************************************************************
 * @ingroup CONFIG
 * @description
 *    Get leaf non-array property value from internal data structure. The client 
 *    determines if the requested property is mandatory or optional and whether 
 *    an unfound property is serious or not.
 *    Thread safe.
 *
 *    For example:
        [{"some":"rubbish"},
         {"read_mode":"0","capture_location":"eth0","port":"3000",
         "server_arr":[{"host":"192.168.122.222"},{"host":"192.168.122.223"}], 
         "server_elem_size":"20"}]
 *
 *    You would use this function to read the value of 'capture_location'.
 *
 * @param name  IN      Name of the requested property
 * @param value OUT     Value of requested property. Shall be set to null if
 *                      requested property not found. It is the responsibility
 *                      of the client to ensure adequate space is allocated for
 *                      value. If not enough space is allocated the client shall
 *                      core dump.
 *
 * @retval <0         Failure or not present. 
 * @retval =0         Success. 
 ******************************************************************************/
short get_property(const char *name, char *value);

/**
 *******************************************************************************
 * @ingroup CONFIG
 * @description
 *    Get list of array value property value from internal data structure. The 
 *    client determines if the requested property is mandatory or optional and 
 *    whether an unfound property is serious or not.
 *    Thread safe.
 *
 *    For example:
        [{"some":"rubbish"},
         {"read_mode":"0","capture_location":"eth0","port":"3000",
         "server_arr":[{"host":"192.168.122.222"},{"host":"192.168.122.223"}], 
         "server_elem_size":"20"}]
 *
 *    You would use this function to read the value of all the server hosts.
 *
 * @param array_name    IN      Name of the array_name
 * @param prop_name     IN      Name of the requested property within array
 * @param values        OUT     Pointer to an array of character string values 
 *                              associated with the name. It is the 
 *                              responsibility of the client to ensure adequate 
 *                              space is allocated for value. If not enough 
 *                              space is allocated the client shall core dump.
 *
 * @retval -1         Failure or not present. 
 * @retval >0         Returns the length of the array pointed to be 'values'. 
 ******************************************************************************/
short get_properties(   const char *array_name, unsigned int element_size, 
                        const char *prop_name, char *values);

#ifdef __cplusplus
}
#endif 
#endif /* FILTER_CONFIG_H */
