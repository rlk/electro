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

#define TITLE "Electro"

/*---------------------------------------------------------------------------*/

#ifdef MPI
#include <mpi.h>
#endif

int  mpi_assert(int);
int  mpi_isroot(void);

int  mpi_rank(void);
int  mpi_size(void);

void mpi_barrier(void);

int  mpi_share_byte(int, void *);
int  mpi_share_char(int, char *);
int  mpi_share_float(int, float *);
int  mpi_share_integer(int, int *);

/*---------------------------------------------------------------------------*/

#define EVENT_NULL           0
#define EVENT_DRAW           1
#define EVENT_EXIT           2

#define EVENT_ENTITY_PARENT  3
#define EVENT_ENTITY_DELETE  4
#define EVENT_ENTITY_CLONE   5
#define EVENT_ENTITY_MOVE    6
#define EVENT_ENTITY_TURN    7
#define EVENT_ENTITY_SIZE    8
#define EVENT_ENTITY_FADE    9
#define EVENT_ENTITY_FLAG   10

#define EVENT_CAMERA_CREATE 11
#define EVENT_SPRITE_CREATE 12
#define EVENT_OBJECT_CREATE 13
#define EVENT_LIGHT_CREATE  14
#define EVENT_PIVOT_CREATE  15

#define EVENT_CAMERA_DIST   16
#define EVENT_CAMERA_ZOOM   17

/*---------------------------------------------------------------------------*/

GLuint shared_load_program(const char *, GLenum);
GLuint shared_load_texture(const char *, int *, int *);

void   shared_set_camera_org(float, float, float);
void   shared_set_camera_rot(float, float, float);

void   shared_set_camera_dist(float);
void   shared_set_camera_magn(float);
void   shared_set_camera_zoom(float);

/*---------------------------------------------------------------------------*/

const char *event_string(int);

/*---------------------------------------------------------------------------*/

#endif
