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

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

/*
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
*/
#include "onitcs.h"

//------------------------------------------------------------------------------

#define SHMERR ((struct shm_head *) (-1))

struct shm_head
{
    int naxes;
    int nbuttons;
    int npoints;
};

static int              sem_id = -1;
static int              shm_id = -1;
static struct shm_head *shm_p  = SHMERR;

//------------------------------------------------------------------------------

static int ipc_ok()
{
    return (sem_id != -1) && (shm_id != -1) && (shm_p != SHMERR);
}

static void *shm_ptr(size_t off)
{
    return ipc_ok() ? ((char *) shm_p + off) : 0;
}

//------------------------------------------------------------------------------

static int sem_init(int key)
{
    int id;
    
    // If the semaphores already exist, then use them with their current values.

    if ((id = semget(key, 3, 0666)) >= 0)
        return id;
                        
    // If they need to be created, then they need to be initialized.

    if ((id = semget(key, 3, 0666 | IPC_CREAT)) >= 0)
    {
        semctl(id, 0, SETVAL, 1);
        semctl(id, 1, SETVAL, 1);
        semctl(id, 2, SETVAL, 1);
        return id;
    }
    else
        perror("semget");

    return -1;
}

static void ipc_init(int key, int na, int nb, int np)
{
    // Initialize the semaphore and shared memory segment.

    size_t size = 4096;

    if ((sem_id = sem_init(key)) >= 0)
    {
        if ((shm_id = shmget(key, size, 0666 | IPC_CREAT)) >= 0)
        {
            if ((shm_p = (struct shm_head *) shmat(shm_id, 0, 0)) != SHMERR)
            {
                if (na || nb || np)
                {
                    memset(shm_p, 0, size);

                    shm_p->naxes    = na;
                    shm_p->nbuttons = nb;
                    shm_p->npoints  = np;
                }
            }
            else perror("shmat");
        }
        else perror("shmget");
    }
}

static void ipc_free()
{
/*
    if (shm_p  != SHMERR) shmdt(shm_p);
    if (shm_id !=     -1) shmctl(shm_id, IPC_RMID, 0);
    if (sem_id !=     -1) semctl(sem_id, IPC_RMID, 0);
*/
}

static void ipc_acquire(int i)
{
    struct sembuf s;

    s.sem_num =  i;
    s.sem_op  = -1;
    s.sem_flg =  0;

    semop(sem_id, &s, 1);
}

static void ipc_release(int i)
{
    struct sembuf s;

    s.sem_num =  i;
    s.sem_op  =  1;
    s.sem_flg =  0;

    semop(sem_id, &s, 1);
}

//------------------------------------------------------------------------------

static float *get_axes()
{
    return (float *) shm_ptr(sizeof (struct shm_head));
}

static int *get_buttons()
{
    return (int *) shm_ptr(sizeof (struct shm_head) +
                           sizeof (float) * shm_p->naxes);
}

static onit_point *get_points()
{
    return (onit_point *) shm_ptr(sizeof (struct shm_head) +
                                  sizeof (float) * shm_p->naxes +
                                  sizeof (int)   * shm_p->nbuttons);
}

//------------------------------------------------------------------------------

int onitcs_init(int na, int nb, int np)
{
    ipc_init(25670, na, nb, np);
    return ipc_ok();
}

void onitcs_fini()
{
    ipc_free();
}

int onitcs_naxes()
{
    return ipc_ok() ? shm_p->naxes : 0;
}

int onitcs_nbuttons()
{
    return ipc_ok() ? shm_p->nbuttons : 0;
}

int onitcs_npoints()
{
    return ipc_ok() ? shm_p->npoints : 0;
}

//------------------------------------------------------------------------------

float *onitcs_acquire_axes()
{
    if (ipc_ok())
    {
        ipc_acquire(0);
        return get_axes();
    }
    return 0;
}

int *onitcs_acquire_buttons()
{
    if (ipc_ok())
    {
        ipc_acquire(1);
        return get_buttons();
    }
    return 0;
}

onit_point *onitcs_acquire_points()
{
    if (ipc_ok())
    {
        ipc_acquire(2);
        return get_points();
    }
    return 0;
}

//------------------------------------------------------------------------------

void onitcs_release_axes()
{
    ipc_release(0);
}

void onitcs_release_buttons()
{
    ipc_release(1);
}

void onitcs_release_points()
{
    ipc_release(2);
}

//------------------------------------------------------------------------------

#if 0
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
*/
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

static size_t sock_read_array(int sock, void *buf, size_t len)
{
    unsigned short n;

    // Read a 16-bit message length and read repeatedly until the full message
    // is received. Break on zero or error read.

    if ((read(sock, &n, sizeof (unsigned short))) != -1)
    {
        size_t t = (size_t) n < len ? (size_t) n : len;
        size_t s = 0;
        size_t r = 1;

        for (s = 0; t > 0 && r > 0; s += r, t -= r)
            r = read(sock, (unsigned char *) buf + s, t);

        return s;
    }
    return 0;
}

static size_t sock_send_array(int sock, void *buf, size_t len)
{
    unsigned short n = (unsigned short) len;

    // Send the given message preceded by its 16-bit length.

    if (send(sock, &n, sizeof (unsigned short), MSG_NOSIGNAL) != SOCKET_ERROR)
    {
        if (send(sock, buf, len, MSG_NOSIGNAL) != SOCKET_ERROR)
        {
            return len;
        }
        else perror("send");
    }
    else perror("send");

    return 0;
}

//------------------------------------------------------------------------------

static int sock_init_set(fd_set *s, int *sock, int num)
{
    int i, m = -1;

    // Initialize the given descriptor set with all valid entries in the given
    // socket array.

    FD_ZERO(s);

    for (i = 0; i < num; ++i)
    {
        if (sock[i] != INVALID_SOCKET)
        {
            FD_SET(sock[i], s);
            if (m < sock[i])
                m = sock[i];
        }
    }

    return m + 1;
}

static size_t sock_recv_any(int *sock, int num, void *buf, size_t len)
{
    int i, n, m;
    fd_set s;
    size_t r;

    // If any of sockets is readable then read the contents to the given buffer.
    // If it reads empty or error then disconnect it. Return the read size.

    if ((m = sock_init_set(&s, sock, num)) > 0)
    {
        if ((n = select(m, &s, 0, 0, &timeout)) != SOCKET_ERROR)
        {
            for (i = 0; n && i < num; ++i)
            {
                if (sock[i] != INVALID_SOCKET)
                {
                    if (FD_ISSET(sock[i], &s))
                    {
                        if ((r = sock_read_array(sock[i], buf, len)))
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
    }

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
            if (sock_send_array(sock[i], buf, len) == 0)
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
#endif // 0
//------------------------------------------------------------------------------
