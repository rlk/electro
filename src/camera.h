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

#ifndef CAMERA_H
#define CAMERA_H

/*---------------------------------------------------------------------------*/

#define CAMERA_FAR 1000.0f

#define CAMERA_ORTHO 1
#define CAMERA_PERSP 2

struct camera
{
    int   type;
    float dist;
    float zoom;
};

/*---------------------------------------------------------------------------*/

int  camera_init(void);

int  camera_create(int);
void camera_render(int, int);
void camera_delete(int);

void camera_set_dist(int, float);
void camera_set_zoom(int, float);

/*---------------------------------------------------------------------------*/

#endif