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
#include "vector.h"
#include "display.h"
#include "matrix.h"
#include "buffer.h"
#include "entity.h"
#include "camera.h"
#include "sprite.h"
#include "object.h"
#include "string.h"
#include "galaxy.h"
#include "light.h"
#include "pivot.h"
#include "event.h"
#include "utility.h"
#include "tracker.h"

/*---------------------------------------------------------------------------*/

struct entity
{
    int type;
    int data;
    int flag;
    int state;

    /* Entity transformation. */

    float rotation[16];
    float position[3];
    float scale[3];
    float bound[6];
    float alpha;

    /* Entity fragment and vertex programs. */

    char  *frag_text;
    char  *vert_text;
    GLuint frag_prog;
    GLuint vert_prog;

    /* Entity hierarchy. */

    int car;
    int cdr;
    int par;
};

static vector_t            entity;
static struct entity_func *entity_func[TYPE_COUNT];

/*---------------------------------------------------------------------------*/

#define E(i) ((struct entity *) vecget(entity, i))

/*
struct entity *E(int i)
{
    return (struct entity *) vecget(entity, i);
}
*/

static int new_entity(void)
{
    int i, n = vecnum(entity);

    for (i = 0; i < n; ++i)
        if (E(i)->type == 0)
            return i;

    return vecadd(entity);
}

int startup_entity(void)
{
    if ((entity = vecnew(256, sizeof (struct entity))))
    {
        E(vecadd(entity))->type = TYPE_ROOT;

        entity_func[TYPE_NULL]   = NULL;
        entity_func[TYPE_ROOT]   = NULL;
        entity_func[TYPE_CAMERA] = startup_camera();
        entity_func[TYPE_SPRITE] = startup_sprite();
        entity_func[TYPE_OBJECT] = startup_object();
        entity_func[TYPE_STRING] = startup_string();
        entity_func[TYPE_GALAXY] = startup_galaxy();
        entity_func[TYPE_LIGHT]  = startup_light();
        entity_func[TYPE_PIVOT]  = startup_pivot();

        return 1;
    }
    return 0;
}

/*===========================================================================*/

int entity_data(int i)
{
    return E(i)->data;
}

int entity_type(int i)
{
    return E(i)->type;
}

int entity_flag(int i)
{
    return E(i)->flag;
}

const char *entity_name(int i)
{
    if (entity_func[E(i)->type])
        return entity_func[E(i)->type]->name;
    else
        return "unknown";
}

/*---------------------------------------------------------------------------*/

void transform_camera(int i)
{
    float M[16];

    /* Inverse scale. */

    glScalef(1 / E(i)->scale[0],
             1 / E(i)->scale[1],
             1 / E(i)->scale[2]);

    /* Inverse (transposed) rotation. */

    load_xps(M, E(i)->rotation);
    glMultMatrixf(M);

    /* Inverse translation. */

    glTranslatef(-E(i)->position[0],
                 -E(i)->position[1],
                 -E(i)->position[2]);
}

void transform_entity(int i)
{
    /* Translation. */

    glTranslatef(E(i)->position[0],
                 E(i)->position[1],
                 E(i)->position[2]);

    /* Rotation. */

    glMultMatrixf(E(i)->rotation);

    /* Billboard. */

    if (E(i)->flag & FLAG_BILLBOARD)
    {
        float M[16];

        glGetFloatv(GL_MODELVIEW_MATRIX, M);

        M[0] = 1; M[4] = 0; M[ 8] = 0;
        M[1] = 0; M[5] = 1; M[ 9] = 0;
        M[2] = 0; M[6] = 0; M[10] = 1;

        glLoadMatrixf(M);
    }

    /* Scale. */

    glScalef(E(i)->scale[0],
             E(i)->scale[1],
             E(i)->scale[2]);
}

/*---------------------------------------------------------------------------*/

static void init_entity(int i)
{
    if (E(i)->state == 0)
    {
        /* Find the bounding box of this entity, if any. */

        if (entity_func[E(i)->type]       &&
            entity_func[E(i)->type]->bbox &&
            entity_func[E(i)->type]->bbox(E(i)->data, E(i)->bound))
            E(i)->flag |= FLAG_BOUNDED;

        /* Initialize any vertex and fragment programs. */

        if (E(i)->vert_text && GL_has_vertex_program)
            E(i)->vert_prog = opengl_vert_prog(E(i)->vert_text);

        if (E(i)->frag_text && GL_has_fragment_program)
            E(i)->frag_prog = opengl_frag_prog(E(i)->frag_text);

        E(i)->state = 1;
    }
}

