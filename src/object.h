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

#ifndef OBJECT_H
#define OBJECT_H

#include "opengl.h"
#include "frustum.h"

/*---------------------------------------------------------------------------*/

int  init_object(void);
void draw_object(int, int, const float[16],
                           const float[16], const struct frustum *, float);

void init_object_gl(int);
void free_object_gl(int);

int  send_create_object(const char *);
void recv_create_object(void);

void clone_object(int);
void delete_object(int);

/*---------------------------------------------------------------------------*/

#endif
