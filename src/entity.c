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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "opengl.h"
#include "frustum.h"
#include "display.h"
#include "physics.h"
#include "matrix.h"
#include "buffer.h"
#include "entity.h"
#include "camera.h"
#include "sprite.h"
#include "object.h"
#include "string.h"
#include "galaxy.h"
#include "terrain.h"
#include "light.h"
#include "pivot.h"
#include "event.h"
#include "utility.h"
#include "tracker.h"
#include "vec.h"

/*---------------------------------------------------------------------------*/

struct entity
{
    unsigned int type;
    unsigned int data;
    unsigned int flags;

    /* Entity transformation. */

    float rotation[16];
    float position[3];
    float scale[3];
    float bound[6];
    float alpha;

    /* Entity hierarchy. */

    unsigned int car;
    unsigned int cdr;
    unsigned int par;

    /* Tracking status. */

    int track_sens;
    int track_mode;

    /* Physical representation. */

    float   center[3];
    dBodyID body;
    dGeomID geom;
};

static struct entity      *entity;
static struct entity_func *entity_func[TYPE_COUNT];

/*---------------------------------------------------------------------------*/

#define ALL_ENTITIES(i, ii) \
    i = ii = 0; vec_all(entity, sizeof (struct entity), &i, &ii);

#define CAR(i) (entity[i].car)
#define CDR(i) (entity[i].cdr)
#define PAR(i) (entity[i].par)

#define FLAG(i, f) (entity[i].flags & f)

static void entity_init_func(unsigned int i)
{
    if (entity_func[entity[i].type] &&
        entity_func[entity[i].type]->init)
        entity_func[entity[i].type]->init(entity[i].data);
}

static void entity_fini_func(unsigned int i)
{
    if (entity_func[entity[i].type] &&
        entity_func[entity[i].type]->fini)
        entity_func[entity[i].type]->fini(entity[i].data);
}
/*
static void entity_aabb_func(unsigned int i, float v[6])
{
    if (entity_func[entity[i].type] &&
        entity_func[entity[i].type]->aabb)
        entity_func[entity[i].type]->aabb(entity[i].data, v);
}
*/
static void entity_draw_func(unsigned int i, int f, float a)
{
    if (entity_func[entity[i].type] &&
        entity_func[entity[i].type]->draw)
        entity_func[entity[i].type]->draw(entity[i].data, i, f, a);
}

static void entity_dupe_func(unsigned int i)
{
    if (entity_func[entity[i].type] &&
        entity_func[entity[i].type]->dupe)
        entity_func[entity[i].type]->dupe(entity[i].data);
}

static void entity_free_func(unsigned int i)
{
    if (entity_func[entity[i].type] &&
        entity_func[entity[i].type]->free)
        entity_func[entity[i].type]->free(entity[i].data);
}

static unsigned int new_entity(void)
{
    unsigned int i;
    void        *v;

    if ((i = vec_add(entity, sizeof (struct entity))))
    {
        memset(entity +i, 0, sizeof (struct entity));
        return i;
    }

    if ((v = vec_gro(entity, sizeof (struct entity))))
    {
        entity = (struct entity *) v;
        return new_entity();
    }
    return 0;
}

/*===========================================================================*/

int get_entity_data(unsigned int i)
{
    return entity[i].data;
}

int get_entity_type(unsigned int i)
{
    return entity[i].type;
}

const char *get_entity_name(unsigned int i)
{
    if (entity_func[entity[i].type])
        return entity_func[entity[i].type]->name;
    else
        return "unknown";
}

/*---------------------------------------------------------------------------*/

void transform_camera(unsigned int i)
{
    float M[16];

    /* Inverse scale. */

    glScalef(1 / entity[i].scale[0],
             1 / entity[i].scale[1],
             1 / entity[i].scale[2]);

    /* Inverse (transposed) rotation. */

    load_xps(M, entity[i].rotation);
    glMultMatrixf(M);

    /* Inverse translation. */

    glTranslatef(-entity[i].position[0],
                 -entity[i].position[1],
                 -entity[i].position[2]);
}

