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
#include "script.h"

/*---------------------------------------------------------------------------*/

static dWorldID      world;
static dSpaceID      space;
static dJointGroupID group;

#define MAX_CONTACTS 4

/*---------------------------------------------------------------------------*/

struct geom_data
{
    int           entity;
    unsigned long response;
    unsigned long callback;

    dReal mass;
    dReal bounce;
    dReal friction;
    dReal soft_erp;
    dReal soft_cfm;
};

static struct geom_data *create_data(int entity)
{
    struct geom_data *data;

    /* Allocate a geom data structure with default values. */

    if ((data = (struct geom_data *) calloc(1, sizeof (struct geom_data))))
    {
        data->entity   = entity;
        data->callback = 0;
        data->mass     = 1.00;
        data->bounce   = 0.50;
        data->friction = dInfinity;
        data->soft_erp = 0.20;
        data->soft_cfm = 0.01;
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
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);

    if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact))
        return;

    /* Two geoms associated with the same body do not collide. */

    if (b1 != b2)
    {
        dContact contact[MAX_CONTACTS];
        size_t sz = sizeof (dContact);
        int i, n;

        /* Find and enumerate all collision points. */

        if ((n = dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sz)))
        {
            struct geom_data *d1 = get_data(o1);
            struct geom_data *d2 = get_data(o2);

            /* Compute collision parameters from geom parameters. */

            float bounce    = MAX(d1->bounce,   d2->bounce);
            float friction  = MIN(d1->friction, d2->friction);
            float soft_erp  = MIN(d1->soft_erp, d2->soft_erp);
            float soft_cfm  = MAX(d1->soft_cfm, d2->soft_cfm);

            unsigned long category1 = dGeomGetCategoryBits(o1);
            unsigned long category2 = dGeomGetCategoryBits(o2);

            /* Create a contact joint for each collision, if requsted. */

            if ((category1 & d2->response) ||
                (category2 & d1->response))
                for (i = 0; i < n; ++i)
                {
                    dJointID c;
                
                    contact[i].surface.mode = dContactBounce
                                            | dContactSoftCFM
                                            | dContactSoftERP;

                    contact[i].surface.mu         = friction;
                    contact[i].surface.bounce     = bounce;
                    contact[i].surface.soft_cfm   = soft_cfm;
                    contact[i].surface.soft_erp   = soft_erp;
                    contact[i].surface.bounce_vel = 0.10;

                    c = dJointCreateContact(world, group, contact + i);
                    dJointAttach(c, b1, b2);
                }

            /* Report contacts to the Lua script, if requested. */

            if ((category1 & d2->callback) ||
                (category2 & d1->callback))
                for (i = 0; i < n; ++i)
                {
                    float p[3];
                    float n[3];

                    p[0] = (float) contact[i].geom.pos[0];
                    p[1] = (float) contact[i].geom.pos[1];
                    p[2] = (float) contact[i].geom.pos[2];

                    n[0] = (float) contact[i].geom.normal[0];
                    n[1] = (float) contact[i].geom.normal[1];
                    n[2] = (float) contact[i].geom.normal[2];

                    do_contact_script(d1->entity, d2->entity,
                                      p, n, contact[i].geom.depth);
                }
        }
    }
}

void physics_step(float dt)
{
    dSpaceCollide(space, 0, callback);
    dWorldQuickStep(world, dt);
    dJointGroupEmpty(group);
}

/*===========================================================================*/
/* OpenGL <-> ODE rotation matrix conversions                                */

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
/* Body mass accumulation functions                                          */

void new_phys_mass(dBodyID body, float v[3])
{
    dMass mass;

    /* Zero the body's mass in preparation for mass accumulation. */

    dMassSetZero(&mass);
    dBodySetMass(body, &mass);

    v[0] = v[1] = v[2] = 0;
}

void add_phys_mass(dBodyID body, dGeomID geom, const float p[3],
                                               const float r[16])
{
    dVector3  v;
    dMatrix3  M;
    dReal   rad;
    dReal   len;
    dMass mass1;
    dMass mass2;

    dGeomID object = dGeomTransformGetGeom(geom);

    /* Create a new mass for the given geom. */

    switch (dGeomGetClass(object))
    {
    case dBoxClass:
        dGeomBoxGetLengths(object, v);
        dMassSetBoxTotal(&mass2, get_data(geom)->mass, v[0], v[1], v[2]);
        break;

    case dSphereClass:
        rad = dGeomSphereGetRadius(object);
        dMassSetSphereTotal(&mass2, get_data(geom)->mass, rad);
        break;

    case dCCylinderClass:
        dGeomCCylinderGetParams(object, &rad, &len);
        dMassSetCappedCylinderTotal(&mass2, get_data(geom)->mass, 3, rad, len);
        break;

    default:
        dMassSetZero(&mass2);
        break;
    }

    /* Transform the geom mass to the given position and rotation. */

    if (p)
        dMassTranslate(&mass2, p[0], p[1], p[2]);
    if (r)
    {
        set_rotation(M, r);
        dMassRotate(&mass2, M);
    }

    /* Accumulate the new mass with the body's existing mass. */

    dBodyGetMass(body, &mass1);
    dMassAdd(&mass1, &mass2);
    dBodySetMass(body, &mass1);
}

