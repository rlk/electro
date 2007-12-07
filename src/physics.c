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
#include "matrix.h"
#include "script.h"
#include "opengl.h"

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
        data->callback = 0x00000000;
        data->response = 0xFFFFFFFF;
        data->mass     = (dReal) 1.00;
        data->bounce   = (dReal) 0.50;
        data->friction = dInfinity;
        data->soft_erp = (dReal) 0.20;
        data->soft_cfm = (dReal) 0.01;
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

    if (b1 == 0 || b2 == 0 || b1 != b2)
    {
        dContact contact[MAX_CONTACTS];
        size_t sz = sizeof (dContact);
        int i, n;

        /* Find and enumerate all collision points. */

        if ((n = dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sz)))
        {
            struct geom_data *d1 = get_data(o1);
            struct geom_data *d2 = get_data(o2);

            if (b1) dBodyEnable(b1);
            if (b2) dBodyEnable(b2);

            /* Compute collision parameters from geom parameters. */

            dReal bounce    = MAX(d1->bounce,   d2->bounce);
            dReal friction  = MIN(d1->friction, d2->friction);
            dReal soft_erp  = MIN(d1->soft_erp, d2->soft_erp);
            dReal soft_cfm  = MAX(d1->soft_cfm, d2->soft_cfm);

            unsigned long category1 = dGeomGetCategoryBits(o1);
            unsigned long category2 = dGeomGetCategoryBits(o2);

            /* Create a contact joint for each collision, if requsted. */

            if ((category1 & d2->response) ||
                (category2 & d1->response))
                for (i = 0; i < n; ++i)
                    if (dGeomGetClass(contact[0].geom.g1) != dRayClass &&
                        dGeomGetClass(contact[0].geom.g2) != dRayClass)
                    {
                        dJointID c;
                
                        contact[i].surface.mode = dContactBounce
                                                | dContactSoftCFM
                                                | dContactSoftERP;

                        contact[i].surface.mu         = friction;
                        contact[i].surface.bounce     = bounce;
                        contact[i].surface.soft_cfm   = soft_cfm;
                        contact[i].surface.soft_erp   = soft_erp;
                        contact[i].surface.bounce_vel = (dReal) 0.10;

                        c = dJointCreateContact(world, group, contact + i);
                        dJointAttach(c, b1, b2);
                    }

            /* Report contacts to the Lua script, if requested. */

            if ((category1 & d2->callback) ||
                (category2 & d1->callback))
                for (i = 0; i < n; ++i)
                {
                    float p[3];
                    float v[3];

                    p[0] = (float) contact[i].geom.pos[0];
                    p[1] = (float) contact[i].geom.pos[1];
                    p[2] = (float) contact[i].geom.pos[2];

                    v[0] = (float) contact[i].geom.normal[0];
                    v[1] = (float) contact[i].geom.normal[1];
                    v[2] = (float) contact[i].geom.normal[2];

                    do_contact_script(d1->entity, d2->entity,
                                      p, v, (float) contact[i].geom.depth);
                }
        }
    }
}

