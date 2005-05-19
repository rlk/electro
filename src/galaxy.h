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

#ifndef GALAXY_H
#define GALAXY_H

#include "entity.h"

/*---------------------------------------------------------------------------*/

struct entity_func *startup_galaxy(void);

/*---------------------------------------------------------------------------*/

int  send_create_galaxy(const char *);
void recv_create_galaxy(void);

void send_set_galaxy_magnitude(int, float);
void recv_set_galaxy_magnitude(void);

void clone_galaxy(int);
void delete_galaxy(int);

/*---------------------------------------------------------------------------*/

void get_star_position(int, int, float[3]);

int  pick_galaxy(int, const float[3], const float[3]);

/*---------------------------------------------------------------------------*/

void prep_tyc_galaxy(const char *, const char *);
void prep_hip_galaxy(const char *, const char *);

/*---------------------------------------------------------------------------*/

#endif