static void fini_entity(int i)
{
    if (E(i)->state == 1)
    {
        /* Finalize any vertex and fragment programs. */

        if (GL_has_vertex_program)
            if (glIsProgramARB(E(i)->vert_prog))
                glDeleteProgramsARB(1, &E(i)->vert_prog);

        if (GL_has_fragment_program)
            if (glIsProgramARB(E(i)->frag_prog))
                glDeleteProgramsARB(1, &E(i)->frag_prog);
                
        E(i)->vert_prog = 0;
        E(i)->frag_prog = 0;
        E(i)->state     = 0;
    }
}

/*---------------------------------------------------------------------------*/

int test_entity_bbox(int i)
{
    struct frustum F;

    if (E(i)->flag & FLAG_BOUNDED)
    {
        get_frustum(&F);
        return tst_frustum(&F, E(i)->bound);
    }

    return 1;
}

void draw_entity_tree(int i, int f, float a)
{
    int j;

    /* Traverse the hierarchy.  Iterate the child list of this entity. */

    for (j = E(i)->car; j; j = E(j)->cdr)
        if ((E(j)->flag & FLAG_HIDDEN) == 0)
        {
            init_entity(j);

            glPushAttrib(GL_POLYGON_BIT |
                         GL_ENABLE_BIT  |
                         GL_DEPTH_BUFFER_BIT);
            {
                /* Enable wireframe if specified. */

                if (E(j)->flag & FLAG_WIREFRAME)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                /* Disable lighting if requested. */

                if (E(j)->flag & FLAG_UNLIT)
                    glDisable(GL_LIGHTING);

                /* Disable depth writing of transparent objects. */

                if (E(j)->flag & FLAG_TRANSPARENT)
                    glDepthMask(GL_FALSE);

                /* Enable line smoothing if requested. */

                if (E(j)->flag & FLAG_LINE_SMOOTH)
                    glEnable(GL_LINE_SMOOTH);

                /* Enable vertex and fragment programs if specified. */

                if (E(j)->frag_prog && GL_has_fragment_program)
                {
                    glEnable(GL_FRAGMENT_PROGRAM_ARB);
                    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, E(j)->frag_prog);
                }

                if (E(j)->vert_prog && GL_has_vertex_program)
                {
                    glEnable(GL_VERTEX_PROGRAM_ARB);
                    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
                    glBindProgramARB(GL_VERTEX_PROGRAM_ARB,   E(j)->vert_prog);
                }

                /* Draw this entity. */

                if (f & DRAW_VARRIER_TEXGEN)
                    set_texture_coordinates();

                if (entity_func[E(j)->type] &&
                    entity_func[E(j)->type]->draw)
                    entity_func[E(j)->type]->draw(j, E(j)->data, f, a);
            }
            glPopAttrib();
        }
}

/*===========================================================================*/

static void detach_entity(int i)
{
    /* Never allow the root entity to be used as a child. */

    if (i)
    {
        int j, k, p = E(i)->par;

        /* Remove the child from its parent's child list. */

        for (j = 0, k = E(p)->car; k; j = k, k = E(k)->cdr)
            if (k == i)
            {
                if (j)
                    E(j)->cdr = E(k)->cdr;
                else
                    E(p)->car = E(k)->cdr;
            }
    }
}

static void attach_entity(int i, int j)
{
    /* Never allow the root entity to be used as a child. */

    if (i && i != j)
    {
        /* Insert the child into the new parent's child list. */

        E(i)->par = j;
        E(i)->cdr = E(j)->car;
        E(j)->car = i;
    }
}

/*===========================================================================*/

static void create_entity(int i, int type, int data)
{
    load_idt(E(i)->rotation);

    E(i)->type     = type;
    E(i)->data     = data;
    E(i)->flag     = 0;
    E(i)->scale[0] = 1;
    E(i)->scale[1] = 1;
    E(i)->scale[2] = 1;
    E(i)->alpha    = 1;

    if (E(i)->type == TYPE_SPRITE)
        E(i)->flag =  FLAG_TRANSPARENT;

    attach_entity(i, 0);
}