void transform_entity(unsigned int i)
{
    /* Translation. */

    if (entity[i].flags & FLAG_BALLBOARD)
    {
        float p[3];

        get_camera_pos(p);

        glTranslatef(p[0], p[1], p[2]);
    }
    else
        glTranslatef(entity[i].position[0],
                     entity[i].position[1],
                     entity[i].position[2]);

    /* Rotation. */

    glMultMatrixf(entity[i].rotation);

    /* Billboard. */

    if (entity[i].flags & FLAG_BILLBOARD)
    {
        float M[16];

        glGetFloatv(GL_MODELVIEW_MATRIX, M);

        M[0] = 1; M[4] = 0; M[ 8] = 0;
        M[1] = 0; M[5] = 1; M[ 9] = 0;
        M[2] = 0; M[6] = 0; M[10] = 1;

        glLoadMatrixf(M);
    }
/*
    if (entity[i].flags & FLAG_BALLBOARD)
    {
        float M[16];

        glGetFloatv(GL_MODELVIEW_MATRIX, M);

        M[12] = 0;
        M[13] = 0;
        M[14] = 0;
        M[15] = 1;

        glLoadMatrixf(M);
    }
*/

    /* Scale. */

    glScalef(entity[i].scale[0],
             entity[i].scale[1],
             entity[i].scale[2]);

    /* Center of mass. */

    glTranslatef(-entity[i].center[0],
                 -entity[i].center[1],
                 -entity[i].center[2]);
}

/*---------------------------------------------------------------------------*/

int test_entity_aabb(unsigned int i)
{
    float V[6][4];

    /* TODO: rework bound handling and testing. */

    if (entity_func[entity[i].type] &&
        entity_func[entity[i].type]->aabb)
    {
        float aabb[6];

        entity_func[entity[i].type]->aabb(entity[i].data, aabb);
        get_viewfrust(V);

        return test_frustum(V, aabb);
    }
    else if (entity[i].flags & FLAG_BOUNDED)
    {
        get_viewfrust(V);
        return test_frustum(V, entity[i].bound);
    }
    return 1;
}

void draw_entity_tree(unsigned int i, int f, float a)
{
    unsigned int j;
    
    /* Traverse the hierarchy.  Iterate the child list of this entity. */

    for (j = CAR(i); j; j = CDR(j))
    {
        const int e = get_camera_eye();

        const int L =  (FLAG(j, FLAG_LEFT_EYE)  && e == 0);
        const int R =  (FLAG(j, FLAG_RIGHT_EYE) && e == 1);
        const int H = (!FLAG(j, FLAG_LEFT_EYE)  &&
                       !FLAG(j, FLAG_RIGHT_EYE) && !FLAG(j, FLAG_HIDDEN));

        if (H || L || R)
        {
            glPushAttrib(GL_ENABLE_BIT  |
                         GL_POLYGON_BIT |
                         GL_DEPTH_BUFFER_BIT);
            {
                /* Enable wireframe, if specified. */

                if (entity[j].flags & FLAG_WIREFRAME)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                /* Enable line smoothing, if requested. */

                if (entity[j].flags & FLAG_LINE_SMOOTH)
                    glEnable(GL_LINE_SMOOTH);

                /* Draw this entity. */

                entity_draw_func(j, f, a);
            }
            glPopAttrib();
        }
    }

    /* Draw this entity's physical state, if requested. */

    if (entity[i].geom && FLAG(i, FLAG_VISIBLE_GEOM))
        draw_phys_geom(entity[i].geom);
    if (entity[i].body && FLAG(i, FLAG_VISIBLE_BODY))
        draw_phys_body(entity[i].body);
}

void draw_entities(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Begin traversing the scene graph at the root. */

    draw_entity_tree(1, 0, 1);
}

/*===========================================================================*/

static void detach_entity(unsigned int i)
{
    int j, k, p = PAR(i);

    /* Remove the child from its parent's child list. */

    for (j = 0, k = CAR(p); k; j = k, k = CDR(k))
        if (k == i)
        {
            if (j)
                CDR(j) = CDR(k);
            else
                CAR(p) = CDR(k);
        }
}

static void attach_entity(unsigned int i, unsigned int j)
{
    if (i != j)
    {
        /* Insert the child into the new parent's child list. */

        PAR(i) = j;
        CDR(i) = CAR(j);
        CAR(j) = i;
    }
}

/*===========================================================================*/

