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

#include "tracker.h"

/*---------------------------------------------------------------------------*/

#define MAXJOY 8

static SDL_Joystick *joy[MAXJOY];

/*---------------------------------------------------------------------------*/

int startup_joystick(void)
{
    int i, n = SDL_NumJoysticks();

    for (i = 0; i < n && MAXJOY; ++i)
        joy[i] = SDL_JoystickOpen(i);

    return 1;
}

/*---------------------------------------------------------------------------*/

void get_joystick(int i, float a[2])
{
    if (get_tracker_joystick(i, a))
        return;

    if (i < SDL_NumJoysticks() && SDL_JoystickOpened(i))
    {
        a[0] = SDL_JoystickGetAxis(joy[i], 0) / 32768.0f;
        a[1] = SDL_JoystickGetAxis(joy[i], 1) / 32768.0f;
    }
    else
    {
        a[0] = 0;
        a[1] = 0;
    }
}

/*---------------------------------------------------------------------------*/