int send_create_entity(int type, int data)
{
    int i;

    if ((i = new_entity()) >= 0)
    {
        send_index(type);
        send_index(data);
    
        create_entity(i, type, data);

        return i;
    }
    return -1;
}

void recv_create_entity(void)
{
    int i    = new_entity();
    int type = recv_index();
    int data = recv_index();

    create_entity(i, type, data);
}

/*---------------------------------------------------------------------------*/

void send_parent_entity(int i, int j)
{
    send_event(EVENT_PARENT_ENTITY);
    send_index(i);
    send_index(j);

    detach_entity(i);
    attach_entity(i, j);
}

void recv_parent_entity(void)
{
    int i = recv_index();
    int j = recv_index();

    detach_entity(i);
    attach_entity(i, j);
}

/*---------------------------------------------------------------------------*/

void send_set_entity_rotation(int i, const float r[3])
{
    float M[16];

    if (E(i)->type == TYPE_CAMERA)
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

void send_set_entity_position(int i, const float p[3])
{
    send_event(EVENT_SET_ENTITY_POSITION);
    send_index(i);

    send_float((E(i)->position[0] = p[0]));
    send_float((E(i)->position[1] = p[1]));
    send_float((E(i)->position[2] = p[2]));
}

void send_set_entity_basis(int i, const float M[16])
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

    load_mat(E(i)->rotation, M);
}

void send_set_entity_scale(int i, const float v[3])
{
    send_event(EVENT_SET_ENTITY_SCALE);
    send_index(i);

    send_float((E(i)->scale[0] = v[0]));
    send_float((E(i)->scale[1] = v[1]));
    send_float((E(i)->scale[2] = v[2]));
}

void send_set_entity_bound(int i, const float v[3])
{
    send_event(EVENT_SET_ENTITY_BOUND);
    send_index(i);

    send_float((E(i)->bound[0] = v[0]));
    send_float((E(i)->bound[1] = v[1]));
    send_float((E(i)->bound[2] = v[2]));
    send_float((E(i)->bound[3] = v[3]));
    send_float((E(i)->bound[4] = v[4]));
    send_float((E(i)->bound[5] = v[5]));
}

void send_set_entity_alpha(int i, float a)
{
    send_event(EVENT_SET_ENTITY_ALPHA);
    send_index(i);

    send_float((E(i)->alpha = a));
}

void send_set_entity_flag(int i, int flags, int state)
{
    send_event(EVENT_SET_ENTITY_FLAG);
    send_index(i);
    send_index(flags);
    send_index(state);

    if (state)
        E(i)->flag = E(i)->flag | ( flags);
    else
        E(i)->flag = E(i)->flag & (~flags);
}

void send_set_entity_frag_prog(int i, const char *text)
{
    int n = text ? (strlen(text) + 1) : 0;

    send_event(EVENT_SET_ENTITY_FRAG_PROG);
    send_index(i);
    send_index(n);

    fini_entity(i);

    if (E(i)->frag_text)
    {
        free(E(i)->frag_text);
        E(i)->frag_text = NULL;
    }

    if (n > 0)
    {
        E(i)->frag_text = memdup(text, n, 1);
        send_array(text, n, 1);
    }
}

void send_set_entity_vert_prog(int i, const char *text)
{
    int n = text ? (strlen(text) + 1) : 0;

    send_event(EVENT_SET_ENTITY_VERT_PROG);
    send_index(i);
    send_index(n);

    fini_entity(i);

    if (E(i)->vert_text)
    {
        free(E(i)->vert_text);
        E(i)->vert_text = NULL;
    }

    if (n > 0)
    {
        E(i)->vert_text = memdup(text, n, 1);
        send_array(text, n, 1);
    }
}

/*---------------------------------------------------------------------------*/

void send_move_entity(int i, const float v[3])
{
    float p[3];

    p[0] = E(i)->position[0] + (E(i)->rotation[0]  * v[0] +
                                E(i)->rotation[4]  * v[1] +
                                E(i)->rotation[8]  * v[2]);
    p[1] = E(i)->position[1] + (E(i)->rotation[1]  * v[0] +
                                E(i)->rotation[5]  * v[1] +
                                E(i)->rotation[9]  * v[2]);
    p[2] = E(i)->position[2] + (E(i)->rotation[2]  * v[0] +
                                E(i)->rotation[6]  * v[1] +
                                E(i)->rotation[10] * v[2]);

    send_set_entity_position(i, p);
}

