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

void status_init(void);

void status_draw_camera(void);

/*---------------------------------------------------------------------------*/

void  status_set_viewport(float, float, float, float, float, float);

void  status_set_camera_pos(float, float, float);
void  status_set_camera_rot(float, float, float);

void  status_set_camera_dist(float);
void  status_set_camera_magn(float);
void  status_set_camera_zoom(float);

/*---------------------------------------------------------------------------*/

int   status_get_viewport_w(void);
int   status_get_viewport_h(void);

void  status_get_camera_pos(float *, float *, float *);
void  status_get_camera_rot(float *, float *, float *);

float status_get_camera_dist(void);
float status_get_camera_magn(void);
float status_get_camera_zoom(void);

/*---------------------------------------------------------------------------*/

#endif
