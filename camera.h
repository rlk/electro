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

#ifndef CAMERA_H
#define CAMERA_H

/*---------------------------------------------------------------------------*/

#define CAMERA_FAR 100000.0f

#define CAMERA_ORTHO 1
#define CAMERA_PERSP 2

struct camera
{
    int   type;
    float dist;
    float zoom;
};

/*---------------------------------------------------------------------------*/

void  viewport_set(float, float, float, float, float, float);
int   viewport_get_x(void);
int   viewport_get_y(void);
int   viewport_get_w(void);
int   viewport_get_h(void);

/*---------------------------------------------------------------------------*/

int  camera_create(int);
void camera_render(int, const float[3], const float[3]);

void  camera_set_dist(int, float);
void  camera_set_zoom(int, float);

/*---------------------------------------------------------------------------*/

#endif
