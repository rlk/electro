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

void mpi_barrier_clients(void);
void mpi_barrier_all(void);
void mpi_split(void);

/*---------------------------------------------------------------------------*/

#define EVENT_NULL           0
#define EVENT_TICK           1
#define EVENT_DRAW           2
#define EVENT_EXIT           3

#define EVENT_ENTITY_PARENT  4
#define EVENT_ENTITY_DELETE  5
#define EVENT_ENTITY_CLONE   6
#define EVENT_ENTITY_MOVE    7
#define EVENT_ENTITY_TURN    8
#define EVENT_ENTITY_SIZE    9
#define EVENT_ENTITY_FADE   10
#define EVENT_ENTITY_FLAG   11

#define EVENT_CAMERA_CREATE 12
#define EVENT_SPRITE_CREATE 13
#define EVENT_OBJECT_CREATE 14
#define EVENT_GALAXY_CREATE 15
#define EVENT_LIGHT_CREATE  16
#define EVENT_PIVOT_CREATE  17
#define EVENT_IMAGE_CREATE  18

#define EVENT_GALAXY_MAGN   19
#define EVENT_CAMERA_DIST   20
#define EVENT_CAMERA_ZOOM   21
#define EVENT_SPRITE_BOUNDS 22
#define EVENT_LIGHT_COLOR   23

/*---------------------------------------------------------------------------*/

const char *event_string(int);

/*---------------------------------------------------------------------------*/

#endif
