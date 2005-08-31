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
#include <sys/shm.h>

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
static int                   *buttons = NULL;

/*---------------------------------------------------------------------------*/

int acquire_tracker(int t_key, int c_key)
{
    /* Acquire the tracker and controller shared memory segments. */

    if ((tracker_id = shmget(t_key, sizeof (struct tracker_header), 0)) >= 0)
        tracker = (struct tracker_header *) shmat(tracker_id, 0, 0);

    if ((control_id = shmget(c_key, sizeof (struct control_header), 0)) >= 0)
        control = (struct control_header *) shmat(control_id, 0, 0);

    /* Allocate storage for button states. */

    if (control != (struct control_header *) (-1))
        buttons = (int *) calloc(control->but_count, sizeof (int));

    /* Return an indication of successful attachment to shared memory. */

    return ((tracker != (struct tracker_header *) (-1)) &&
            (control != (struct control_header *) (-1)) && buttons);
}

void release_tracker(void)
{
    /* Detach and remove shared memory segments. */

    if (control != (struct control_header *) (-1)) shmdt(control);
    if (tracker != (struct tracker_header *) (-1)) shmdt(tracker);

    if (buttons) free(buttons);

    /*
    if (control_id >= 0) shmctl(control_id, IPC_RMID, 0);
    if (tracker_id >= 0) shmctl(tracker_id, IPC_RMID, 0);
    */
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

int get_tracker_rotation(int id, float r[3])
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

int get_tracker_position(int id, float p[3])
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

int get_tracker_joystick(int id, float a[2])
{
    if (control != (struct control_header *) (-1))
    {
        float *p = (float *) ((unsigned char *) control + control->val_offset);

        /* Return valuators ID and ID + 1. */
        
        if (0 <= id && id <= control->val_count - 1)
        {
            a[0] = *(p + id + 0);
            a[1] = *(p + id + 1);

            return 1;
        }
    }
    return 0;
}

int get_tracker_buttons(int *id, int *st)
{
    if (buttons && control != (struct control_header *) (-1))
    {
        int *p = (int *) ((unsigned char *) control + control->but_offset);
        int  i;

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

#ifdef SNIP

/*---------------------------------------------------------------------------*/

#ifdef TRACKD
#include <trackdAPI.h>
#endif

/*---------------------------------------------------------------------------*/

static void *tracker = NULL;
static void *control = NULL;
static int  *buttons = NULL;

/*---------------------------------------------------------------------------*/

int startup_tracker(int tkey, int ckey)
{
#ifdef TRACKD
    tracker = trackdInitTrackerReader(tkey);
    control = trackdInitControllerReader(ckey);

    if (tracker == NULL || control == NULL)
    {
        error("Tracker/controller startup failure");
        return 0;
    }

    buttons = (int *) calloc(trackdGetNumberOfButtons(control), sizeof (int));

#else
    tracker = NULL;
    control = NULL;
    buttons = NULL;
#endif

    return 1;
}

/*---------------------------------------------------------------------------*/

int get_tracker_status(void)
{
#ifdef TRACKD
    return (tracker && control && buttons);
#else
    return 0;
#endif
}

int get_tracker_rotation(int id, float r[3])
{
#ifdef TRACKD
    if (tracker)
    {
        trackdGetEulerAngles(tracker, id, r);
        return 1;
    }
    else
#endif
    {
        r[0] = 0.0f;
        r[1] = 0.0f;
        r[2] = 0.0f;
        return 0;
    }
}

int get_tracker_position(int id, float p[3])
{
#ifdef TRACKD
    if (tracker)
    {
        trackdGetPosition(tracker, id, p);
        return 1;
    }
    else
#endif
    {
        p[0] = 0.0f;
        p[1] = 0.0f;
        p[2] = 0.0f;
        return 0;
    }
}

int get_tracker_joystick(int id, float a[2])
{
#ifdef TRACKD
    if (control)
    {
        a[0] =  trackdGetValuator(control, id + 0);
        a[1] = -trackdGetValuator(control, id + 1);
        return 1;
    }
    else
#endif
    {
        a[0] = 0.0f;
        a[1] = 0.0f;
        return 0;
    }
}

int get_tracker_buttons(int *id, int *dn)
{
#ifdef TRACKD
    if (control)
    {
        int i, n = trackdGetNumberOfButtons(control);

        for (i = 0; i < n; ++i)
            if (buttons[i] != trackdGetButton(control, i))
            {
                buttons[i]  = trackdGetButton(control, i);

                *id = i + 1;
                *dn = buttons[i];

                return 1;
            }
    }
#endif
    return 0;
}

/*---------------------------------------------------------------------------*/

#endif
