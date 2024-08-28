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
* File: tcp.c
* Date: Feb 27, 2012
* Author: LMI/LXR/SH John Lee
************************************************************************/

/**
 ******************************************************************************
 * @file tcp.c
 * @ingroup COMMS
 *      Source file implementation of a socket interface facade.
 ******************************************************************************/

/*******************************************************************************
* Include public/global header files
*******************************************************************************/
#include <stdlib.h>     /* atoi() prototype */
#include <errno.h>      /* UNIX function error returns */
#include <string.h>     /* strerror() prototype */
#include <memory.h>     /* Contains memcpy () & memset () */
#include <unistd.h>     /* Prototype for close() */
#include <netdb.h>      /* Contains gethostbyname () */
#include <sys/socket.h> /* Needed for socket operations */
#include <netinet/in.h> /* Network definitions */
#include <arpa/inet.h>  /* Prototype for inet_addr() */
#include <fcntl.h>      /* Prototype for fcntl() */
#include <sys/types.h>
#include <sys/socket.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <poll.h>

/*******************************************************************************
* Include private header files
*******************************************************************************/
#include <tcp.h>
#include <logger.h>


/*******************************************************************************
* Define Constants and Macros
*******************************************************************************/
#define ERROR_RETURN(handle,errno,error_string,error_string_len,error)	\
    strerror_r(errno,error_string,error_string_len); \
    if(handle) \
        close(handle); \
    return error;
    
#define VALIDATE_PORT(port) \
    if(port < 1 || port > HIGHEST_IP_PORT) \
		return -1;
    
#define VALIDATE_ERROR_BUFFER(error, error_string, error_string_len) \
    if((error_string==NULL) || (error_string_len<=0)) { \
        strerror_r(errno,error_string,error_string_len); \
        return error; \
    }
    
/**
 *******************************************************************************
 * @ingroup HTTP_SERVER
 * @description
 *  This function creates an Internet address in the form of a struct sockaddr 
 *  from a server name and port.
 ******************************************************************************/ 
static int get_sockaddr_in(const char* server_name, const int port, struct sockaddr_in *sp);

/**
 * open_server_socket
 */
