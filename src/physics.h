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

#ifndef PHYSICS_H
#define PHYSICS_H

#include <ode/ode.h>

/*---------------------------------------------------------------------------*/

void physics_step(float);

/*---------------------------------------------------------------------------*/

dBodyID create_physics_body(void);
dGeomID create_physics_box(dBodyID, const float[6]);

void    set_physics_position(dGeomID, const float[3]);
void    get_physics_position(dGeomID,       float[3]);
void    set_physics_rotation(dGeomID, const float[16]);
void    get_physics_rotation(dGeomID,       float[16]);

/*---------------------------------------------------------------------------*/

int startup_physics();

/*---------------------------------------------------------------------------*/

#endif