void send_turn_entity(int i, const float r[3])
{
    float M[16], R[16];

    /* Compose a transformation matrix. */

    load_rot_mat(M, E(i)->rotation[0],
                    E(i)->rotation[1],
                    E(i)->rotation[2], r[0]);
    mult_rot_mat(M, E(i)->rotation[4],
                    E(i)->rotation[5],
                    E(i)->rotation[6], r[1]);
    mult_rot_mat(M, E(i)->rotation[8],
                    E(i)->rotation[9],
                    E(i)->rotation[10], r[2]);

    /* Transform the entity's basis. */

    load_idt(R);

    mult_mat_vec(R + 0, M, E(i)->rotation + 0);
    mult_mat_vec(R + 4, M, E(i)->rotation + 4);
    mult_mat_vec(R + 8, M, E(i)->rotation + 8);

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
    int i = recv_index();

    E(i)->position[0] = recv_float();
    E(i)->position[1] = recv_float();
    E(i)->position[2] = recv_float();
}

void recv_set_entity_basis(void)
{
    int i = recv_index();

    load_idt(E(i)->rotation);

    E(i)->rotation[0]  = recv_float();
    E(i)->rotation[1]  = recv_float();
    E(i)->rotation[2]  = recv_float();

    E(i)->rotation[4]  = recv_float();
    E(i)->rotation[5]  = recv_float();
    E(i)->rotation[6]  = recv_float();

    E(i)->rotation[8]  = recv_float();
    E(i)->rotation[9]  = recv_float();
    E(i)->rotation[10] = recv_float();
}

void recv_set_entity_scale(void)
{
    int i = recv_index();

    E(i)->scale[0] = recv_float();
    E(i)->scale[1] = recv_float();
    E(i)->scale[2] = recv_float();
}

void recv_set_entity_bound(void)
{
    int i = recv_index();

    E(i)->bound[0] = recv_float();
    E(i)->bound[1] = recv_float();
    E(i)->bound[2] = recv_float();
    E(i)->bound[3] = recv_float();
    E(i)->bound[4] = recv_float();
    E(i)->bound[5] = recv_float();
}

void recv_set_entity_alpha(void)
{
    int i = recv_index();

    E(i)->alpha = recv_float();
}

void recv_set_entity_flag(void)
{
    int i     = recv_index();
    int flags = recv_index();
    int state = recv_index();

    if (state)
        E(i)->flag = E(i)->flag | ( flags);
    else
        E(i)->flag = E(i)->flag & (~flags);
}

void recv_set_entity_frag_prog(void)
{
    int i = recv_index();
    int n = recv_index();

    fini_entity(i);

    if (E(i)->frag_text)
    {
        free(E(i)->frag_text);
        E(i)->frag_text = NULL;
    }

    if (n > 0)
    {
        E(i)->frag_text = (char *) malloc(n);
        recv_array(E(i)->frag_text, n, 1);
    }
}

void recv_set_entity_vert_prog(void)
{
    int i = recv_index();
    int n = recv_index();

    fini_entity(i);

    if (E(i)->vert_text)
    {
        free(E(i)->vert_text);
        E(i)->vert_text = NULL;
    }

    if (n > 0)
    {
        E(i)->vert_text = (char *) malloc(n);
        recv_array(E(i)->vert_text, n, 1);
    }
}

/*---------------------------------------------------------------------------*/

void get_entity_position(int i, float p[3])
{
    p[0] = E(i)->position[0];
    p[1] = E(i)->position[1];
    p[2] = E(i)->position[2];
}

void get_entity_x_vector(int i, float v[3])
{
    v[0] = E(i)->rotation[0];
    v[1] = E(i)->rotation[1];
    v[2] = E(i)->rotation[2];
}

void get_entity_y_vector(int i, float v[3])
{
    v[0] = E(i)->rotation[4];
    v[1] = E(i)->rotation[5];
    v[2] = E(i)->rotation[6];
}

void get_entity_z_vector(int i, float v[3])
{
    v[0] = E(i)->rotation[8];
    v[1] = E(i)->rotation[9];
    v[2] = E(i)->rotation[10];
}

void get_entity_scale(int i, float v[3])
{
    v[0] = E(i)->scale[0];
    v[1] = E(i)->scale[1];
    v[2] = E(i)->scale[2];
}

