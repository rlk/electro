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

#ifndef TERRAIN_H
#define TERRAIN_H

#include "entity.h"

/*---------------------------------------------------------------------------*/

struct entity_func *startup_terrain(void);

/*---------------------------------------------------------------------------*/

int  send_create_terrain(const char *, int, int);
void recv_create_terrain(void);

void clone_terrain(int);
void delete_terrain(int);

/*---------------------------------------------------------------------------*/

#endif
