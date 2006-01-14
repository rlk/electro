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

#include <SDL.h>
#include <SDL_thread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "utility.h"
#include "socket.h"
#include "server.h"
#include "net.h"

/*---------------------------------------------------------------------------*/

#define MAXCONN 4

static int    done = 0;
static int    port = 0;
static SOCKET sock = 0;
static SOCKET conn[MAXCONN];

/*---------------------------------------------------------------------------*/

void net_send_all(const char *str)
{
    if (port)
    {
        int i;

        /* Send output to all connected clients. */

        for (i = 0; i < MAXCONN; ++i)
            if (conn[i] >= 0)
                write(conn[i], str, strlen(str));
    }
}

static void net_conn(void)
{
    sockaddr_t addr;
    socklen_t  len = sizeof (sockaddr_t);

    int i;

    /* Search for an available connection slot. */

    for (i = 0; i < MAXCONN; ++i)
        if (conn[i] == -1)
            break;

    /* If a slot is found, accept the connection. */

    if (i < MAXCONN)
    {
        if ((conn[i] = accept(sock, (struct sockaddr *) &addr, &len)) >= 0)
        {
            return;
        }
        else error("accept : %s", system_error());
    }
    else error("connection limit exceeded");
}

static void net_data(int i)
{
    char str[MAXSTR];
    int n;

    memset(str, 0, MAXSTR);

    /* Read an incoming command. */

    if ((n = read(conn[i], str, MAXSTR)) >= 0)
    {
        /* Enqueue the command, or close the connection on zero. */

        if (n > 0)
            send_user_event(str);
        else
        {
            close(conn[i]);
            conn[i] = -1;
        }
    }
    else error("read : %s", system_error());
}

static void net_loop(void)
{
    int i, n = sock + 1;
    fd_set fds;

    /* Initialize the socket descriptor set with all connected sockets. */

    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    for (i = 0; i < MAXCONN; ++i)
    {
        if (conn[i] >= 0)
            FD_SET(conn[i], &fds);

        if (n < (int) conn[i] + 1)
            n = (int) conn[i] + 1;
    }

    /* Check for activity on all sockets. */

    if (select(n, &fds, NULL, NULL, NULL) >= 0)
    {
        /* Check for a new connection. */

        if (FD_ISSET(sock, &fds))
            net_conn();

        /* Check for data on an existing connection. */

        for (i = 0; i < MAXCONN; ++i)
            if (conn[i] >= 0 && FD_ISSET(conn[i], &fds))
                net_data(i);
    }
    else error("select : %s", system_error());
}

static int net_main(void *data)
{
    int i;

    /* Initialize the array of connections. */

    for (i = 0; i < MAXCONN; ++i)
        conn[i] = -1;

    /* Create a new TCP socket for incomming sessions. */

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) >= 0)
    {
        sockaddr_t addr;

        addr.sin_family      = AF_INET;
        addr.sin_port        = htons((short) port);
        addr.sin_addr.s_addr = INADDR_ANY;

        /* Bind the new socket and listen for incoming connections. */

        if (bind(sock, (struct sockaddr *) &addr, sizeof (sockaddr_t)) >= 0)
        {
            if (listen(sock, MAXCONN) >= 0)
            {
                while (!done)
                    net_loop();
            }
            else error("listen : %s", system_error());
        }
        else error("bind : %s", system_error());
    }
    else error("socket : %s", system_error());

    return 0;
}

/*---------------------------------------------------------------------------*/

int startup_net(int p)
{
    if ((port = p))
    {
        if (SDL_CreateThread(net_main, NULL))
            return 1;
        else
            return 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/