void get_entity_bound(int i, float v[6])
{
    if (entity_func[E(i)->type]       &&
        entity_func[E(i)->type]->bbox &&
        entity_func[E(i)->type]->bbox(E(i)->data, E(i)->bound))
        E(i)->flag |= FLAG_BOUNDED;

    v[0] = E(i)->bound[0];
    v[1] = E(i)->bound[1];
    v[2] = E(i)->bound[2];
    v[3] = E(i)->bound[3];
    v[4] = E(i)->bound[4];
    v[5] = E(i)->bound[5];
}

float get_entity_alpha(int i)
{
    return E(i)->alpha;
}

int get_entity_flag(int i)
{
    return E(i)->flag;
}

/*---------------------------------------------------------------------------*/

static void create_clone(int i, int j)
{
    if (entity_func[E(i)->type] &&
        entity_func[E(i)->type]->dupe)
        entity_func[E(i)->type]->dupe(E(i)->data);

    create_entity(j, E(i)->type, E(i)->data);
}

int send_create_clone(int i)
{
    int j;

    if ((j = new_entity()) >= 0)
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

static void free_entity(int i)
{
    if (i)
    {
        fini_entity(i);

        /* Delete all child entities. */

        while (E(i)->car)
            free_entity(E(i)->car);

        /* Remove this entity from the parent's child list. */

        detach_entity(i);

        /* Delete the type-specific data. */

        if (entity_func[E(i)->type] &&
            entity_func[E(i)->type]->free)
            entity_func[E(i)->type]->free(E(i)->data);

        /* Pave it. */

        memset(E(i), 0, sizeof (struct entity));
    }
}

void send_delete_entity(int i)
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

int get_entity_parent(int i)
{
    return E(i)->par;
}

int get_entity_child(int i, int n)
{
    int j, m;

    for (m = 0, j = E(i)->car; j; m++, j = E(j)->cdr)
        if (n == m)
            return j;

    return -1;
}

/*===========================================================================*/

void draw_entities(void)
{
    glLoadIdentity();

    /* Begin traversing the scene graph at the root. */

    draw_entity_tree(0, 0, 1);
    opengl_check("draw_entities");
}

void step_entities(void)
{
    float r[2][3];
    float p[2][3];
    float M[16];

    int i, n = vecnum(entity);

    /* Acquire tracking info. */

    get_tracker_position(0, p[0]);
    get_tracker_position(1, p[1]);
    get_tracker_rotation(0, r[0]);
    get_tracker_rotation(1, r[1]);

    load_rot_mat(M, 1, 0, 0, r[0][0]);
    mult_rot_mat(M, 0, 1, 0, r[0][1]);
    mult_rot_mat(M, 0, 0, 1, r[0][2]);

    /* Distribute it to all cameras and tracked entities. */

    for (i = 0; i < n; ++i)
        if (E(i)->type == TYPE_CAMERA)
            send_set_camera_offset(E(i)->data, p[0], M);

        else if (E(i)->type)
        {
            if (E(i)->flag & FLAG_POS_TRACKED_0)
                send_set_entity_position(i, p[0]);
            if (E(i)->flag & FLAG_POS_TRACKED_1)
                send_set_entity_position(i, p[1]);

            if (E(i)->flag & FLAG_ROT_TRACKED_0)
                send_set_entity_rotation(i, r[0]);
            if (E(i)->flag & FLAG_ROT_TRACKED_1)
                send_set_entity_rotation(i, r[1]);
        }
}

/*---------------------------------------------------------------------------*/

void init_entities(void)
{
    int i, n = vecnum(entity);

    /* Ask all entities with GL state to initialize themselves. */

    for (i = 0; i < n; ++i)
        if (E(i)->type)
        {
            init_entity(i);

            if (entity_func[E(i)->type] &&
                entity_func[E(i)->type]->init)
                entity_func[E(i)->type]->init(E(i)->data);
        }
}

void fini_entities(void)
{
    int i, n = vecnum(entity);

    /* Ask all entities with GL state to finalize themselves. */

    for (i = 0; i < n; ++i)
        if (E(i)->type)
        {
            fini_entity(i);

            if (entity_func[E(i)->type] &&
                entity_func[E(i)->type]->fini)
                entity_func[E(i)->type]->fini(E(i)->data);
        }
}

/*===========================================================================*/
