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

static void *tracker = NULL;

/*---------------------------------------------------------------------------*/

void init_tracker(void)
{
#ifdef TRACKD
    tracker = trackdInitTrackerReader(4126);
#else
    tracker = NULL;
#endif
}

void free_tracker(void)
{
}

void get_tracker_position(float *x, float *y, float *z)
{
    if (tracker)
    {
#ifdef TRACKD
        float p[3];

        trackdGetPosition(tracker, 0, p);

        *x = p[0];
        *y = p[1];
        *z = p[2];
#endif
    }
    else
    {
        *x = 0;
        *y = 0;
        *z = 0;
    }
}

/*---------------------------------------------------------------------------*/
