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

#ifndef SHARED_H
#define SHARED_H

#include "opengl.h"

/*---------------------------------------------------------------------------*/

#define TITLE "Total Perspective Vortex"

/*---------------------------------------------------------------------------*/

int mpi_assert(int);
int mpi_isroot(void);

int mpi_share_float(int, float *);
int mpi_share_integer(int, int *);

/*---------------------------------------------------------------------------*/

#define EVENT_DRAW 1
#define EVENT_EXIT 2

#define EVENT_ENTITY_CREATE 3
#define EVENT_ENTITY_PARENT 4
#define EVENT_ENTITY_DELETE 5
#define EVENT_ENTITY_MOVE   6
#define EVENT_ENTITY_TURN   7
#define EVENT_ENTITY_SIZE   8

#define EVENT_SPRITE_CREATE 9
#define EVENT_CAMERA_CREATE 10

#define EVENT_CAMERA_DIST   11
#define EVENT_CAMERA_ZOOM   12


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