void mov_phys_mass(dBodyID body, dGeomID geom, const float p[3],
                                               const float r[16])
{
    dMatrix3 M;
    dMass mass;

    dGeomID object = dGeomTransformGetGeom(geom);

    /* Move the geom so that the body's center of mass is at the origin. */

    if (body)
    {
        dBodyGetMass(body, &mass);
        dGeomSetPosition(object, p[0] - mass.c[0],
                                 p[1] - mass.c[1],
                                 p[2] - mass.c[2]);
    }
    else
        dGeomSetPosition(object, p[0], p[1], p[2]);

    /* Apply the geom's rotation. */

    set_rotation(M, r);
    dGeomSetRotation(object, M);
}

void end_phys_mass(dBodyID body, float v[3])
{
    dMass mass;

    /* Translate the center of mass to the origin. */

    dBodyGetMass(body, &mass);

    v[0] = mass.c[0];
    v[1] = mass.c[1];
    v[2] = mass.c[2];

    dMassTranslate(&mass, -mass.c[0], -mass.c[1], -mass.c[2]);
    dBodySetMass(body, &mass);
}

/*---------------------------------------------------------------------------*/
/* Joint operation type switchers                                            */

static dJointID create_phys_joint(int t)
{
    switch (t)
    {
    case dJointTypeBall:      return dJointCreateBall     (world, 0);
    case dJointTypeHinge:     return dJointCreateHinge    (world, 0);
    case dJointTypeSlider:    return dJointCreateSlider   (world, 0);
    case dJointTypeUniversal: return dJointCreateUniversal(world, 0);
    case dJointTypeHinge2:    return dJointCreateHinge2   (world, 0);
    }
    return 0;
}

static void set_phys_joint_anchor(dJointID joint, const float v[3])
{
    switch (dJointGetType(joint))
    {
    case dJointTypeBall:
        dJointSetBallAnchor     (joint, v[0], v[1], v[2]); break;
    case dJointTypeHinge:
        dJointSetHingeAnchor    (joint, v[0], v[1], v[2]); break;
    case dJointTypeHinge2:
        dJointSetHinge2Anchor   (joint, v[0], v[1], v[2]); break;
    case dJointTypeUniversal:
        dJointSetUniversalAnchor(joint, v[0], v[1], v[2]); break;
    }
}

static void set_phys_joint_axis_1(dJointID joint, const float v[3])
{
    switch (dJointGetType(joint))
    {
    case dJointTypeHinge:
        dJointSetHingeAxis     (joint, v[0], v[1], v[2]); break;
    case dJointTypeSlider:
        dJointSetSliderAxis    (joint, v[0], v[1], v[2]); break;
    case dJointTypeHinge2:
        dJointSetHinge2Axis1   (joint, v[0], v[1], v[2]); break;
    case dJointTypeUniversal:
        dJointSetUniversalAxis1(joint, v[0], v[1], v[2]); break;
    }
}

static void set_phys_joint_axis_2(dJointID joint, const float v[3])
{
    switch (dJointGetType(joint))
    {
    case dJointTypeHinge2:
        dJointSetHinge2Axis2   (joint, v[0], v[1], v[2]); break;
    case dJointTypeUniversal:
        dJointSetUniversalAxis2(joint, v[0], v[1], v[2]); break;
    }
}

static void set_phys_joint_attr(dJointID joint, int p, float v)
{
    switch (dJointGetType(joint))
    {
    case dJointTypeHinge:     dJointSetHingeParam    (joint, p, v); break;
    case dJointTypeSlider:    dJointSetSliderParam   (joint, p, v); break;
    case dJointTypeHinge2:    dJointSetHinge2Param   (joint, p, v); break;
    case dJointTypeUniversal: dJointSetUniversalParam(joint, p, v); break;
    }
}

/*---------------------------------------------------------------------------*/
/* Body functions                                                            */

dBodyID set_phys_body_type(dBodyID body, int b)
{
    if (body)
        dBodyDestroy(body);

    if (b)
        return dBodyCreate(world);

    return 0;
}

void set_phys_body_attr_f(dBodyID body, int p, float f)
{
    switch (p)
    {
    case BODY_ATTR_GRAVITY:
        dBodySetGravityMode(body, f);
        break;
    }
}

/*---------------------------------------------------------------------------*/
/* Geom functions                                                            */

