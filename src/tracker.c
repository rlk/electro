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

#include "tracker.h"
#include "utility.h"

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
