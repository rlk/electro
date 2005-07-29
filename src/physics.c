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

#include <ode/ode.h>

#include "utility.h"
#include "physics.h"

/*---------------------------------------------------------------------------*/

static dWorldID      world;
static dSpaceID      space;
static dJointGroupID group;

#define MAX_CONTACTS 4

/*---------------------------------------------------------------------------*/

struct geom_data
{
    float density;
    float friction;
    float restitution;
};

static struct geom_data *create_data(void)
{
    struct geom_data *data;

    /* Allocate a geom data structure with default. */

    if ((data = (struct geom_data *) calloc(1, sizeof (struct geom_data))))
    {
        data->density     = 5.0;
        data->friction    = dInfinity;
        data->restitution = 0.5;
    }

    return data;
}

static struct geom_data *get_data(dGeomID geom)
{
    return (struct geom_data *) dGeomGetData(geom);
}

/*---------------------------------------------------------------------------*/

static void callback(void *data, dGeomID o1, dGeomID o2)
{
    dContact contact[MAX_CONTACTS];
    size_t sz = sizeof (dContact);
    int i, n;

    float friction    = MIN(get_data(o1)->friction,
                            get_data(o2)->friction);
    float restitution = MAX(get_data(o1)->restitution,
                            get_data(o2)->restitution);

    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);

    /* If the two bodies are connected, do nothing. */

    if (b1 && b2 && dAreConnectedExcluding(b1, b1, dJointTypeContact))
        return;
    
    /* Initialize the surface properties for each contact. */

    for (i=0; i<MAX_CONTACTS; ++i)
    {
        contact[i].surface.mode       = dContactBounce;
        contact[i].surface.mu         = friction;
        contact[i].surface.mu2        = 0.00;
        contact[i].surface.bounce     = restitution;
        contact[i].surface.bounce_vel = 0.10;
        contact[i].surface.soft_cfm   = 0.01;
    }

    /* Enumerate the collision points, creating a contact joint for each. */

    if ((n = dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sz)))
        for (i = 0; i < n; ++i)
        {
            dJointID c = dJointCreateContact(world, group, contact + i);
            dJointAttach(c, b1, b2);
        }
}

void physics_step(float dt)
{
    dSpaceCollide(space, 0, callback);
    dWorldQuickStep(world, dt);
    dJointGroupEmpty(group);
}

/*===========================================================================*/

static void set_rotation(dMatrix3 D, const float S[16])
{
    D[0] = S[0]; D[1] = S[4]; D[2]  = S[8];
    D[4] = S[1]; D[5] = S[5]; D[6]  = S[9];
    D[8] = S[2]; D[9] = S[6]; D[10] = S[10];
}

static void get_rotation(float D[16], const dMatrix3 S)
{
    D[0] = S[0]; D[4] = S[1]; D[8]  = S[2];  D[12] = 0;
    D[1] = S[4]; D[5] = S[5]; D[9]  = S[6];  D[13] = 0;
    D[2] = S[8]; D[6] = S[9]; D[10] = S[10]; D[14] = 0;
    D[3] =    0; D[7] =    0; D[11] =     0; D[15] = 1;   
}

/*---------------------------------------------------------------------------*/

static dGeomID set_physics_plane(dGeomID geom,
                                 float a, float b, float c, float d)
{
    dGeomPlaneSetParams(geom, a, b, c, d);
    return geom;
}

static dGeomID set_physics_box(dGeomID geom, float x, float y, float z)
{
    float d = get_data(geom)->density;
    dMass m;

    dGeomBoxSetLengths(geom, x, y, z);
    dMassSetBox(&m, d, x, y, z);
    dBodySetMass(dGeomGetBody(geom), &m);

    return geom;
}

static dGeomID set_physics_sphere(dGeomID geom, float r)
{
    float d = get_data(geom)->density;
    dMass m;

    dGeomSphereSetRadius(geom, r);
    dMassSetSphere(&m, d, r);
    dBodySetMass(dGeomGetBody(geom), &m);

    return geom;
}

static dGeomID set_physics_capsule(dGeomID geom, float r, float l)
{
    float d = get_data(geom)->density;
    dMass m;

    dGeomCCylinderSetParams(geom, r, l);
    dMassSetCappedCylinder(&m, d, 3, r, l);
    dBodySetMass(dGeomGetBody(geom), &m);

    return geom;
}

