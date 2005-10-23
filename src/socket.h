/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    ELECTRO is free software;  you can redistribute it and/or modify it    */
/*    under the terms of the  GNU General Public License  as published by    */
/*    the  Free Software Foundation;  either version 2 of the License, or    */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#ifndef SOCKET_H
#define SOCKET_H

/*---------------------------------------------------------------------------*/
/* These definitions homogenize Winsock with Berkeley Sockets.               */

#ifdef _WIN32
#include <winsock.h>
#else
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

typedef int SOCKET;
#define SOCKET_ERROR    -1
#define INVALID_SOCKET  -1
#endif

typedef struct sockaddr_in sockaddr_t;

/*---------------------------------------------------------------------------*/

#endif
