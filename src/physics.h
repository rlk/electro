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

/* Solid parameters. */

#define SOLID_TYPE            1  /*  i */
#define SOLID_BOX_PARAM       2  /* 3f */
#define SOLID_PLANE_PARAM     3  /* 4f */
#define SOLID_SPHERE_PARAM    4  /*  f */
#define SOLID_CAPSULE_PARAM   5  /* 2f */
#define SOLID_CATEGORY_BITS   6  /*  i */
#define SOLID_COLLIDER_BITS   7  /*  i */
#define SOLID_DENSITY         8  /*  f */
#define SOLID_FRICTION        9  /*  f */
#define SOLID_RESTITUTION    10  /*  f */

/* Solid types. */

#define SOLID_TYPE_NONE       0
#define SOLID_TYPE_BOX        1
#define SOLID_TYPE_PLANE      2
#define SOLID_TYPE_SPHERE     3
#define SOLID_TYPE_CAPSULE    4

/*---------------------------------------------------------------------------*/

/* Joint parameters. */

#define JOINT_TYPE            1  /*  i */
#define JOINT_ANCHOR          2  /* 3f */
#define JOINT_AXIS_1          3  /* 3f */
#define JOINT_AXIS_2          4  /* 3f */
#define JOINT_MIN_VALUE       5  /*  f */
#define JOINT_MAX_VALUE       6  /*  f */
#define JOINT_VELOCITY        7  /*  f */
#define JOINT_FORCE           8  /*  f */
#define JOINT_RESTITUTION     9  /*  f */

/* Joint types. */

#define JOINT_TYPE_NONE       0
#define JOINT_TYPE_BALL       1
#define JOINT_TYPE_HINGE      2
#define JOINT_TYPE_SLIDER     3
#define JOINT_TYPE_UNIVERSAL  4
#define JOINT_TYPE_HINGE_2    5

/*---------------------------------------------------------------------------*/

void physics_step(float);

/*---------------------------------------------------------------------------*/

dGeomID set_physics_solid(dGeomID, int, int, float, float, float, float);
void    set_physics_joint(dGeomID, dGeomID, int, int, float, float, float);

void    set_physics_position(dGeomID, const float[3]);
void    get_physics_position(dGeomID,       float[3]);
void    set_physics_rotation(dGeomID, const float[16]);
void    get_physics_rotation(dGeomID,       float[16]);

/*---------------------------------------------------------------------------*/

int startup_physics();

/*---------------------------------------------------------------------------*/

#endif
