/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    TOTAL PERSPECTIVE VORTEX is free software;  you can redistribute it    */
/*    and/or modify it under the terms of the  GNU General Public License    */
/*    as published by the  Free Software Foundation;  either version 2 of    */
/*    the License, or (at your option) any later version.                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#ifndef VIEWER_H
#define VIEWER_H

/*---------------------------------------------------------------------------*/

void viewer_init(void);
void viewer_draw(void);
void viewer_bill(void);

int viewer_point(int, int);
int viewer_click(int, int);
int viewer_keybd(int, int);
int viewer_event(int);

void viewer_get_pos(double[3]);
void viewer_get_vec(double[3]);
void viewer_get_mag(double[1]);

int viewer_depth(void);

/*---------------------------------------------------------------------------*/

#endif