static void center_geom_entity(dBodyID body, unsigned int i, unsigned int d)
{
    unsigned int j;

    /* Move the body's center of mass to the origin. */

    if (entity[i].geom && d)
        mov_phys_mass(body, entity[i].geom, entity[i].position,
                                            entity[i].rotation);

    /* Continue traversing the hierarchy. */

    for (j = CAR(i); j; j = CDR(j))
        center_geom_entity(body, j, d + 1);
}

static void remass_geom_entity(dBodyID body, unsigned int i, unsigned int d)
{
    unsigned int j;

    /* Accumulate this geom's mass. */

    if (entity[i].geom)
    {
        if (d)
            add_phys_mass(body, entity[i].geom, entity[i].position,
                                                entity[i].rotation);
        else
            add_phys_mass(body, entity[i].geom, NULL, NULL);
    }

    /* Continue traversing the hierarchy. */

    for (j = CAR(i); j; j = CDR(j))
        remass_geom_entity(body, j, d + 1);
}

static void remass_body_entity(unsigned int i, unsigned int j)
{
    /* Compute a body's moment of inertia by adding all child geom masses. */

    if (i)
    {
        new_phys_mass(entity[i].body, entity[i].center);

        remass_geom_entity(entity[i].body, i, 0);
        center_geom_entity(entity[i].body, i, 0);

        end_phys_mass(entity[i].body, entity[i].center);
    }
    else center_geom_entity(0, j, 1);
}

static int find_body_entity(unsigned int i)
{
    /* Search up the tree for an entity with a rigid body strucure. */

    if (i == 0 || entity[i].body)
        return i;
    else
        return find_body_entity(PAR(i));
}

/*---------------------------------------------------------------------------*/

static void create_entity(unsigned int i, int type, int data)
{
    /* Initialize a new entity. */

    load_idt(entity[i].rotation);

    entity[i].type     = type;
    entity[i].data     = data;
    entity[i].flags    = 0;
    entity[i].scale[0] = 1;
    entity[i].scale[1] = 1;
    entity[i].scale[2] = 1;
    entity[i].alpha    = 1;

    /* Attach it at the root. */

    attach_entity(i, 1);
}

unsigned int send_create_entity(int type, int data)
{
    unsigned int i;

    if ((i = new_entity()))
    {
        send_index(type);
        send_index(data);
    
        create_entity(i, type, data);

        return i;
    }
    return 0;
}

void recv_create_entity(void)
{
    unsigned int i = new_entity();

    int type = recv_index();
    int data = recv_index();

    create_entity(i, type, data);
}

/*---------------------------------------------------------------------------*/

void send_parent_entity(unsigned int i, unsigned int j)
{
    send_event(EVENT_PARENT_ENTITY);
    send_index(i);
    send_index(j);

    detach_entity(i);
    attach_entity(i, j);
}

void recv_parent_entity(void)
{
    unsigned int i = recv_index();
    unsigned int j = recv_index();

    detach_entity(i);
    attach_entity(i, j);
}

/*---------------------------------------------------------------------------*/

static void update_entity_position(unsigned int i)
{
    if (entity[i].body)
        set_phys_position(entity[i].body, entity[i].position);
    if (entity[i].geom)
        remass_body_entity(find_body_entity(i), i);
}

static void update_entity_rotation(unsigned int i)
{
    if (entity[i].body)
        set_phys_rotation(entity[i].body, entity[i].rotation);
    if (entity[i].geom)
        remass_body_entity(find_body_entity(i), i);
}

static void set_entity_position(unsigned int i, const float p[3])
{
    send_event(EVENT_SET_ENTITY_POSITION);
    send_index(i);

    send_float((entity[i].position[0] = p[0]));
    send_float((entity[i].position[1] = p[1]));
    send_float((entity[i].position[2] = p[2]));
}

static void set_entity_basis(unsigned int i, const float M[16])
{
    send_event(EVENT_SET_ENTITY_BASIS);
    send_index(i);

    send_float(M[0]);
    send_float(M[1]);
    send_float(M[2]);

    send_float(M[4]);
    send_float(M[5]);
    send_float(M[6]);

    send_float(M[8]);
    send_float(M[9]);
    send_float(M[10]);

    load_mat(entity[i].rotation, M);
}

/*---------------------------------------------------------------------------*/

void send_set_entity_tracking(unsigned int i, int sens, int mode)
{
    entity[i].track_sens = sens;
    entity[i].track_mode = mode;
}

