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
* File: tcp.h
* Date: Feb 27, 2012
* Author: LMI/LXR/SH John Lee
************************************************************************/

/**
 *******************************************************************************
 * @file tcp.h
 * @defgroup COMMS comms
 *
 * @lld_start
 * @lld_overview
 * 
 * This API respresents a facade on the native socket interface.
 *
 * @lld_end
 ******************************************************************************/
#ifndef FILTER_COMMS_H
#define FILTER_COMMS_H

#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif 
    
/*******************************************************************************
* Define Constants and Macros
*******************************************************************************/
#define HIGHEST_IP_PORT 65535
#define MAX_ADDRESSES 64
#define TCP_ERRBUF_SIZE 200
#define IOTESTS_CLOSED_POLL_FLAGS POLLERR | POLLHUP

/**
 *******************************************************************************
 * @ingroup COMMS
 * @description
 *    Callback to the server to indicate that connection to client is 
 *    established.
 *
 * @param handle            IN      Handle to the server side socket
 * @param client_address    IN      Address of connecting client.
 *
 * @retval <=0       Failure. 
 * @retval >0        Success. 
 ******************************************************************************/
typedef int (COM_CALL_BACK)(int handle, struct sockaddr_in client_address);
   
/**
 *******************************************************************************
 * @ingroup COMMS
 * @description
 *    Target destination for client to connect to.
 ******************************************************************************/
struct target
{
    struct sockaddr_in address;
	int handle;
};
typedef struct target COM_IP_ADDRESS;
struct target servers[MAX_ADDRESSES];

/**********---------------- SERVER - SIDE ------------------*******************/   
    
/**
 *******************************************************************************
 * @ingroup COMMS
 * @description
 *    Open named server side socket.
 *
 * @param port              IN      Socket listens on this port
 * @param error_string      OUT     Buffer containing and error strings 
 *                                  returned from this function. It is the
 *                                  responsibility of the client to allocate
 *                                  sufficient memory to contain error string.
 * @param error_string_len  IN      Length of error_string.
 *
 * @retval -1         Failure - Invalid parameters passed. 
 * @retval -2         Failure - Unable to open socket. 
 * @retval -3         Failure - Unable to set socket options. 
 * @retval -4         Failure - Unable to bind. 
 * @retval -5         Failure - Unable to create listening queue. 
 * @retval -6         Failure - Unable to set listening socket non-blocking. 
 * @retval >=0        Success - Handle to the named server side socket. 
 ******************************************************************************/
int open_server_socket( int port, struct sockaddr_in *our_addr, 
                        char *error_string, size_t error_string_len);
   
/**
 *******************************************************************************
 * @ingroup COMMS
 * @description
 *    Wait for client to attempt connection establishment.
 *    WARNING:  This function shall block until the client has made a connection
 *              request to the server.
 *
 * @param handle            IN      Handle to the server side socket
 * @param fn                IN      Callback to client indicating connection
 *                                  is established.                               
 * @param error_string      OUT     Buffer containing and error strings 
 *                                  returned from this function. It is the
 *                                  responsibility of the client to allocate
 *                                  sufficient memory to contain error string.
 * @param error_string_len  IN      Length of error_string.
 *
 * @retval -1         Failure - Invalid parameters passed. 
 * @retval -2         Failure - Unable to open socket. 
 * @retval >=0        Success - Handle the return from callback. The callback
 *                          should not return negative integers as these are
 *                          reserved. 
 ******************************************************************************/   
int poll_server_socket( int handle, COM_CALL_BACK fn, 
                        char *error_string, size_t error_string_len);
                        
/**
 *******************************************************************************
 * @ingroup COMMS
 * @description
 *    Wait for client to attempt connection establishment.
 *    WARNING:  This function shall block until the client has made a connection
 *              request to the server.
 *
 * @param handle            IN      Handle to the server side socket                
 * @param error_string      OUT     Buffer containing and error strings 
 *                                  returned from this function. It is the
 *                                  responsibility of the client to allocate
 *                                  sufficient memory to contain error string.
 * @param error_string_len  IN      Length of error_string.
 *
 * @retval -1         Failure - Invalid parameters passed. 
 * @retval -2         Failure - Unable to close socket. 
 * @retval >=0        Success. 
 ******************************************************************************/ 
int close_server_socket(int handle, char *error_string, size_t error_string_len);
                        
/**********---------------- CLIENT - SIDE ------------------*******************/ 

/**
 *******************************************************************************
 * @ingroup COMMS
 * @description
 *    Attempt to connect to the remote address specified.
 *
 * @param remote_address    OUT     Target address to connect to.            
 * @param error_string      OUT     Buffer containing and error strings 
 *                                  returned from this function. It is the
 *                                  responsibility of the client to allocate
 *                                  sufficient memory to contain error string.
 * @param error_string_len  IN      Length of error_string.
 *
 * @retval -1         Failure - Invalid parameters passed. 
 * @retval -2         Failure - Unable to create client-side socket. 
 * @retval -3         Failure - Unable to set client-side socket non-blocking. 
 * @retval -4         Failure - Unable to connect. Timeout. 
 * @retval -5         Failure - Unable to connect. Already connected.
 * @retval -6         Failure - Unable to connect. 
 * @retval >=0        Success. 
 ******************************************************************************/ 
int connect_to_server_socket(  COM_IP_ADDRESS  *remote_address, 
                               char *error_string, size_t error_string_len);

/**********---------------- General Utilities ------------------***************/ 

/**
 *******************************************************************************
 * @ingroup COMMS
 * @description
 *    Tests a file descriptor to see if it is still open.
 *
 * @param fd    IN     File descriptor to test.             
 *
 * @retval 1        Socket is open. 
 * @retval 0        Socket is not open. 
 ******************************************************************************/ 
static inline int iotests_fd_open(int fd)
{
	struct pollfd mypollfd;
	mypollfd.fd = fd;
	mypollfd.events = IOTESTS_CLOSED_POLL_FLAGS;

	if (poll(&mypollfd, 1, 0) == 0) 
		return 1;
	else
		return 0;
};

/**
 *******************************************************************************
 * @ingroup COMMS
 * @description
 *    Extract server connection details from command line arguments.
 *
 * @param argc          IN      The number of program arguments.              
 * @param argv          IN      The program arguments.
 * @param servers       OUT     Pointer to list of server connection structures.
 *
 * @retval -1         Failure - Invalid parameters passed. 
 * @retval -2         Failure - Can't translate server name into host identity. 
 * @retval >0         Success. Number of servers parsed. 
 ******************************************************************************/ 
int address_list_from_arguments(int server_count, char hosts[MAX_ADDRESSES][FILENAME_MAX], char ports[MAX_ADDRESSES][FILENAME_MAX], COM_IP_ADDRESS servers[]);

#ifdef __cplusplus
}
#endif 
#endif /* FILTER_COMMS_H */
