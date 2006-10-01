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

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef _WIN32
typedef unsigned int uint32_t;
#else
#include <sys/shm.h>
#endif

#include "tracker.h"
#include "utility.h"
#include "matrix.h"
#include "socket.h"

/*---------------------------------------------------------------------------*/

struct tracker_header
{
    uint32_t version;
    uint32_t count;
    uint32_t offset;
    uint32_t size;
    uint32_t time[2];
    uint32_t command;
};

struct control_header
{
    uint32_t version;
    uint32_t but_offset;
    uint32_t val_offset;
    uint32_t but_count;
    uint32_t val_count;
    uint32_t time[2];
    uint32_t command;
};

struct sensor
{
    float    p[3];
    float    r[3];
    uint32_t t[2];
    uint32_t calib;
    uint32_t frame;
};

/*---------------------------------------------------------------------------*/

#ifndef _WIN32
static int tracker_id = -1;
static int control_id = -1;
#else
static volatile HANDLE tracker_id = NULL;
static volatile HANDLE control_id = NULL;
#endif

static struct tracker_header *tracker = (struct tracker_header *) (-1);
static struct control_header *control = (struct control_header *) (-1);
static uint32_t              *buttons = NULL;

/*---------------------------------------------------------------------------*/

struct message
{
    float x;
    float y;
    float z;
};

static int sock = INVALID_SOCKET;

/*---------------------------------------------------------------------------*/

struct transform
{
    float M[16];
};

static struct transform *transform  = NULL;
static int               transforms = 0;

/*---------------------------------------------------------------------------*/

int acquire_tracker(int t_key, int c_key, int port)
{
    int i;

    /* Acquire the tracker and controller shared memory segments. */

#ifndef _WIN32
    if ((tracker_id = shmget(t_key, sizeof (struct tracker_header), 0)) >= 0)
        tracker = (struct tracker_header *) shmat(tracker_id, 0, 0);

    if ((control_id = shmget(c_key, sizeof (struct control_header), 0)) >= 0)
        control = (struct control_header *) shmat(control_id, 0, 0);
#else
    char shmkey[256];

    sprintf(shmkey, "%d", t_key);
    if ((tracker_id = OpenFileMapping(FILE_MAP_WRITE, FALSE, shmkey)))
        tracker = (struct tracker_header *)
                    MapViewOfFile(tracker_id, FILE_MAP_WRITE, 0, 0, 0);
    else
        tracker = (struct tracker_header *) (-1);

    sprintf(shmkey, "%d", c_key);
    if ((control_id = OpenFileMapping(FILE_MAP_WRITE, FALSE, shmkey)))
        control = (struct control_header *)
                    MapViewOfFile(control_id, FILE_MAP_WRITE, 0, 0, 0);
    else
        control = (struct control_header *) (-1);
#endif

    /* Allocate storage for button states. */

    if (control != (struct control_header *) (-1))
        buttons = (uint32_t *) calloc(control->but_count, sizeof (uint32_t));

    /* Open a UDP socket for receiving. */

    if (port)
    {
        if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) >= 0)
        {
            sockaddr_t addr;

            /* Accept connections from any address on the given port. */

            addr.sin_family      = AF_INET;
            addr.sin_port        = htons((short) port);
            addr.sin_addr.s_addr = htonl(INADDR_ANY);

            /* Bind the socket to this address. */

            if (bind(sock, (struct sockaddr *) &addr, sizeof (sockaddr_t))>= 0)
            {
                transforms = 1;
            }
            else error("bind %d : %s", port, system_error());
        }
        else error("socket : %s", system_error());
    }

    /* Initialize the tracker transform. */

    if (tracker != (struct tracker_header *) (-1))
        transforms = (int) tracker->count;

    if ((transform = (struct transform *) calloc(transforms,
                                                 sizeof (struct transform))))
    {
        for (i = 0; i < transforms; ++i)
        {
            transform[i].M[0]  = 1.0f;
            transform[i].M[5]  = 1.0f;
            transform[i].M[10] = 1.0f;
            transform[i].M[15] = 1.0f;
        }
    }

    return 1;
}

void release_tracker(void)
{
    /* Detach shared memory segments. */

#ifndef _WIN32
    if (control != (struct control_header *) (-1)) shmdt(control);
    if (tracker != (struct tracker_header *) (-1)) shmdt(tracker);

    control_id = -1;
    tracker_id = -1;
#else
    if (control != (struct control_header *) (-1)) UnmapViewOfFile(control);
    if (tracker != (struct tracker_header *) (-1)) UnmapViewOfFile(tracker);

    CloseHandle(control_id);
    CloseHandle(tracker_id);

    control_id = NULL;
    tracker_id = NULL;
#endif

    if (sock != INVALID_SOCKET) close(sock);

    if (buttons)   free(buttons);
    if (transform) free(transform);

    /* Mark everything as uninitialized. */

    sock = INVALID_SOCKET;

    control = (struct control_header *) (-1);
    tracker = (struct tracker_header *) (-1);

    buttons    = NULL;
    transform  = NULL;
    transforms = 0;
}