void send_set_entity_rotation(unsigned int i, const float r[3])
{
    float M[16];

    if (entity[i].type == TYPE_CAMERA)
    {
        load_rot_mat(M, 0, 0, 1, r[2]);
        mult_rot_mat(M, 0, 1, 0, r[1]);
        mult_rot_mat(M, 1, 0, 0, r[0]);
    }
    else
    {
        load_rot_mat(M, 1, 0, 0, r[0]);
        mult_rot_mat(M, 0, 1, 0, r[1]);
        mult_rot_mat(M, 0, 0, 1, r[2]);
    }

    send_set_entity_basis(i, M);
}

void send_set_entity_position(unsigned int i, const float p[3])
{
    set_entity_position(i, p);
    update_entity_position(i);
}

void send_set_entity_basis(unsigned int i, const float M[16])
{
    set_entity_basis(i, M);
    update_entity_rotation(i);
}

void send_set_entity_scale(unsigned int i, const float v[3])
{
    send_event(EVENT_SET_ENTITY_SCALE);
    send_index(i);

    send_float((entity[i].scale[0] = v[0]));
    send_float((entity[i].scale[1] = v[1]));
    send_float((entity[i].scale[2] = v[2]));
}

void send_set_entity_bound(unsigned int i, const float b[6])
{
    send_event(EVENT_SET_ENTITY_BOUND);
    send_index(i);

    send_float((entity[i].bound[0] = b[0]));
    send_float((entity[i].bound[1] = b[1]));
    send_float((entity[i].bound[2] = b[2]));
    send_float((entity[i].bound[3] = b[3]));
    send_float((entity[i].bound[4] = b[4]));
    send_float((entity[i].bound[5] = b[5]));
}

void send_set_entity_alpha(unsigned int i, float a)
{
    send_event(EVENT_SET_ENTITY_ALPHA);
    send_index(i);

    send_float((entity[i].alpha = a));
}

void send_set_entity_flags(unsigned int i, int flags, int state)
{
    send_event(EVENT_SET_ENTITY_FLAGS);
    send_index(i);
    send_index(flags);
    send_index(state);

    if (state)
        entity[i].flags = entity[i].flags | ( flags);
    else
        entity[i].flags = entity[i].flags & (~flags);
}

/*---------------------------------------------------------------------------*/

void set_entity_body_type(unsigned int i, int t)
{
    entity[i].body = set_phys_body_type(entity[i].body, t);

    update_entity_position(i);
    update_entity_rotation(i);
}

void set_entity_geom_type(unsigned int i, int t, const float *v)
{
    int j = find_body_entity(i);

    entity[i].geom = set_phys_geom_type(entity[i].geom,
                                        entity[j].body, i, t, v);
    update_entity_position(i);
    update_entity_rotation(i);
}

void set_entity_join_type(unsigned int i, unsigned int j, int t)
{
    set_phys_join_type((i > 1) ? entity[i].body : 0,
                       (j > 1) ? entity[j].body : 0, t);
}

/*---------------------------------------------------------------------------*/

void set_entity_body_attr_i(unsigned int i, int p, int d)
{
    if (entity[i].body)
    {
        set_phys_body_attr_i(entity[i].body, p, d);

        update_entity_position(i);
        update_entity_rotation(i);
    }
}

void set_entity_geom_attr_f(unsigned int i, int p, float f)
{
    if (entity[i].geom)
    {
        set_phys_geom_attr_f(entity[i].geom, p, f);

        update_entity_position(i);
        update_entity_rotation(i);
    }
}

void set_entity_geom_attr_i(unsigned int i, int p, int d)
{
    if (entity[i].geom)
    {
        set_phys_geom_attr_i(entity[i].geom, p, d);

        update_entity_position(i);
        update_entity_rotation(i);
    }
}

void set_entity_join_attr_f(unsigned int i,
                            unsigned int j, int p, float f)
{
    set_phys_join_attr_f((i > 1) ? entity[i].body : 0,
                         (j > 1) ? entity[j].body : 0, p, f);
}

void set_entity_join_attr_v(unsigned int i,
                            unsigned int j, int p, const float *v)
{
    set_phys_join_attr_v((i > 1) ? entity[i].body : 0,
                         (j > 1) ? entity[j].body : 0, p, v);
}

/*---------------------------------------------------------------------------*/

