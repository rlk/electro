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

#ifndef SHARED_H
#define SHARED_H

#include "opengl.h"

/*---------------------------------------------------------------------------*/

#define TITLE "Total Perspective Vortex"

/*---------------------------------------------------------------------------*/

void mpi_error(int);
int  mpi_root(void);

/*---------------------------------------------------------------------------*/

#define EVENT_DRAW 1
#define EVENT_EXIT 2

#define EVENT_CAMERA_MOVE 3
#define EVENT_CAMERA_TURN 4
#define EVENT_CAMERA_ZOOM 5
#define EVENT_CAMERA_DIST 6
#define EVENT_CAMERA_MAGN 7

#define EVENT_SPRITE_LOAD 8
#define EVENT_SPRITE_FREE 9
#define EVENT_SPRITE_MOVE 10
#define EVENT_SPRITE_TURN 11
#define EVENT_SPRITE_SIZE 12
#define EVENT_SPRITE_FADE 13

/*---------------------------------------------------------------------------*/

#define NAMELEN 32

struct viewport
{
    char name[NAMELEN];      /* Node host name                               */
    float X;                 /* Window X position, relative to desktop       */
    float Y;                 /* Window Y position, relative to desktop       */
    float x;                 /* Tile X position, relative to center of array */
    float y;                 /* Tile Y position, relative to center of array */
    float w;                 /* Tile width                                   */
    float h;                 /* Tile height                                  */
};

void viewport_init(int);
void viewport_tile(const char *, float, float, float, float, float, float);
void viewport_sync(int);

/*---------------------------------------------------------------------------*/

GLuint shared_load_program(const char *, GLenum);
GLuint shared_load_texture(const char *, int *, int *);

void   shared_set_camera_org(float, float, float);
void   shared_set_camera_rot(float, float, float);

void   shared_set_camera_dist(float);
void   shared_set_camera_magn(float);
void   shared_set_camera_zoom(float);

/*---------------------------------------------------------------------------*/

#endif