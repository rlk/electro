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

/* Body parameters */

#define BODY_ATTR_GRAVITY   1

/* Geom parameters */

#define GEOM_ATTR_CATEGORY  1
#define GEOM_ATTR_COLLIDES  2
#define GEOM_ATTR_MASS      3
#define GEOM_ATTR_BOUNCE    4
#define GEOM_ATTR_FRICTION  5
#define GEOM_ATTR_SOFT_ERP  6
#define GEOM_ATTR_SOFT_CFM  7

/* Joint parameters */

#define JOINT_ATTR_ANCHOR   (dParamGroup * 4 + 1)
#define JOINT_ATTR_AXIS_1   (dParamGroup * 4 + 2)
#define JOINT_ATTR_AXIS_2   (dParamGroup * 4 + 3)

/*---------------------------------------------------------------------------*/

void physics_step(float);

/*---------------------------------------------------------------------------*/

dBodyID set_phys_body_type(dBodyID,          int);
dGeomID set_phys_geom_type(dGeomID, dBodyID, int, const float *);
void    set_phys_join_type(dBodyID, dBodyID, int);

void    set_phys_body_attr_f(dBodyID,          int, float);
void    set_phys_geom_attr_i(dGeomID,          int, int);
void    set_phys_geom_attr_f(dGeomID,          int, float);
void    set_phys_join_attr_f(dBodyID, dBodyID, int, float);
void    set_phys_join_attr_v(dBodyID, dBodyID, int, const float *);

/*---------------------------------------------------------------------------*/

void new_phys_mass(dBodyID, float[3]);
void add_phys_mass(dBodyID, dGeomID, const float[3], const float[16]);
void mov_phys_mass(dBodyID, dGeomID, const float[3], const float[16]);
void end_phys_mass(dBodyID, float[3]);

/*---------------------------------------------------------------------------*/

void set_phys_position(dBodyID, const float[3]);
void set_phys_rotation(dBodyID, const float[16]);
void get_phys_position(dBodyID,       float[3]);
void get_phys_rotation(dBodyID,       float[16]);

/*---------------------------------------------------------------------------*/

int startup_physics();

/*---------------------------------------------------------------------------*/

#endif