int get_entity_body_attr_i(unsigned int i, int p)
{
    if (entity[i].body)
        return get_phys_body_attr_i(entity[i].body, p);
    else
        return 0;
}

void get_entity_body_attr_v(unsigned int i, int p, float *v)
{
    switch (p)
    {
    case BODY_ATTR_CENTER:
        v[0] = entity[i].center[0];
        v[1] = entity[i].center[1];
        v[2] = entity[i].center[2];
        break;
    }
}

int get_entity_geom_attr_i(unsigned int i, int p)
{
    if (entity[i].geom)
        return get_phys_geom_attr_i(entity[i].geom, p);
    else
        return 0;
}

float get_entity_geom_attr_f(unsigned int i, int p)
{
    if (entity[i].geom)
        return get_phys_geom_attr_f(entity[i].geom, p);
    else
        return 0;
}

float get_entity_join_attr_f(unsigned int i, unsigned int j, int p)
{
    return get_phys_join_attr_f((i > 1) ? entity[i].body : 0,
                                (j > 1) ? entity[j].body : 0, p);
}

void get_entity_join_attr_v(unsigned int i, unsigned int j, int p, float *v)
{
    get_phys_join_attr_v((i > 1) ? entity[i].body : 0,
                         (j > 1) ? entity[j].body : 0, p, v);
}

/*---------------------------------------------------------------------------*/

void add_entity_force(unsigned int i, float x, float y, float z)
{
    if (entity[i].body)
        add_phys_force(entity[i].body, x, y, z);
}

void add_entity_torque(unsigned int i, float x, float y, float z)
{
    if (entity[i].body)
        add_phys_torque(entity[i].body, x, y, z);
}

/*---------------------------------------------------------------------------*/

void send_move_entity(unsigned int i, const float v[3])
{
    float p[3];

    p[0] = entity[i].position[0] + (entity[i].rotation[0]  * v[0] +
                                    entity[i].rotation[4]  * v[1] +
                                    entity[i].rotation[8]  * v[2]);
    p[1] = entity[i].position[1] + (entity[i].rotation[1]  * v[0] +
                                    entity[i].rotation[5]  * v[1] +
                                    entity[i].rotation[9]  * v[2]);
    p[2] = entity[i].position[2] + (entity[i].rotation[2]  * v[0] +
                                    entity[i].rotation[6]  * v[1] +
                                    entity[i].rotation[10] * v[2]);

    send_set_entity_position(i, p);
}

void send_turn_entity(unsigned int i, const float r[3])
{
    float M[16], R[16];

    /* Compose a transformation matrix. */

    load_rot_mat(M, entity[i].rotation[0],
                    entity[i].rotation[1],
                    entity[i].rotation[2], r[0]);
    mult_rot_mat(M, entity[i].rotation[4],
                    entity[i].rotation[5],
                    entity[i].rotation[6], r[1]);
    mult_rot_mat(M, entity[i].rotation[8],
                    entity[i].rotation[9],
                    entity[i].rotation[10], r[2]);

    /* Transform the entity's basis. */

    load_idt(R);

    mult_mat_vec(R + 0, M, entity[i].rotation + 0);
    mult_mat_vec(R + 4, M, entity[i].rotation + 4);
    mult_mat_vec(R + 8, M, entity[i].rotation + 8);

    /* Re-orthogonalize the basis. */

    cross(R + 8, R + 0, R + 4);
    cross(R + 4, R + 8, R + 0);
    cross(R + 0, R + 4, R + 8);

    /* Re-normalize the basis. */

    normalize(R + 0);
    normalize(R + 4);
    normalize(R + 8);

    send_set_entity_basis(i, R);
}

/*---------------------------------------------------------------------------*/

void recv_set_entity_position(void)
{
    unsigned int i = recv_index();

    entity[i].position[0] = recv_float();
    entity[i].position[1] = recv_float();
    entity[i].position[2] = recv_float();
}

void recv_set_entity_basis(void)
{
    unsigned int i = recv_index();

    load_idt(entity[i].rotation);

    entity[i].rotation[0]  = recv_float();
    entity[i].rotation[1]  = recv_float();
    entity[i].rotation[2]  = recv_float();

    entity[i].rotation[4]  = recv_float();
    entity[i].rotation[5]  = recv_float();
    entity[i].rotation[6]  = recv_float();

    entity[i].rotation[8]  = recv_float();
    entity[i].rotation[9]  = recv_float();
    entity[i].rotation[10] = recv_float();
}