/*---------------------------------------------------------------------------*/

int get_tracker_status(void)
{
    return ((sock    != INVALID_SOCKET) ||
            (tracker != (struct tracker_header *) (-1)));
}

int get_tracker_rotation(unsigned int id, float R[16])
{
    if (tracker != (struct tracker_header *) (-1))
    {
        if (id < tracker->count)
        {
            /* Return the rotation of sensor ID. */

            struct sensor *S =
                (struct sensor *)((unsigned char *) tracker
                                                  + tracker->offset
                                                  + tracker->size * id);
#define REORDER 1
            /* TODO: Fix potential gimbal lock here? */

#ifdef REORDER
            load_rot_mat(R, 0, 1, 0, S->r[0]); /* azimuth   */
            mult_rot_mat(R, 1, 0, 0, S->r[1]); /* elevation */
            mult_rot_mat(R, 0, 0, 1, S->r[2]); /* roll      */
#else
            load_rot_mat(R, 1, 0, 0, S->r[1]); /* elevation */
            mult_rot_mat(R, 0, 1, 0, S->r[0]); /* azimuth   */
            mult_rot_mat(R, 0, 0, 1, S->r[2]); /* roll      */
#endif
            mult_mat_mat(R, R, transform[id].M);

            return 1;
        }
    }
    else load_idt(R);

    return 0;
}

int get_tracker_position(unsigned int id, float p[3])
{
    static float last_p[3] = { 0.0f, 0.0f, 0.0f };

    p[0] = last_p[0];
    p[1] = last_p[1];
    p[2] = last_p[2];

    if (tracker != (struct tracker_header *) (-1))
    {
        if (id < tracker->count)
        {
            /* Return the position of sensor ID. */

            struct sensor *S =
                (struct sensor *)((unsigned char *) tracker
                                                  + tracker->offset
                                                  + tracker->size * id);

            if (fabs(S->p[0]) > 0.0 ||
                fabs(S->p[1]) > 0.0 ||
                fabs(S->p[2]) > 0.0)
                mult_mat_pos(p, transform[id].M, S->p);
        }
    }

    /* Recieve messages on the tracker socket.  Return the most recent. */

    if (id == 0 && sock != INVALID_SOCKET)
    {
        struct message mesg;
        struct timeval zero = { 0, 0 };

        fd_set fds;

        FD_ZERO(&fds);
        FD_SET(sock, &fds);

        while (select(sock + 1, &fds, NULL, NULL, &zero) > 0)
        {            
            if (FD_ISSET(sock, &fds))
            {
                ssize_t s;

                if ((s = recv(sock, &mesg, sizeof (struct message), 0)) ==
                                           sizeof (struct message))
                {
                    float q[3];

                    q[0] = net_to_host_float(mesg.x);
                    q[1] = net_to_host_float(mesg.y);
                    q[2] = net_to_host_float(mesg.z);

                    mult_mat_pos(p, transform[id].M, q);
                }
            }

            FD_ZERO(&fds);
            FD_SET(sock, &fds);
        }
    }

    /* If neither tracking source was updated, return the previous result. */

    last_p[0] = p[0];
    last_p[1] = p[1];
    last_p[2] = p[2];

    return 1;
}

int get_tracker_joystick(unsigned int id, float a[2])
{
    if (control != (struct control_header *) (-1))
    {
        float *p = (float *) ((unsigned char *) control + control->val_offset);

        /* Return valuators ID and ID + 1. */
        
        if (id < control->val_count - 1)
        {
            a[0] = +(*(p + id + 0));
            a[1] = -(*(p + id + 1));

            return 1;
        }
    }
    return 0;
}

int get_tracker_buttons(unsigned int *id, unsigned int *st)
{
    uint32_t  i;
    uint32_t *p;

    if (buttons && control != (struct control_header *) (-1))
    {
        p = (uint32_t *) ((unsigned char *) control + control->but_offset);

        /* Seek the first button that does not match its cached state. */

        for (i = 0; i < control->but_count; ++i, ++p)
            if (buttons[i] != *p)
            {
                /* Update the cache and return the button ID and state. */
            
                buttons[i]  = *p;

                *id =  i + 1;
                *st = *p;

                return 1;
            }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void set_tracker_transform(unsigned int id, float M[16])
{
    if (transform && (int) id < transforms)
        memcpy(transform[id].M, M, 16 * sizeof (float));
}

void get_tracker_transform(unsigned int id, float M[16])
{
    if (transform && (int) id < transforms)
        memcpy(M, transform[id].M, 16 * sizeof (float));
}

/*---------------------------------------------------------------------------*/
