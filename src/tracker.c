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

#include <stdlib.h>
#include <sys/types.h>

#ifdef _WIN32
typedef unsigned int uint32_t;
#else
#include <sys/shm.h>
#endif

#include "tracker.h"
#include "utility.h"

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

static int tracker_id = -1;
static int control_id = -1;

static struct tracker_header *tracker = (struct tracker_header *) (-1);
static struct control_header *control = (struct control_header *) (-1);
static uint32_t              *buttons = NULL;

/*---------------------------------------------------------------------------*/

int acquire_tracker(int t_key, int c_key)
{
#ifndef _WIN32
    /* Acquire the tracker and controller shared memory segments. */

    if ((tracker_id = shmget(t_key, sizeof (struct tracker_header), 0)) >= 0)
        tracker = (struct tracker_header *) shmat(tracker_id, 0, 0);

    if ((control_id = shmget(c_key, sizeof (struct control_header), 0)) >= 0)
        control = (struct control_header *) shmat(control_id, 0, 0);

    /* Allocate storage for button states. */

    if (control != (struct control_header *) (-1))
        buttons = (uint32_t *) calloc(control->but_count, sizeof (uint32_t));
#endif

    /* Return an indication of successful attachment to shared memory. */

    return ((tracker != (struct tracker_header *) (-1)) &&
            (control != (struct control_header *) (-1)) && buttons);
}

void release_tracker(void)
{
#ifndef _WIN32
    /* Detach shared memory segments. */

    if (control != (struct control_header *) (-1)) shmdt(control);
    if (tracker != (struct tracker_header *) (-1)) shmdt(tracker);

    if (buttons) free(buttons);
#endif

    /* Mark everything as uninitialized. */

    control = (struct control_header *) (-1);
    tracker = (struct tracker_header *) (-1);

    buttons = NULL;

    control_id = -1;
    tracker_id = -1;
}

/*---------------------------------------------------------------------------*/

int get_tracker_status(void)
{
    return ((tracker != (struct tracker_header *) (-1)) &&
            (control != (struct control_header *) (-1)) && buttons);
}

int get_tracker_rotation(unsigned int id, float r[3])
{
    if (tracker != (struct tracker_header *) (-1))
    {
        if (0 <= id && id < tracker->count)
        {
            /* Return the rotation of sensor ID. */

            struct sensor *S =
                (struct sensor *)((unsigned char *) tracker
                                                  + tracker->offset
                                                  + tracker->size * id);
            r[0] = S->r[1];   /* elevation */
            r[1] = S->r[0];   /* azimuth   */
            r[2] = S->r[2];   /* roll      */

            return 1;
        }
    }
    return 0;
}

int get_tracker_position(unsigned int id, float p[3])
{
    if (tracker != (struct tracker_header *) (-1))
    {
        if (0 <= id && id < tracker->count)
        {
            /* Return the position of sensor ID. */

            struct sensor *S =
                (struct sensor *)((unsigned char *) tracker
                                                  + tracker->offset
                                                  + tracker->size * id);
            p[0] = S->p[0];
            p[1] = S->p[1];
            p[2] = S->p[2];

            return 1;
        }
    }
    return 0;
}

int get_tracker_joystick(unsigned int id, float a[2])
{
    if (control != (struct control_header *) (-1))
    {
        float *p = (float *) ((unsigned char *) control + control->val_offset);

        /* Return valuators ID and ID + 1. */
        
        if (0 <= id && id < control->val_count - 1)
        {
            a[0] = *(p + id + 0);
            a[1] = *(p + id + 1);

            return 1;
        }
    }
    return 0;
}

int get_tracker_buttons(unsigned int *id, unsigned int *st)
{
    if (buttons && control != (struct control_header *) (-1))
    {
        uint32_t i, *p = (uint32_t *)
							((unsigned char *) control + control->but_offset);

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