void recv_set_entity_scale(void)
{
    unsigned int i = recv_index();

    entity[i].scale[0] = recv_float();
    entity[i].scale[1] = recv_float();
    entity[i].scale[2] = recv_float();
}

void recv_set_entity_alpha(void)
{
    unsigned int i = recv_index();

    entity[i].alpha = recv_float();
}

void recv_set_entity_flags(void)
{
    unsigned int i = recv_index();

    int flags = recv_index();
    int state = recv_index();

    if (state)
        entity[i].flags = entity[i].flags | ( flags);
    else
        entity[i].flags = entity[i].flags & (~flags);
}

void recv_set_entity_bound(void)
{
    unsigned int i = recv_index();

    entity[i].bound[0] = recv_float();
    entity[i].bound[1] = recv_float();
    entity[i].bound[2] = recv_float();
    entity[i].bound[3] = recv_float();
    entity[i].bound[4] = recv_float();
    entity[i].bound[5] = recv_float();
}

/*---------------------------------------------------------------------------*/

void get_entity_position(unsigned int i, float p[3])
{
    p[0] = entity[i].position[0];
    p[1] = entity[i].position[1];
    p[2] = entity[i].position[2];
}

void get_entity_x_vector(unsigned int i, float v[3])
{
    v[0] = entity[i].rotation[0];
    v[1] = entity[i].rotation[1];
    v[2] = entity[i].rotation[2];
}

void get_entity_y_vector(unsigned int i, float v[3])
{
    v[0] = entity[i].rotation[4];
    v[1] = entity[i].rotation[5];
    v[2] = entity[i].rotation[6];
}

void get_entity_z_vector(unsigned int i, float v[3])
{
    v[0] = entity[i].rotation[8];
    v[1] = entity[i].rotation[9];
    v[2] = entity[i].rotation[10];
}

void get_entity_scale(unsigned int i, float v[3])
{
    v[0] = entity[i].scale[0];
    v[1] = entity[i].scale[1];
    v[2] = entity[i].scale[2];
}

void get_entity_bound(unsigned int i, float v[6])
{
    if (entity_func[entity[i].type] &&
        entity_func[entity[i].type]->aabb)
        entity_func[entity[i].type]->aabb(entity[i].data, v);
    else
        memset(v, 0, 6 * sizeof (float));
}

float get_entity_alpha(unsigned int i)
{
    return entity[i].alpha;
}

int get_entity_flags(unsigned int i)
{
    return entity[i].flags;
}

/*---------------------------------------------------------------------------*/

static void create_clone(unsigned int i, unsigned int j)
{
    entity_dupe_func(i);
    create_entity(j, entity[i].type, entity[i].data);
}

unsigned int send_create_clone(unsigned int i)
{
    unsigned int j;

    if ((j = new_entity()))
    {
        send_event(EVENT_CREATE_CLONE);
        send_index(i);

        create_clone(i, j);
    }
    return j;
}

void recv_create_clone(void)
{
    create_clone(recv_index(), new_entity());
}

/*---------------------------------------------------------------------------*/

static void free_entity(unsigned int i)
{
    /* Delete all physical objects. */

    if (entity[i].geom) dGeomDestroy(entity[i].geom);
    if (entity[i].body) dBodyDestroy(entity[i].body);

    /* Delete all child entities. */

    while (CAR(i))
        free_entity(CAR(i));

    /* Remove this entity from the parent's child list. */

    detach_entity(i);

    /* Delete the type-specific data. */

    entity_free_func(i);

    /* Release it. */

    vec_del(entity, sizeof (struct entity), i);
}

void send_delete_entity(unsigned int i)
{
    send_event(EVENT_DELETE_ENTITY);
    send_index(i);

    free_entity(i);
}

void recv_delete_entity(void)
{
    free_entity(recv_index());
}

/*---------------------------------------------------------------------------*/

unsigned int get_entity_parent(unsigned int i)
{
    return PAR(i);
}

unsigned int get_entity_child(unsigned int i, unsigned int n)
{
    unsigned int j;
    unsigned int m;

    for (m = 0, j = CAR(i); j; m++, j = CDR(j))
        if (n == m)
            return j;

    return 0;
}

