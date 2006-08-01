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
#include <ctype.h>

#include "utility.h"
#include "socket.h"
#include "server.h"
#include "net.h"

/*---------------------------------------------------------------------------*/

#define MAXCONN 4

static int    done = 0;
static int    port = 0;
static SOCKET sock = 0;

/*---------------------------------------------------------------------------*/

struct client
{
    SOCKET sock;

    int  len;
    char str[MAXSTR];
};

static struct client conn[MAXCONN];

/*---------------------------------------------------------------------------*/

void net_send_all(const char *str)
{
    if (port)
    {
        int i;

        /* Send output to all connected clients. */

        for (i = 0; i < MAXCONN; ++i)
            if (conn[i].sock >= 0)
                write(conn[i].sock, str, strlen(str));
    }
}

static void net_conn(void)
{
    sockaddr_t addr;
    socklen_t  n = sizeof (sockaddr_t);

    int i;

    /* Search for an available connection slot. */

    for (i = 0; i < MAXCONN; ++i)
        if (conn[i].sock == -1)
            break;

    /* If a slot is found, accept the connection. */

    if (i < MAXCONN)
    {
        if ((conn[i].sock = accept(sock, (struct sockaddr *) &addr, &n)) >= 0)
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
    int c, n;

    memset(str, 0, MAXSTR);

    /* Read an incoming command. */

    if ((n = read(conn[i].sock, str, MAXSTR)) >= 0)
    {
        /* Accumulate the command.  Activate it on EOL. */

        for (c = 0; c < n && conn[i].len < MAXSTR; ++c)
        {
            if (isprint(str[c]))
                conn[i].str[conn[i].len++] = str[c];

            if ((str[c] == 13 || str[c] == 10) && conn[i].len > 0)
            {
                send_user_event(conn[i].str);
                memset(conn[i].str, 0, MAXSTR);
                conn[i].len = 0;
            }
        }

        /* Close the connection on zero. */

        if (n == 0)
        {
            close(conn[i].sock);
            conn[i].sock = -1;
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
        if (conn[i].sock >= 0)
            FD_SET(conn[i].sock, &fds);

        if (n < (int) conn[i].sock + 1)
            n = (int) conn[i].sock + 1;
    }

    /* Check for activity on all sockets. */

    if (select(n, &fds, NULL, NULL, NULL) > 0)
    {
        /* Check for a new connection. */

        if (FD_ISSET(sock, &fds))
            net_conn();

        /* Check for data on an existing connection. */

        for (i = 0; i < MAXCONN; ++i)
            if (conn[i].sock >= 0 && FD_ISSET(conn[i].sock, &fds))
                net_data(i);
    }
}

static int net_main(void *data)
{
    int i;

    /* Initialize the array of connections. */

    for (i = 0; i < MAXCONN; ++i)
    {
        conn[i].sock = -1;
        conn[i].len  =  0;

        memset(conn[i].str, 0, MAXSTR);
    }

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