int open_server_socket( int port, struct sockaddr_in *our_addr,
                        char *error_string, size_t error_string_len)
{
    int handle = 0, yes=1;
        
    /* Validate parameters */
    VALIDATE_PORT(port)
    VALIDATE_ERROR_BUFFER(-1, error_string, error_string_len)

    /* Open a TCP Socket */
    handle = socket(AF_INET, SOCK_STREAM, 0);
    if(handle == -1)
    {
        ERROR_RETURN(handle,errno,error_string,error_string_len,-2)
    }

    /* Bind our local Address to this socket */
    memset(&our_addr->sin_zero,0,sizeof(our_addr->sin_zero));
    our_addr->sin_family=AF_INET;
    our_addr->sin_addr.s_addr=htonl(INADDR_ANY);
    our_addr->sin_port=htons(port);

    if(setsockopt(handle,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
    {
        ERROR_RETURN(handle,errno,error_string,error_string_len,-3)
    }

    if(bind(handle, (struct sockaddr *) our_addr, sizeof(*our_addr)) < 0)
    {
        ERROR_RETURN(handle,errno,error_string,error_string_len,-4)
    }

    if(listen(handle,SOMAXCONN)<0)
    {
        ERROR_RETURN(handle,errno,error_string,error_string_len,-5)
    }
    return handle;
} 

/**
 * poll_server_socket
 */
int poll_server_socket( int handle, COM_CALL_BACK fn, char *error_string, 
                        size_t error_string_len)
{
    int unamed_handle=0,result=0;
    struct sockaddr_in client_addr = {0};
    socklen_t len=0;
    
    VALIDATE_ERROR_BUFFER(-1, error_string, error_string_len)
    if((handle<=0) || (NULL==fn))
        return -1;
    
    if((unamed_handle=accept(handle,(struct sockaddr *)&client_addr,&len))<0)
    {
        strerror_r(errno,error_string,sizeof(error_string));
	    return -2;
    }
    
    result = (*fn)(unamed_handle, client_addr);
    if(result<=0)
        close(unamed_handle);
    
    return result;
}

/**
 * close_server_socket
 */
int close_server_socket(int handle, char *error_string, size_t error_string_len)
{
    int result=0;
    
    VALIDATE_ERROR_BUFFER(-1, error_string, error_string_len)
    if(handle<=0) 
        return -1;

    if(close(handle)<0)
    {
        strerror_r(errno,error_string,sizeof(error_string));
        return -2;
    }  
    return result;    
}

/**
 * connect_to_server_socket
 */
int connect_to_server_socket(   COM_IP_ADDRESS  *remote_address, 
                                char *error_string, size_t error_string_len)
{
    int flags = 0, orig_flags = 0, x_err=0, connect_ret=0, tcpsendq = 0;
    fd_set rfd;
    fd_set wfd;
    struct timeval tv;
    
    VALIDATE_ERROR_BUFFER(-1, error_string, error_string_len)

    /* Connect hasn't succeeded before or first time */
    if(remote_address->handle<0)
    {
        remote_address->handle=socket(AF_INET,SOCK_STREAM,0);
        if(remote_address->handle < 0)
        {
            remote_address->handle=-1;
            ERROR_RETURN(remote_address->handle,errno,error_string,error_string_len,-2)
        }

        /* make socket non-blocking temporarily */
        if((flags = fcntl(remote_address->handle, F_GETFL, 0))<0)
        {
            remote_address->handle=-1;
            ERROR_RETURN(remote_address->handle,errno,error_string,error_string_len,-3)
        }
        
        orig_flags = flags;
        flags |= O_NONBLOCK;

        if(fcntl( remote_address->handle, F_SETFL, flags ) < 0 )
        {
            remote_address->handle=-1;
            ERROR_RETURN(remote_address->handle,errno,error_string,error_string_len,-3)
        }

        connect_ret = connect( remote_address->handle,
            (struct sockaddr*) &remote_address->address, sizeof(remote_address->address));

        if(connect_ret < 0)
        {
            if((errno == EINPROGRESS ) || (errno == EALREADY))
            {
                FD_ZERO(&rfd);
                FD_ZERO(&wfd);
                
                FD_SET(remote_address->handle,&rfd);
                FD_SET(remote_address->handle,&wfd);

                tv.tv_sec = 0;
                tv.tv_usec = 10000;

                /* Acts as a timer on the connect - if the select returns with
                 * a timeout or an error then we assume that connectivity is not
                 * possible and we reset the address handle to ensure retries
                 * otherwise we continue with the send. Note if the connect can 
                 * take longer than 10 milliseconds (10000 micro seconds)
                 * then this functionality will cause failure on valid tcp 
                 * connections */
                x_err = select(FD_SETSIZE, &rfd, &wfd, NULL, &tv);

                if((x_err == -1) || (x_err == 0))
                {
                    remote_address->handle=-1;
                    ERROR_RETURN(remote_address->handle,errno,error_string,error_string_len,-4)
                }
            }
            else if (errno == EISCONN)
            {
                /* The connection is already established */
                remote_address->handle=-1;
                ERROR_RETURN(remote_address->handle,errno,error_string,error_string_len,-5)
            }
            else
            {
                /* Cannot make the connection */
                remote_address->handle=-1;
                ERROR_RETURN(remote_address->handle,errno,error_string,error_string_len,-6)
            }
        }

            /* Return socket flag to blocked */
        if(fcntl(remote_address->handle, F_SETFL,orig_flags) < 0)
        {
            remote_address->handle=-1;
            ERROR_RETURN(remote_address->handle,errno,error_string,error_string_len,-6)
        }
        tcpsendq = 64*1024;
        setsockopt(remote_address->handle, SOL_SOCKET, SO_SNDBUF, &tcpsendq, sizeof(tcpsendq));
    }
    return 0;
}

/**
 * get_sockaddr_in
 */
static int get_sockaddr_in(const char* server_name, const int port, struct sockaddr_in *sp)
{        
    struct sockaddr_in local_address, *local=&local_address;
    
    /* Validate parameters */
	if((server_name == NULL) || (port < 1 || port > HIGHEST_IP_PORT)) {
		return -1;
	}
    
    /* Translate the server name into a host identity */
	struct hostent* server = gethostbyname(server_name);
    if(server == NULL) {
        return -2;
    }

    memset(local, 0, sizeof(struct sockaddr_in));
        
    //(char *)sp->sin_addr.s_addr
	// Set the address fields
	local->sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char *)&local->sin_addr.s_addr, server->h_length);
	local->sin_port = htons(port);
    memcpy(sp,local,sizeof(struct sockaddr_in));
	return 0;
}

/**
 *******************************************************************************
 * @ingroup COMMS
 * @description
 *    Extract server connection details from command line arguments.
 *
 * @param server_count  IN      The number of servers specified.
 * @param hosts         IN      The list of host names for servers.
 * @param ports         IN      The list of ports for servers.
 * @param servers       OUT     Pointer to list of server connection structures.
 *
 * @retval -1         Failure - Invalid parameters passed.
 * @retval -2         Failure - Can't translate server name into host identity.
 * @retval >0         Success. Number of servers parsed.
 ******************************************************************************/
int address_list_from_arguments(int server_count, char hosts[MAX_ADDRESSES][FILENAME_MAX], char ports[MAX_ADDRESSES][FILENAME_MAX], COM_IP_ADDRESS servers[])
{
    struct sockaddr_in address, *sp = &address;
    int result = 0;
    
    /* Check if too many addresses have been specified. */
	if (server_count > MAX_ADDRESSES) {
		return -1;
	}

    memset(sp, 0, sizeof(address));
	memset(servers, 0, sizeof(servers));

	/* Iterate over all arguments and barf on first error encountered */
	for (int i = 0; i < server_count; i++) {
		result = get_sockaddr_in(hosts[i], atoi(ports[i]), sp);
        if (result) {
            return result;
        }

		if (sp != NULL) {
			servers[i].address = (*sp);
		}
		else {
			return -2;
		}
	}
	return server_count;
}