/*===========================================================================*/

static int step_entity_tree(unsigned int i, float dt, int head,
                            const float view_p[3],
                            const float view_R[16])
{
    int c = 0;
    int j;

    if (entity[i].type)
    {
        float R[16], sens_R[16];
        float p[3],  sens_p[3];

        if (entity[i].type == TYPE_CAMERA)
        {
            /* Automatically track camera offsets. */

            get_tracker_position(head, sens_p);
            get_tracker_rotation(head, sens_R);

            send_set_camera_offset(entity[i].data, sens_p, sens_R);

            /* Track child entities using this camera's transform. */

            for (j = CAR(i); j; j = CDR(j))
                c += step_entity_tree(j, dt, head, entity[i].position,
                                                   entity[i].rotation);
        }
        else
        {
            /* Track non-camera entity position as requested. */

            if (entity[i].flags & FLAG_TRACK_POS)
            {
                get_tracker_position(entity[i].track_sens, sens_p);

                if (entity[i].track_mode == TRACK_WORLD)
                {
                    mult_mat_pos(p, view_R, sens_p);
                    p[0] += view_p[0];
                    p[1] += view_p[1];
                    p[2] += view_p[2];
                    send_set_entity_position(i, p);
                }
                else
                    send_set_entity_position(i, sens_p);

                c++;
            }

            /* Track non-camera entity rotation as requested. */

            if (entity[i].flags & FLAG_TRACK_ROT)
            {
                get_tracker_rotation(entity[i].track_sens, sens_R);

                if (entity[i].track_mode == TRACK_WORLD)
                {
                    mult_mat_mat(R, view_R, sens_R);
                    send_set_entity_basis(i, R);
                }
                else
                    send_set_entity_basis(i, sens_R);

                c++;
            }

            /* Track all child entities. */

            for (j = CAR(i); j; j = CDR(j))
                c += step_entity_tree(j, dt, head, view_p, view_R);
        }
    }

    return c;
}

int step_entities(float dt, int head)
{
    float R[16];
    float p[3];

    int c = 0;

    /* Traverse the entity tree and apply tracking to all tracked entities. */

    if (get_tracker_status())
    {
        load_idt(R);

        p[0] = 0;
        p[1] = 0;
        p[2] = 0;

        c += step_entity_tree(1, dt, head, p, R);
    }

    /* Run the physical simulation and update all entity states. */

    if (physics_step(dt))
    {
        unsigned int i, ii;

        for (ALL_ENTITIES(i, ii))
            if (entity[i].type && entity[i].body)
            {
                get_phys_position(entity[i].body, p);
                set_entity_position(i, p);
                get_phys_rotation(entity[i].body, R);
                set_entity_basis   (i, R);

                c++;
            }
    }

    return c;
}

/*---------------------------------------------------------------------------*/

void nuke_entities(void)
{
    unsigned int i, ii;

    /* Clear all entity state. */

    for (ALL_ENTITIES(i, ii))
        send_delete_entity(i);
}

void init_entities(void)
{
    unsigned int i, ii;

    /* Ask all entities with GL state to initialize themselves. */

    for (ALL_ENTITIES(i, ii))
        entity_init_func(i);
}

void fini_entities(void)
{
    unsigned int i, ii;

    /* Ask all entities with GL state to finalize themselves. */

    for (ALL_ENTITIES(i, ii))
        entity_fini_func(i);
}

/*===========================================================================*/

int startup_entity(void)
{
    if ((entity = vec_new(256, sizeof (struct entity))))
    {
        unsigned int i = new_entity();

        entity[i].type = TYPE_ROOT;

        entity_func[TYPE_NULL]    = NULL;
        entity_func[TYPE_ROOT]    = NULL;
        entity_func[TYPE_CAMERA]  = startup_camera();
        entity_func[TYPE_SPRITE]  = startup_sprite();
        entity_func[TYPE_OBJECT]  = startup_object();
        entity_func[TYPE_STRING]  = startup_string();
        entity_func[TYPE_GALAXY]  = startup_galaxy();
        entity_func[TYPE_TERRAIN] = startup_terrain();
        entity_func[TYPE_LIGHT]   = startup_light();
        entity_func[TYPE_PIVOT]   = startup_pivot();

        return 1;
    }
    return 0;
}

