// OPENNI TRACKER Copyright (C) 2011 Robert Kooima
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//------------------------------------------------------------------------------

#ifdef _WIN32
#include <io.h>
#include <winsock2.h>

typedef int socklen_t;

#else
#include <stddef.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#define SOCKET_ERROR   -1
#define INVALID_SOCKET -1
#endif

//------------------------------------------------------------------------------

#include "onitcs.h"

#define MAXCLI 4

struct onit_server
{
    int client_sock[MAXCLI];
    int listen_sock;
};

struct onit_client
{
    int server_sock;
};

static struct timeval timeout = { 0, 0 };

//------------------------------------------------------------------------------

static int sock_no_delay(int sock)
{
    // We require minimal latency and no response, so disable the Nagle
    // algorithm on the given socket.

    socklen_t len = (socklen_t) sizeof (int);
    int       val = 1;

    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                   (const char *) &val, len) == SOCKET_ERROR)
        perror("setsockopt");

    return sock;
}

//------------------------------------------------------------------------------

static int sock_connect(const char *node, const char *service)
{
    struct addrinfo *r, h;
    int              e, s;

    memset(&h, 0, sizeof (struct addrinfo));

    // Enumerate the addresses for the given remote node name and service.
    // Try to connect to each and return the first success.

    h.ai_family   = AF_UNSPEC;
    h.ai_socktype = SOCK_STREAM;

    for (e = getaddrinfo(node, service, &h, &r); !e && r; r = r->ai_next)
    {
        if ((s = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) != -1)
        {
            if (connect(s, r->ai_addr, r->ai_addrlen) != SOCKET_ERROR)
            {
                return s;
            }
            close(s);
        }
    }
    return INVALID_SOCKET;
}

//------------------------------------------------------------------------------

static int sock_bind_listen(const char *service)
{
    struct addrinfo *r, h;
    int              e, s;

    memset(&h, 0, sizeof (struct addrinfo));

    // Enumerate the addresses for the given local service. Try to bind each.
    // Listen and return with the first success.

    h.ai_family   = AF_UNSPEC;
    h.ai_flags    = AI_PASSIVE;
    h.ai_socktype = SOCK_STREAM;

    for (e = getaddrinfo(0, service, &h, &r); !e && r; r = r->ai_next)
    {
        if ((s = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) != -1)
        {
            if (bind(s, r->ai_addr, r->ai_addrlen) != SOCKET_ERROR)
            {
                if (listen(s, MAXCLI) != SOCKET_ERROR)
                {
                    return s;
                }
            }
            close(s);
        }
    }
    return INVALID_SOCKET;
}

//------------------------------------------------------------------------------

static int sock_poll_accept(int sock)
{
    // If the listen socket is readable then accept a client connection.
    // If connection succeeds then return the new socket.

    fd_set s;
    int n, d;

    FD_ZERO(&s);
    FD_SET(sock, &s);

    if ((n = select(sock + 1, &s, 0, 0, &timeout)) != SOCKET_ERROR)
    {
        if (n)
        {
            if ((d = accept(sock, 0, 0)) != INVALID_SOCKET)
            {
                return sock_no_delay(d);
            }
            else perror("accept");
        }
    }
    else perror("select");

    return INVALID_SOCKET;
}

//------------------------------------------------------------------------------

static size_t sock_recv_any(int *sock, int num, void *buf, size_t len)
{
    // Prepare a socket descriptor set including all given valid sockets.

    int i, n, m = -1;
    fd_set s;
    size_t r;

    FD_ZERO(&s);

    for (i = 0; i < num; ++i)
    {
        if (sock[i] != INVALID_SOCKET)
        {
            FD_SET(sock[i], &s);
            if (m < sock[i])
                m = sock[i];
        }
    }

    // If any of sockets is readable then read the contents to the given buffer.
    // If it reads empty or error then disconnect it. Return the read size.

    if ((n = select(m + 1, &s, 0, 0, &timeout)) != SOCKET_ERROR)
    {
        for (i = 0; n && i < num; ++i)
        {
            if (sock[i] != INVALID_SOCKET)
            {
                if (FD_ISSET(sock[i], &s))
                {
                    if ((r = read(sock[i], buf, len)))
                        return r;
                    else
                    {
                        close(sock[i]);
                        sock[i] = INVALID_SOCKET;
                    }
                }
            }
        }
    }
    else perror("select");

    return 0;
}

//------------------------------------------------------------------------------

static void sock_send_all(int *sock, int num, void *buf, size_t len)
{
    // Write the contents of the given buffer to all given valid sockets.
    // Disconnect on error.

    int i;

    for (i = 0; i < num; ++i)
    {
        if (sock[i] != INVALID_SOCKET)
        {
//          if (write(sock[i], buf, len) == SOCKET_ERROR)
            if (send(sock[i], buf, len, MSG_NOSIGNAL) == SOCKET_ERROR)
            {
                close(sock[i]);
                sock[i] = INVALID_SOCKET;
            }
        }
    }
}

//------------------------------------------------------------------------------
// Server API

onit_server *onit_server_init(const char *service)
{
    // Allocate and initialize a server state structure.

    onit_server *S;
    int i;

    if ((S = (onit_server *) calloc(1, sizeof (onit_server))))
    {
        for (i = 0; i < MAXCLI; ++i)
            S->client_sock[i] = INVALID_SOCKET;

        S->listen_sock = sock_bind_listen(service);
    }
    return S;
}

void onit_server_fini(onit_server *S)
{
    // Close all sockets and release the server structure.

    int i;

    for (i = 0; i < MAXCLI; ++i)
        close(S->client_sock[i]);

    close(S->listen_sock);
    free(S);
}

void onit_server_poll(onit_server *S)
{
    int  sock;
    int  i;

    // If a new connection is pending then add it to the list.

    if (S->listen_sock != INVALID_SOCKET)
    {
        if ((sock = sock_poll_accept(S->listen_sock)) != INVALID_SOCKET)
        {
            for (i = 0; i < MAXCLI; ++i)
                if (S->client_sock[i] == INVALID_SOCKET)
                {
                    S->client_sock[i] = sock;
                    break;
                }

            if (i == MAXCLI) close(sock);
        }
    }
}

void onit_server_send(onit_server *S, void *buf, size_t len)
{
    char tmp[256];
    sock_recv_any(S->client_sock, MAXCLI, tmp, 256);
    sock_send_all(S->client_sock, MAXCLI, buf, len);
}

//------------------------------------------------------------------------------
// Client API

onit_client *onit_client_init(const char *node, const char *service)
{
    // Allocate and initialize a client state structure.

    onit_client *C;

    if ((C = (onit_client *) calloc(1, sizeof (onit_client))))
    {
        C->server_sock = sock_connect(node, service);
    }
    return C;
}

void onit_client_fini(onit_client *C)
{
    // Close the socket and release the client structure.

    close(C->server_sock);
    free(C);
}

size_t onit_client_recv(onit_client *C, void *buf, size_t len)
{
    return sock_recv_any(&C->server_sock, 1, buf, len);
}

//------------------------------------------------------------------------------