dGeomID set_phys_geom_type(dGeomID geom, dBodyID body,
                           int i, int t, const float *v)
{
    dGeomID transform = 0;
    dGeomID object    = 0;

    /* Destroy the old geom, its transform, and its data. */

    if (geom)
    {
        free(dGeomGetData(geom));
        dGeomDestroy(geom);
    }

    /* Create a new geom of the required type. */

    switch (t)
    {
    case dSphereClass:
        transform = dCreateGeomTransform(space);
        object    = dCreateSphere(0, v[0]);
        break;
    case dCCylinderClass:
        transform = dCreateGeomTransform(space);
        object    = dCreateCCylinder(0, v[0], v[1]);
        break;
    case dBoxClass:
        transform = dCreateGeomTransform(space);
        object    = dCreateBox(0, v[0], v[1], v[2]);
        break;
    case dPlaneClass:
        object    = dCreatePlane(space, v[0], v[1], v[2], v[3]);
        break;
    }

    /* Assign geom data and encapsulate the new geom in a transform. */

    if (object)
        dGeomSetData(object, create_data(i));

    if (transform && object)
    {
        dGeomTransformSetCleanup(transform, 1);
        dGeomTransformSetGeom(transform, object);
        dGeomSetData(transform, dGeomGetData(object));
        dGeomSetBody(transform, body);
    }

    return transform ? transform : object;
}

void set_phys_geom_attr_i(dGeomID geom, int p, int i)
{
    switch (p)
    {
    case GEOM_ATTR_CATEGORY:
        dGeomSetCategoryBits(geom,   i);
        break;
    case GEOM_ATTR_COLLIDER:
        dGeomSetCollideBits(geom,   i);
        break;
    case GEOM_ATTR_RESPONSE:
        get_data(geom)->response = i;
        break;
    case GEOM_ATTR_CALLBACK:
        get_data(geom)->callback = i;
        break;
    }
}

void set_phys_geom_attr_f(dGeomID geom, int p, float f)
{
    dGeomID object = dGeomTransformGetGeom(geom);

    switch (p)
    {
    case GEOM_ATTR_MASS:
        get_data(object)->mass = f;
        break;
    case GEOM_ATTR_BOUNCE:
        get_data(object)->bounce = f;
        break;
    case GEOM_ATTR_FRICTION:
        get_data(object)->friction = f;
        break;
    case GEOM_ATTR_SOFT_ERP:
        get_data(object)->soft_erp = f;
        break;
    case GEOM_ATTR_SOFT_CFM:
        get_data(object)->soft_cfm = f;
        break;
    }
}

/*---------------------------------------------------------------------------*/
/* Joint functions                                                           */

dJointID find_shared_joint(dBodyID body1, dBodyID body2)
{
    int i1, n1 = dBodyGetNumJoints(body1);
    int i2, n2 = dBodyGetNumJoints(body2);

    for (i1 = 0; i1 < n1; ++i1)
        for (i2 = 0; i2 < n2; ++i2)
            if (dBodyGetJoint(body1, i1) ==
                dBodyGetJoint(body2, i2)) return dBodyGetJoint(body1, i1);

    return 0;
}

void set_phys_join_type(dBodyID body1, dBodyID body2, int t)
{
    dJointID joint = find_shared_joint(body1, body2);

    if (joint)
        dJointDestroy(joint);

    if ((joint = create_phys_joint(t)))
        dJointAttach(joint, body1, body2);
}

void set_phys_join_attr_f(dBodyID body1, dBodyID body2, int p, float f)
{
    dJointID joint = find_shared_joint(body1, body2);

    if (joint)
        set_phys_joint_attr(joint, p, f);
}

void set_phys_join_attr_v(dBodyID body1, dBodyID body2, int p, const float *v)
{
    dJointID joint = find_shared_joint(body1, body2);

    if (joint)
        switch (p)
        {
        case JOINT_ATTR_ANCHOR: set_phys_joint_anchor(joint, v); break;
        case JOINT_ATTR_AXIS_1: set_phys_joint_axis_1(joint, v); break;
        case JOINT_ATTR_AXIS_2: set_phys_joint_axis_2(joint, v); break;
        }
}

/*---------------------------------------------------------------------------*/

void add_phys_force(dBodyID body, float x, float y, float z)
{
    dBodyEnable(body);
    dBodyAddForce(body, x, y, z);
}

void add_phys_torque(dBodyID body, float x, float y, float z)
{
    dBodyEnable(body);
    dBodyAddTorque(body, x, y, z);
}

/*---------------------------------------------------------------------------*/
/* Position and rotation accessors                                           */

void set_phys_position(dBodyID body, const float p[3])
{
    dBodySetPosition(body, p[0], p[1], p[2]);
}

void get_phys_position(dBodyID body, float p[3])
{
    p[0] = dBodyGetPosition(body)[0];
    p[1] = dBodyGetPosition(body)[1];
    p[2] = dBodyGetPosition(body)[2];
}

void set_phys_rotation(dBodyID body, const float r[16])
{
    dMatrix3 R;

    set_rotation(R, r);
    dBodySetRotation(body, R);
}

void get_phys_rotation(dBodyID body, float r[16])
{
    get_rotation(r, dBodyGetRotation(body));
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
