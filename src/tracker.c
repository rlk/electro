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

#ifdef TRACKD
#include <trackdAPI.h>
#endif

/*---------------------------------------------------------------------------*/

static void *tracker    = NULL;
static void *controller = NULL;

/*---------------------------------------------------------------------------*/

void init_tracker(void)
{
#ifdef TRACKD
    tracker    = trackdInitTrackerReader(TRACKER_KEY);
    controller = trackdInitControllerReader(CONTROLLER_KEY);
#else
    tracker    = NULL;
    controller = NULL;
#endif
}

/*---------------------------------------------------------------------------*/

int get_tracker_rotation(int id, float e[3][3])
{
#ifdef TRACKD
    if (tracker)
    {
        float M[4][4];

        trackdGetMatrix(tracker, id, M);

        e[0][0] = M[0][0];
        e[0][1] = M[0][1];
        e[0][2] = M[0][2];

        e[1][0] = M[1][0];
        e[1][1] = M[1][1];
        e[1][2] = M[1][2];

        e[2][0] = M[2][0];
        e[2][1] = M[2][1];
        e[2][2] = M[2][2];

        return 1;
    }
#endif

    return 0;
}

int get_tracker_position(int id, float p[3])
{
#ifdef TRACKD
    if (tracker)
    {
        trackdGetPosition(tracker, id, p);
        return 1;
    }
#endif

    return 0;
}

int get_tracker_joystick(int id, float a[2])
{
#ifdef TRACKD
    if (controller)
    {
        a[0] = trackdGetValuator(controller, id + 0);
        a[1] = trackdGetValuator(controller, id + 1);

        return 1;
    }
#endif

    return 0;
}

/*---------------------------------------------------------------------------*/
