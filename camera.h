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

#ifndef STATUS_H
#define STATUS_H

/*---------------------------------------------------------------------------*/

void camera_init(void);
void camera_draw(void);

/*---------------------------------------------------------------------------*/

void  camera_set_viewport(float, float, float, float, float, float);

int   camera_get_viewport_w(void);
int   camera_get_viewport_h(void);

/*---------------------------------------------------------------------------*/

void  camera_set_org(float, float, float);
void  camera_set_rot(float, float, float);

void  camera_set_dist(float);
void  camera_set_magn(float);
void  camera_set_zoom(float);

void  camera_set_pos(void);

/*---------------------------------------------------------------------------*/

#endif