/*---------------------------------------------------------------------------*/

static dGeomID create_physics_plane(void)
{
    dGeomID geom = dCreatePlane(space, 0, 1, 0, 0);
    void   *data = create_data();

    dGeomSetData(geom, data);

    return set_physics_plane(geom, 0, 1, 0, 0);
}

static dGeomID create_physics_box(void)
{
    dBodyID body = dBodyCreate(world);
    dGeomID geom = dCreateBox(space, 1, 1, 1);
    void   *data = create_data();

    dGeomSetBody(geom, body);
    dGeomSetData(geom, data);

    return set_physics_box(geom, 1, 1, 1);
}

static dGeomID create_physics_sphere(void)
{
    dBodyID body = dBodyCreate(world);
    dGeomID geom = dCreateSphere(space, 1);
    void   *data = create_data();

    dGeomSetBody(geom, body);
    dGeomSetData(geom, data);

    return set_physics_sphere(geom, 1);
}

static dGeomID create_physics_capsule(void)
{
    dBodyID body = dBodyCreate(world);
    dGeomID geom = dCreateCCylinder(space, 1, 1);
    void   *data = create_data();

    dGeomSetBody(geom, body);
    dGeomSetData(geom, data);

    return set_physics_capsule(geom, 1, 1);
}

/*---------------------------------------------------------------------------*/

dGeomID set_physics_solid(dGeomID geom, int o, int f,
                          float a, float b, float c, float d)
{
    dBodyID body = geom ? dGeomGetBody(geom) : 0;

    switch (o)
    {
    case SOLID_TYPE:
        if (geom) free(dGeomGetData(geom));

        if (body) dBodyDestroy(body);
        if (geom) dGeomDestroy(geom);

        switch (f)
        {
        case SOLID_TYPE_BOX:     return create_physics_box();
        case SOLID_TYPE_PLANE:   return create_physics_plane();
        case SOLID_TYPE_SPHERE:  return create_physics_sphere();
        case SOLID_TYPE_CAPSULE: return create_physics_capsule();
        }
        return 0;

    case SOLID_BOX_PARAM:
        return set_physics_box(geom, a, b, c);
    case SOLID_PLANE_PARAM:
        return set_physics_plane(geom, a, b, c, d);
    case SOLID_SPHERE_PARAM:
        return set_physics_sphere(geom, a);
    case SOLID_CAPSULE_PARAM:
        return set_physics_capsule(geom, a, b);

    case SOLID_CATEGORY_BITS:
        dGeomSetCategoryBits(geom, f);
        break;
    case SOLID_COLLIDER_BITS:
        dGeomSetCollideBits(geom, f);
        break;

    case SOLID_DENSITY:
        get_data(geom)->density = a;
        break;
    case SOLID_FRICTION:
        get_data(geom)->friction = a;
        break;
    case SOLID_RESTITUTION:
        get_data(geom)->restitution = a;
        break;
    }

    return geom;
}

void set_physics_joint(dGeomID geom1, dGeomID geom2, int o,
                       int f, float a, float b, float c)
{
}

/*---------------------------------------------------------------------------*/

void set_physics_position(dGeomID geom, const float p[3])
{
    if (dGeomGetClass(geom) != dPlaneClass)
        dGeomSetPosition(geom, p[0], p[1], p[2]);
}

int get_physics_position(dGeomID geom, float p[3])
{
    if (dGeomGetClass(geom) != dPlaneClass)
    {
        p[0] = dGeomGetPosition(geom)[0];
        p[1] = dGeomGetPosition(geom)[1];
        p[2] = dGeomGetPosition(geom)[2];
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void set_physics_rotation(dGeomID geom, const float r[16])
{
    dMatrix3 R;

    if (dGeomGetClass(geom) != dPlaneClass)
    {
        set_rotation(R, r);
        dGeomSetRotation(geom, R);
    }
}

int get_physics_rotation(dGeomID geom, float r[16])
{
    if (dGeomGetClass(geom) != dPlaneClass)
    {
        get_rotation(r, dGeomGetRotation(geom));
        return 1;
    }
    return 0;
}

/*===========================================================================*/

int startup_physics(void)
{
    world = dWorldCreate();
    space = dHashSpaceCreate(0);
    group = dJointGroupCreate(0);

    dWorldSetGravity(world, 0, -9.8, 0);
    dWorldSetAutoDisableFlag(world, 1);

    return 1;
}