int physics_step(float dt)
{
    dSpaceCollide(space, 0, callback);
    dWorldQuickStep(world, dt);
    dJointGroupEmpty(group);

    return 1;
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
    D[0]  = (float) S[0]; 
    D[1]  = (float) S[4]; 
    D[2]  = (float) S[8];
    D[3]  = (float)    0; 
    D[4]  = (float) S[1]; 
    D[5]  = (float) S[5]; 
    D[6]  = (float) S[9]; 
    D[7]  = (float)    0; 
    D[8]  = (float) S[2];  
    D[9]  = (float) S[6];  
    D[10] = (float) S[10]; 
    D[11] = (float)    0; 
    D[12] = (float)    0;
    D[13] = (float)    0;
    D[14] = (float)    0;
    D[15] = (float)    1;
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

    if (dGeomGetClass(geom) != dPlaneClass)
    {
        dGeomID object = dGeomTransformGetGeom(geom);
        dReal   m      = get_data(geom)->mass;

        /* Create a new mass for the given geom. */

        switch (dGeomGetClass(object))
        {
        case dBoxClass:
            dGeomBoxGetLengths(object, v);
            dMassSetBoxTotal(&mass2, m, v[0], v[1], v[2]);
            break;

        case dSphereClass:
            rad = dGeomSphereGetRadius(object);
            dMassSetSphereTotal(&mass2, m, rad);
            break;

        case dCCylinderClass:
            dGeomCCylinderGetParams(object, &rad, &len);
            dMassSetCappedCylinderTotal(&mass2, m, 3, rad, len);
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
}

void mov_phys_mass(dBodyID body, dGeomID geom, const float p[3],
                                               const float r[16])
{
    dMatrix3 M;
    dMass mass;

    set_rotation(M, r);

    /* If the geom has a body, assume it is a geom transform. */

    if (body)
    {
        dGeomID object = dGeomTransformGetGeom(geom);

        /* Move the geom so that the body's center of mass is at the origin. */

        dBodyGetMass(body, &mass);
        dGeomSetRotation(object, M);
        dGeomSetPosition(object, p[0] - mass.c[0],
                                 p[1] - mass.c[1],
                                 p[2] - mass.c[2]);
    }
    else
    {
        /* If the geom has no body, make sure we don't try to move a plane. */

        if (dGeomGetClass(geom) != dPlaneClass)
        {
            dGeomSetRotation(geom, M);
            dGeomSetPosition(geom, p[0], p[1], p[2]);
        }
    }
}

void end_phys_mass(dBodyID body, float v[3])
{
    dMass mass;

    /* Translate the center of mass to the origin. */

    dBodyGetMass(body, &mass);

    v[0] = (float) mass.c[0];
    v[1] = (float) mass.c[1];
    v[2] = (float) mass.c[2];

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

static void set_phys_joint_anchor(dJointID j, const float v[3])
{
    switch (dJointGetType(j))
    {
    case dJointTypeBall:
        dJointSetBallAnchor     (j, v[0], v[1], v[2]); break;
    case dJointTypeHinge:
        dJointSetHingeAnchor    (j, v[0], v[1], v[2]); break;
    case dJointTypeHinge2:
        dJointSetHinge2Anchor   (j, v[0], v[1], v[2]); break;
    case dJointTypeUniversal:
        dJointSetUniversalAnchor(j, v[0], v[1], v[2]); break;
    }
}

static void set_phys_joint_axis_1(dJointID j, const float v[3])
{
    switch (dJointGetType(j))
    {
    case dJointTypeHinge:
        dJointSetHingeAxis     (j, v[0], v[1], v[2]); break;
    case dJointTypeSlider:
        dJointSetSliderAxis    (j, v[0], v[1], v[2]); break;
    case dJointTypeHinge2:
        dJointSetHinge2Axis1   (j, v[0], v[1], v[2]); break;
    case dJointTypeUniversal:
        dJointSetUniversalAxis1(j, v[0], v[1], v[2]); break;
    }
}

static void set_phys_joint_axis_2(dJointID j, const float v[3])
{
    switch (dJointGetType(j))
    {
    case dJointTypeHinge2:
        dJointSetHinge2Axis2   (j, v[0], v[1], v[2]); break;
    case dJointTypeUniversal:
        dJointSetUniversalAxis2(j, v[0], v[1], v[2]); break;
    }
}

static void set_phys_joint_attr(dJointID j, int p, float v)
{
    switch (dJointGetType(j))
    {
    case dJointTypeHinge:     dJointSetHingeParam    (j, p, v); break;
    case dJointTypeSlider:    dJointSetSliderParam   (j, p, v); break;
    case dJointTypeHinge2:    dJointSetHinge2Param   (j, p, v); break;
    case dJointTypeUniversal: dJointSetUniversalParam(j, p, v); break;
    }
}

static float get_phys_joint_attr(dJointID j, int p)
{
    switch (dJointGetType(j))
    {
    case dJointTypeHinge:     return (float) dJointGetHingeParam    (j, p);
    case dJointTypeSlider:    return (float) dJointGetSliderParam   (j, p);
    case dJointTypeHinge2:    return (float) dJointGetHinge2Param   (j, p);
    case dJointTypeUniversal: return (float) dJointGetUniversalParam(j, p);
    }
    return 0;
}

static float get_phys_joint_value(dJointID j)
{
    switch (dJointGetType(j))
    {
    case dJointTypeHinge:  return (float) DEG(dJointGetHingeAngle    (j));
    case dJointTypeSlider: return (float)     dJointGetSliderPosition(j);
    case dJointTypeHinge2: return (float) DEG(dJointGetHinge2Angle1  (j));
    }
    return 0;
}

static float get_phys_joint_rate(dJointID j, int n)
{
    switch (dJointGetType(j))
    {
    case dJointTypeHinge:  return (float) DEG(dJointGetHingeAngleRate    (j));
    case dJointTypeSlider: return (float)     dJointGetSliderPositionRate(j);
    case dJointTypeHinge2:
        if (n == 1)
            return (float) DEG(dJointGetHinge2Angle1Rate(j));
        else
            return (float) DEG(dJointGetHinge2Angle2Rate(j));
    }
    return 0;
}


static void get_phys_joint_anchor(dJointID j, float *v)
{
    dVector3 V = { 0, 0, 0 };

    switch (dJointGetType(j))
    {
    case dJointTypeBall:      dJointGetBallAnchor     (j, V); break;
    case dJointTypeHinge:     dJointGetHingeAnchor    (j, V); break;
    case dJointTypeHinge2:    dJointGetHinge2Anchor   (j, V); break;
    case dJointTypeUniversal: dJointGetUniversalAnchor(j, V); break;
    }

    v[0] = (float) V[0];
    v[1] = (float) V[1];
    v[2] = (float) V[2];
}

static void get_phys_joint_axis_1(dJointID j, float *v)
{
    dVector3 V = { 0, 0, 0 };

    switch (dJointGetType(j))
    {
    case dJointTypeHinge:     dJointGetHingeAxis     (j, V); break;
    case dJointTypeSlider:    dJointGetSliderAxis    (j, V); break;
    case dJointTypeHinge2:    dJointGetHinge2Axis1   (j, V); break;
    case dJointTypeUniversal: dJointGetUniversalAxis1(j, V); break;
    }

    v[0] = (float) V[0];
    v[1] = (float) V[1];
    v[2] = (float) V[2];
}

static void get_phys_joint_axis_2(dJointID j, float *v)
{
    dVector3 V = { 0, 0, 0 };

    switch (dJointGetType(j))
    {
    case dJointTypeHinge2:    dJointGetHinge2Axis2   (j, V); break;
    case dJointTypeUniversal: dJointGetUniversalAxis2(j, V); break;
    }

    v[0] = (float) V[0];
    v[1] = (float) V[1];
    v[2] = (float) V[2];
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

void set_phys_body_attr_i(dBodyID body, int p, int i)
{
    switch (p)
    {
    case BODY_ATTR_GRAVITY:
        dBodySetGravityMode(body, i);
        break;
    }
}

int get_phys_body_attr_i(dBodyID body, int p)
{
    switch (p)
    {
    case BODY_ATTR_GRAVITY: return dBodyGetGravityMode(body);
    }
    return 0;
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
    case dRayClass:
        transform = dCreateGeomTransform(space);
        object    = dCreateRay(space, (dReal) sqrt(v[3] * v[3] +
                                                   v[4] * v[4] +
                                                   v[5] * v[5]));
        dGeomRaySet(object, v[0], v[1], v[2], v[3], v[4], v[5]);
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
    case GEOM_ATTR_CATEGORY: dGeomSetCategoryBits(geom, i); break;
    case GEOM_ATTR_COLLIDER: dGeomSetCollideBits (geom, i); break;
    case GEOM_ATTR_RESPONSE: get_data(geom)->response = i;  break;
    case GEOM_ATTR_CALLBACK: get_data(geom)->callback = i;  break;
    }
}

void set_phys_geom_attr_f(dGeomID geom, int p, float f)
{
    switch (p)
    {
    case GEOM_ATTR_MASS:     get_data(geom)->mass     = f; break;
    case GEOM_ATTR_BOUNCE:   get_data(geom)->bounce   = f; break;
    case GEOM_ATTR_FRICTION: get_data(geom)->friction = f; break;
    case GEOM_ATTR_SOFT_ERP: get_data(geom)->soft_erp = f; break;
    case GEOM_ATTR_SOFT_CFM: get_data(geom)->soft_cfm = f; break;
    }
}

int get_phys_geom_attr_i(dGeomID geom, int p)
{
    switch (p)
    {
    case GEOM_ATTR_CATEGORY: return dGeomGetCategoryBits(geom);
    case GEOM_ATTR_COLLIDER: return dGeomGetCollideBits (geom);
    case GEOM_ATTR_RESPONSE: return get_data(geom)->response;
    case GEOM_ATTR_CALLBACK: return get_data(geom)->callback;
    }
    return 0;
}

float get_phys_geom_attr_f(dGeomID geom, int p)
{
    switch (p)
    {
    case GEOM_ATTR_MASS:     return (float) get_data(geom)->mass;
    case GEOM_ATTR_BOUNCE:   return (float) get_data(geom)->bounce;
    case GEOM_ATTR_FRICTION: return (float) get_data(geom)->friction;
    case GEOM_ATTR_SOFT_ERP: return (float) get_data(geom)->soft_erp;
    case GEOM_ATTR_SOFT_CFM: return (float) get_data(geom)->soft_cfm;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Joint functions                                                           */

static dJointID find_shared_joint(dBodyID body1, dBodyID body2)
{
    dJointID joint = 0;

    if (body1 == 0)
    {
        if (body2 == 0)
            return 0;
        else
            return find_shared_joint(body2, body1);
    }
    else
    {
        int i, n = dBodyGetNumJoints(body1);

        for (i = 0; i < n; ++i)
            if ((joint = dBodyGetJoint(body1, i)))
            {
                if (dJointGetBody(joint, 0) == body2 ||
                    dJointGetBody(joint, 1) == body2) return joint;
            }

        return 0;
    }
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
    {
        if (body1) dBodyEnable(body1);
        if (body2) dBodyEnable(body2);

        switch (p)
        {
        case dParamLoStop:
        case dParamHiStop:
        case dParamLoStop2:
        case dParamHiStop2:
            set_phys_joint_attr(joint, p, RAD(f));
            break;
        default:
            set_phys_joint_attr(joint, p, f);
            break;
        }
    }
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

float get_phys_join_attr_f(dBodyID body1, dBodyID body2, int p)
{
    dJointID joint = find_shared_joint(body1, body2);

    if (joint)
        switch (p)
        {
        case JOINT_ATTR_VALUE:  return get_phys_joint_value(joint);
        case JOINT_ATTR_RATE_1: return get_phys_joint_rate(joint, 1);
        case JOINT_ATTR_RATE_2: return get_phys_joint_rate(joint, 2);
        case dParamLoStop:
        case dParamHiStop:
        case dParamLoStop2:
        case dParamHiStop2:
            return DEG(get_phys_joint_attr(joint, p));
        default:
            return get_phys_joint_attr(joint, p);
        }

    return 0;
}

void get_phys_join_attr_v(dBodyID body1, dBodyID body2, int p, float *v)
{
    dJointID joint = find_shared_joint(body1, body2);

    if (joint)
        switch (p)
        {
        case JOINT_ATTR_ANCHOR: get_phys_joint_anchor(joint, v); break;
        case JOINT_ATTR_AXIS_1: get_phys_joint_axis_1(joint, v); break;
        case JOINT_ATTR_AXIS_2: get_phys_joint_axis_2(joint, v); break;
        }
}

/*---------------------------------------------------------------------------*/

void add_phys_force(dBodyID body, float x, float y, float z)
{
    if (body)
    {
        dBodyEnable(body);
        dBodyAddForce(body, x, y, z);
    }
}

void add_phys_torque(dBodyID body, float x, float y, float z)
{
    if (body)
    {
        dBodyEnable(body);
        dBodyAddTorque(body, x, y, z);
    }
}

/*---------------------------------------------------------------------------*/
/* Position and rotation accessors                                           */

void set_phys_position(dBodyID body, const float p[3])
{
    dBodySetPosition(body, p[0], p[1], p[2]);
}

void get_phys_position(dBodyID body, float p[3])
{
    p[0] = (float) dBodyGetPosition(body)[0];
    p[1] = (float) dBodyGetPosition(body)[1];
    p[2] = (float) dBodyGetPosition(body)[2];
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
/* Physics state renderers                                                   */

static void draw_phys_plane(dGeomID geom)
{
    dVector4 v;

    dGeomPlaneGetParams(geom, v);
    opengl_draw_grd((float) v[0],
                    (float) v[1],
                    (float) v[2],
                    (float) v[3]);
}

static void draw_phys_box(dGeomID geom)
{
    dVector3 v;

    dGeomBoxGetLengths(geom, v);
    opengl_draw_box((float) v[0],
                    (float) v[1],
                    (float) v[2]);
}

static void draw_phys_sphere(dGeomID geom)
{
    opengl_draw_sph((float) dGeomSphereGetRadius(geom));
}

static void draw_phys_ccylinder(dGeomID geom)
{
    dReal r;
    dReal l;

    dGeomCCylinderGetParams(geom, &r, &l);
    opengl_draw_cap((float) r, (float) l);
}

static void draw_phys_ray(dGeomID geom)
{
    dVector3 p;
    dVector3 v;

    dReal l = dGeomRayGetLength(geom);

    dGeomRayGet(geom, p, v);

    opengl_draw_vec((float)  p[0],
                    (float)  p[1],
                    (float)  p[2],
                    (float) (p[0] + v[0] * l),
                    (float) (p[1] + v[1] * l),
                    (float) (p[2] + v[2] * l));
}

void draw_phys_geom(dGeomID geom)
{
    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glEnable(GL_COLOR_MATERIAL);
        glColor4f(1.0f, 1.0f, 0.0f, 0.5f);

        if (dGeomGetClass(geom) == dPlaneClass)
            draw_phys_plane(geom);
        else
        {
            dGeomID object = dGeomTransformGetGeom(geom);

            switch (dGeomGetClass(object))
            {
            case dBoxClass:       draw_phys_box      (object); break;
            case dSphereClass:    draw_phys_sphere   (object); break;
            case dCCylinderClass: draw_phys_ccylinder(object); break;
            case dRayClass:       draw_phys_ray      (object); break;
            }
        }
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

void draw_phys_body(dBodyID body)
{
    dMass mass;

    dBodyGetMass(body, &mass);

    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_COLOR_MATERIAL);

        opengl_draw_xyz((float) mass.c[0],
                        (float) mass.c[1],
                        (float) mass.c[2]);
    }
    glPopAttrib();
}

/*===========================================================================*/

int startup_physics(void)
{
    world = dWorldCreate();
    space = dHashSpaceCreate(0);
    group = dJointGroupCreate(0);

    dWorldSetGravity(world, 0, (dReal) -9.8, 0);
    dWorldSetAutoDisableFlag(world, 1);

    return 1;
}

