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

#define CAMERA_NEAR     1.0f
#define CAMERA_FAR 100000.0f

#define CAMERA_ORTHO 1
#define CAMERA_PERSP 2

struct camera
{
    int   count;
    int   type;
    float dist;
    float zoom;
};

/*---------------------------------------------------------------------------*/

int  init_camera(void);
void draw_camera(int, int, const float[16], float);

int  send_create_camera(int);
void recv_create_camera(void);

void clone_camera(int);
void delete_camera(int);

/*---------------------------------------------------------------------------*/

void send_set_camera_distance(int, float);
void recv_set_camera_distance(void);

void send_set_camera_zoom(int, float);
void recv_set_camera_zoom(void);

/*---------------------------------------------------------------------------*/

void get_camera_direction(int, int, float[3], float[3]);

/*---------------------------------------------------------------------------*/

#endif
