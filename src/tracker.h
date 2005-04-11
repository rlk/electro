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

#ifndef TRACKER_H
#define TRACKER_H

/*---------------------------------------------------------------------------*/

#define TRACKER_KEY    4126
#define CONTROLLER_KEY 4127

/*---------------------------------------------------------------------------*/

void init_tracker(void);

int  get_tracker_rotation(int, float[3]);
int  get_tracker_position(int, float[3]);
int  get_tracker_joystick(int, float[2]);

/*---------------------------------------------------------------------------*/

#endif
