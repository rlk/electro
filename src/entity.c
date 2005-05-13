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
#include "matrix.h"
#include "buffer.h"
#include "camera.h"
#include "sprite.h"
#include "object.h"
#include "galaxy.h"
#include "light.h"
#include "pivot.h"
#include "event.h"
#include "utility.h"
#include "tracker.h"
#include "entity.h"

/*---------------------------------------------------------------------------*/

struct entity
{
    int type;
    int data;
    int flag;

    float position[3];
    float basis[3][3];
    float scale[3];
    float alpha;

    char  *frag_text;
    char  *vert_text;
    GLuint frag_prog;
    GLuint vert_prog;

    int car;
    int cdr;
    int par;
};

/*---------------------------------------------------------------------------*/

static vector_t E;

#define get_entity(j) ((struct entity *) vecget(E, j))

static int new_entity(void)
{
    int n = vecnum(E);
    int i;

    for (i = 0; i < n; ++i)
        if (get_entity(i)->type == 0)
            return i;

    return vecadd(E);
}

/*---------------------------------------------------------------------------*/

int entity_data(int j)
{
    return get_entity(j)->data;
}

int entity_type(int j)
{
    return get_entity(j)->type;
}

int entity_flag(int j)
{
    return get_entity(j)->flag;
}

/*---------------------------------------------------------------------------*/

static void basis_mult(float e[3][3])
{
    float M[16];

    M[0] = e[0][0]; M[4] = e[1][0]; M[8]  = e[2][0]; M[12] = 0.0f;
    M[1] = e[0][1]; M[5] = e[1][1]; M[9]  = e[2][1]; M[13] = 0.0f;
    M[2] = e[0][2]; M[6] = e[1][2]; M[10] = e[2][2]; M[14] = 0.0f;
    M[3] =    0.0f; M[7] =    0.0f; M[11] =    0.0f; M[15] = 1.0f;

    glMultMatrixf(M);
}

static void basis_invt(float e[3][3])
{
    float M[16];

    M[0] = e[0][0]; M[4] = e[0][1]; M[8]  = e[0][2]; M[12] = 0.0f;
    M[1] = e[1][0]; M[5] = e[1][1]; M[9]  = e[1][2]; M[13] = 0.0f;
    M[2] = e[2][0]; M[6] = e[2][1]; M[10] = e[2][2]; M[14] = 0.0f;
    M[3] =    0.0f; M[7] =    0.0f; M[11] =    0.0f; M[15] = 1.0f;

    glMultMatrixf(M);
}

/*---------------------------------------------------------------------------*/

void transform_camera(int j, float N[16], const float M[16],
                             float J[16], const float I[16], const float p[3])
{
    struct entity *e = get_entity(j);

    float A[16];
    float B[16];
    float q[3];

    m_copy(N, M);
    m_copy(J, I);

    /* Camera tracking */

    q[0] = -p[0];
    q[1] = -p[1];
    q[2] = -p[2];

    glTranslatef(q[0], q[1], q[2]);
    m_trans(A, B, q);
    m_mult(N, N, A);
    m_mult(J, B, J);

    /* Camera rotation. */

    basis_invt(e->basis);
    m_basis(B, A, e->basis[0], e->basis[1], e->basis[2]);
    m_mult(N, N, A);
    m_mult(J, B, J);

    /* Camera position. */

    q[0] = -e->position[0];
    q[1] = -e->position[1];
    q[2] = -e->position[2];

    glTranslatef(q[0], q[1], q[2]);
    m_trans(A, B, e->position);
    m_mult(N, N, A);
    m_mult(J, B, J);
}

void transform_entity(int j, float N[16], const float M[16],
                             float J[16], const float I[16])
{
    struct entity *e = get_entity(j);

    float A[16];
    float B[16];

    m_copy(N, M);
    m_copy(J, I);

    /* Entity position. */

    glTranslatef(e->position[0],
                 e->position[1],
                 e->position[2]);

    m_trans(A, B, e->position);
    m_mult(N, N, A);
    m_mult(J, B, J);

    /* Entity rotation. */

    basis_mult(e->basis);

    m_basis(A, B, e->basis[0],
                  e->basis[1],
                  e->basis[2]);
    m_mult(N, N, A);
    m_mult(J, B, J);

    /* Billboard. */

    if (e->flag & FLAG_BILLBOARD)
    {
        glGetFloatv(GL_MODELVIEW_MATRIX, A);

        A[0] = 1.f;  A[4] = 0.f;  A[8]  = 0.f;
        A[1] = 0.f;  A[5] = 1.f;  A[9]  = 0.f;
        A[2] = 0.f;  A[6] = 0.f;  A[10] = 1.f;

        glLoadMatrixf(A);

        m_xpos(B, A);
        m_mult(N, N, A);
        m_mult(J, B, J);
    }

    /* Scale. */

    glScalef(e->scale[0],
             e->scale[1],
             e->scale[2]);

    m_scale(A, B, e->scale);
    m_mult(N, N, A);
    m_mult(J, B, J);
}

void draw_entity_list(int j, const float M[16],
                             const float I[16],
                             const struct frustum *F, float a)
{
    /* Traverse the hierarchy.  Iterate the child list of this entity. */

    for (k = get_entity(j)->car; k; k = get_entity(k)->cdr)
    {
        struct entity *e = get_entity(k);

        if ((e->flag & FLAG_HIDDEN) == 0)
        {
            glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
            {
                /* Enable wireframe if specified. */

                if (e->flag & FLAG_WIREFRAME)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                /* Disable lighting if requested. */

                if (e->flag & FLAG_UNLIT)
                    glDisable(GL_LIGHTING);

                /* Enable line smoothing if requested. */

                if (e->flag & FLAG_LINE_SMOOTH)
                    glEnable(GL_LINE_SMOOTH);

                /* Enable vertex and fragment programs if specified. */

                if (e->frag_prog && GL_has_fragment_program)
                {
                    glEnable(GL_FRAGMENT_PROGRAM_ARB);
                    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, e->frag_prog);
                }

                if (e->vert_prog && GL_has_vertex_program)
                {
                    glEnable(GL_VERTEX_PROGRAM_ARB);
                    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
                    glBindProgramARB(GL_VERTEX_PROGRAM_ARB,   e->vert_prog);
                }

                /* Draw this entity. */

                switch (e->type)
                {
                case TYPE_CAMERA: draw_camera(k, e->data, M, I, F, a); break;
                case TYPE_SPRITE: draw_sprite(k, e->data, M, I, F, a); break;
                case TYPE_OBJECT: draw_object(k, e->data, M, I, F, a); break;
                case TYPE_GALAXY: draw_galaxy(k, e->data, M, I, F, a); break;
                case TYPE_LIGHT:  draw_light (k, e->data, M, I, F, a); break;
                case TYPE_PIVOT:  draw_pivot (k, e->data, M, I, F, a); break;
                }
            }
            glPopAttrib();

            /* Protect the space-time continuum. */

            opengl_check(get_entity_debug_id(k));
        }
    }
}

/*---------------------------------------------------------------------------*/

int init_entity(void)
{
    if ((E = vecnew(256, sizeof (struct entity))))
    {
        E[0].type = TYPE_ROOT;

        return (init_camera() && init_galaxy() &&
                init_sprite() && init_object() && init_light());
    }
    return 0;
}

void draw_entity(void)
{
    struct frustum F;
    float M[16];
    float I[16];

    /* Initialize the view frustum and transform matrices. */

    memset(&F, 0, sizeof (struct frustum));
    m_init(M);
    m_init(I);

    glLoadIdentity();

    /* Begin traversing the scene graph at the root. */

    glPushAttrib(GL_SCISSOR_BIT | GL_VIEWPORT_BIT);
    {
        draw_entity_list(0, M, I, &F, 1.0f);
    }
    glPopAttrib();
}

void step_entity(void)
{
    float r[2][3];
    float p[2][3];
    float e[3][3];

    int n = vecnum(E);
    int j;

    /* Acquire tracking info. */

    get_tracker_position(0, p[0]);
    get_tracker_position(1, p[1]);
    get_tracker_rotation(0, r[0]);
    get_tracker_rotation(1, r[1]);

    v_basis(e, r[0]);

    /* Distribute it to all cameras and tracked entities. */

    for (j = 0; j < n; ++j)
    {
        int type = get_entity_type(j);
        int data = get_entity_data(j);
        int flag = get_entity_flag(j);

        if (E[j].type == TYPE_CAMERA)
            send_set_camera_offset(data, p[0], e);

        else if (E[j].type)
        {
            if (flag & FLAG_POS_TRACKED_0)
                send_set_entity_position(j, p[0]);
            if (flag & FLAG_POS_TRACKED_1)
                send_set_entity_position(j, p[1]);

            if (flag & FLAG_ROT_TRACKED_0)
                send_set_entity_rotation(j, r[0]);
            if (flag & FLAG_ROT_TRACKED_1)
                send_set_entity_rotation(j, r[1]);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void init_entity_frag_prog(struct entity *e)
{
    glGenProgramsARB(1, &e->vert_prog);
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, e->vert_prog);

    glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB,
                       strlen(e->vert_text), e->vert_text);

    if (glGetError() == GL_INVALID_OPERATION)
        print("Vertex program: %s\n",
              glGetString(GL_PROGRAM_ERROR_STRING_ARB));
}

static void init_entity_vert_prog(struct entity *e)
{
    glGenProgramsARB(1, &e->frag_prog);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, e->frag_prog);

    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB,
                       strlen(e->frag_text), e->frag_text);

    if (glGetError() == GL_INVALID_OPERATION)
        print("Fragment program: %s\n",
              glGetString(GL_PROGRAM_ERROR_STRING_ARB));
}

/*---------------------------------------------------------------------------*/

void init_entity_gl(void)
{
    int n = vecnum(E);
    int j;

    /* Ask all entities with GL state to initialize themselves. */

    for (j = 0; j < n; ++j)
    {
        struct entity *e = get_entity(j);

        switch (e->type)
        {
        case TYPE_SPRITE: init_sprite_gl(e->data); break;
        case TYPE_OBJECT: init_object_gl(e->data); break;
        case TYPE_GALAXY: init_galaxy_gl(e->data); break;
        }

        /* Initialize any vertex and fragment programs. */

        if (GL_has_vertex_program && e->vert_text)
            init_entity_vert_prog(e);

        if (GL_has_fragment_program && e->frag_text)
            init_entity_frag_prog(e);
    }
}

void free_entity_gl(void)
{
    int n = vecnum(E);
    int j;

    /* Ask all entities with GL state to finalize themselves. */

    for (j = 0; j < E_max; ++j)
    {
        struct entity *e = get_entity(j);

        switch (e->type)
        {
        case TYPE_SPRITE: free_sprite_gl(e->data); break;
        case TYPE_OBJECT: free_object_gl(e->data); break;
        case TYPE_GALAXY: free_galaxy_gl(e->data); break;
        }

        /* Finalize any vertex and fragment programs. */

        if (GL_has_vertex_program)
            if (glIsProgramARB(e->vert_prog))
                glDeleteProgramsARB(1, &e->vert_prog);

        if (GL_has_fragment_program)
            if (glIsProgramARB(e->frag_prog))
                glDeleteProgramsARB(1, &e->frag_prog);
                
        e->vert_prog = 0;
        e->frag_prog = 0;
    }
}

/*---------------------------------------------------------------------------*/

void detach_entity(int cd, int pd)
{
    /* Never allow the root entity to be used as a child. */

    if (cd && entity_exists(pd) && entity_exists(cd))
    {
        int j;
        int jd;
        int od = E[cd].par;

        /* Remove the child from its parent's child list. */

        for (jd = 0, j = E[od].car; j; jd = j, j = E[j].cdr)
            if (j == cd)
            {
                if (jd)
                    E[jd].cdr = E[j].cdr;
                else
                    E[od].car = E[j].cdr;
            }
    }
}

void attach_entity(int cd, int pd)
{
    /* Never allow the root entity to be used as a child. */

    if (cd && entity_exists(pd) && entity_exists(cd) && pd != cd)
    {
        /* Insert the child into the new parent's child list. */

        E[cd].par = pd;
        E[cd].cdr = E[pd].car;
        E[pd].car = cd;
    }
}

/*---------------------------------------------------------------------------*/

static void create_entity(int j, int type, int data)
{
    E[j].type = type;
    E[j].data = data;
    E[j].flag = 0;

    E[j].basis[0][0] = 1.0f;
    E[j].basis[1][1] = 1.0f;
    E[j].basis[2][2] = 1.0f;
    E[j].scale[0]    = 1.0f;
    E[j].scale[1]    = 1.0f;
    E[j].scale[2]    = 1.0f;

    E[j].alpha       = 1.0f;

    attach_entity(j, 0);
}

int send_create_entity(int type, int data)
{
    int j;

    if (E && (j = alloc_entity()) >= 0)
    {
        pack_index(j);
        pack_index(type);
        pack_index(data);
    
        create_entity(j, type, data);

        return j;
    }
    return -1;
}

void recv_create_entity(void)
{
    int j   = unpack_index();
    int type = unpack_index();
    int data = unpack_index();

    create_entity(j, type, data);
}

/*---------------------------------------------------------------------------*/

void send_parent_entity(int cd, int pd)
{
    pack_event(EVENT_PARENT_ENTITY);
    pack_index(cd);
    pack_index(pd);

    detach_entity(cd, E[cd].par);
    attach_entity(cd, pd);
}

void recv_parent_entity(void)
{
    int cd = unpack_index();
    int pd = unpack_index();

    detach_entity(cd, E[cd].par);
    attach_entity(cd, pd);
}

/*---------------------------------------------------------------------------*/

void send_set_entity_rotation(int j, const float r[3])
{
    float e[3][3];

    if (entity_exists(j))
    {
        v_basis(e, r);
        send_set_entity_basis(j, e[0], e[1], e[2]);
    }
}

void send_set_entity_position(int j, const float p[3])
{
    pack_event(EVENT_SET_ENTITY_POSITION);
    pack_index(j);

    pack_float((E[j].position[0] = p[0]));
    pack_float((E[j].position[1] = p[1]));
    pack_float((E[j].position[2] = p[2]));
}

void send_set_entity_basis(int j, const float e0[3],
                                   const float e1[3],
                                   const float e2[3])
{
    pack_event(EVENT_SET_ENTITY_BASIS);
    pack_index(j);

    pack_float((E[j].basis[0][0] = e0[0]));
    pack_float((E[j].basis[0][1] = e0[1]));
    pack_float((E[j].basis[0][2] = e0[2]));

    pack_float((E[j].basis[1][0] = e1[0]));
    pack_float((E[j].basis[1][1] = e1[1]));
    pack_float((E[j].basis[1][2] = e1[2]));

    pack_float((E[j].basis[2][0] = e2[0]));
    pack_float((E[j].basis[2][1] = e2[1]));
    pack_float((E[j].basis[2][2] = e2[2]));
}

void send_set_entity_scale(int j, const float v[3])
{
    pack_event(EVENT_SET_ENTITY_SCALE);
    pack_index(j);

    pack_float((E[j].scale[0] = v[0]));
    pack_float((E[j].scale[1] = v[1]));
    pack_float((E[j].scale[2] = v[2]));
}

void send_set_entity_alpha(int j, float a)
{
    pack_event(EVENT_SET_ENTITY_ALPHA);
    pack_index(j);

    pack_float((E[j].alpha = a));
}

void send_set_entity_flag(int j, int flags, int state)
{
    pack_event(EVENT_SET_ENTITY_FLAG);
    pack_index(j);
    pack_index(flags);
    pack_index(state);

    if (state)
        E[j].flag = E[j].flag | (flags);
    else
        E[j].flag = E[j].flag & (~flags);
}

void send_set_entity_frag_prog(int j, const char *txt)
{
    int len = strlen(txt) + 1;

    pack_event(EVENT_SET_ENTITY_FRAG_PROG);
    pack_index(j);
    pack_index(len);
    pack_alloc(len, txt);

    E[j].frag_text = memdup(txt, strlen(txt) + 1, 1);
}

void send_set_entity_vert_prog(int j, const char *txt)
{
    int len = strlen(txt) + 1;

    pack_event(EVENT_SET_ENTITY_VERT_PROG);
    pack_index(j);
    pack_index(len);
    pack_alloc(len, txt);

    E[j].vert_text = memdup(txt, strlen(txt) + 1, 1);
}

/*---------------------------------------------------------------------------*/

void send_move_entity(int j, const float v[3])
{
    float p[3];

    if (entity_exists(j))
    {
        p[0] = E[j].position[0] + (E[j].basis[0][0] * v[0] +
                                    E[j].basis[1][0] * v[1] +
                                    E[j].basis[2][0] * v[2]);
        p[1] = E[j].position[1] + (E[j].basis[0][1] * v[0] +
                                    E[j].basis[1][1] * v[1] +
                                    E[j].basis[2][1] * v[2]);
        p[2] = E[j].position[2] + (E[j].basis[0][2] * v[0] +
                                    E[j].basis[1][2] * v[1] +
                                    E[j].basis[2][2] * v[2]);

        send_set_entity_position(j, p);
    }
}

void send_turn_entity(int j, const float r[3])
{
    float M[16], A[16], B[16], e[3][3];

    if (entity_exists(j))
    {
        /* Compose a transformation matrix. */

        m_init(M);

        m_rotat(A, B, E[j].basis[0], r[0]);
        m_mult(M, M, A);

        m_rotat(A, B, E[j].basis[1], r[1]);
        m_mult(M, M, A);

        m_rotat(A, B, E[j].basis[2], r[2]);
        m_mult(M, M, A);

        /* Transform the basis. */

        m_xfrm(e[0], M, E[j].basis[0]);
        m_xfrm(e[1], M, E[j].basis[1]);
        m_xfrm(e[2], M, E[j].basis[2]);

        /* Re-orthogonalize the basis. */

        v_cross(e[2], e[0], e[1]);
        v_cross(e[1], e[2], e[0]);
        v_cross(e[0], e[1], e[2]);

        /* Re-normalize the basis. */

        v_normal(e[0], e[0]);
        v_normal(e[1], e[1]);
        v_normal(e[2], e[2]);

        send_set_entity_basis(j, e[0], e[1], e[2]);
    }
}

/*---------------------------------------------------------------------------*/

void recv_set_entity_position(void)
{
    int j = unpack_index();

    E[j].position[0] = unpack_float();
    E[j].position[1] = unpack_float();
    E[j].position[2] = unpack_float();
}

void recv_set_entity_basis(void)
{
    int j = unpack_index();

    E[j].basis[0][0] = unpack_float();
    E[j].basis[0][1] = unpack_float();
    E[j].basis[0][2] = unpack_float();

    E[j].basis[1][0] = unpack_float();
    E[j].basis[1][1] = unpack_float();
    E[j].basis[1][2] = unpack_float();

    E[j].basis[2][0] = unpack_float();
    E[j].basis[2][1] = unpack_float();
    E[j].basis[2][2] = unpack_float();
}

void recv_set_entity_scale(void)
{
    int j = unpack_index();

    E[j].scale[0] = unpack_float();
    E[j].scale[1] = unpack_float();
    E[j].scale[2] = unpack_float();
}

void recv_set_entity_alpha(void)
{
    int j = unpack_index();

    E[j].alpha = unpack_float();
}

void recv_set_entity_flag(void)
{
    int j    = unpack_index();
    int flags = unpack_index();
    int state = unpack_index();

    if (state)
        E[j].flag = E[j].flag | (flags);
    else
        E[j].flag = E[j].flag & (~flags);
}

void recv_set_entity_frag_prog(void)
{
    int         j  = unpack_index();
    int         len = unpack_index();
    const char *txt = unpack_alloc(len);

    E[j].frag_text = memdup(txt, strlen(txt) + 1, 1);
}

void recv_set_entity_vert_prog(void)
{
    int         j  = unpack_index();
    int         len = unpack_index();
    const char *txt = unpack_alloc(len);

    E[j].vert_text = memdup(txt, strlen(txt) + 1, 1);
}

/*---------------------------------------------------------------------------*/

void get_entity_position(int j, float p[3])
{
    if (entity_exists(j))
    {
        p[0] = E[j].position[0];
        p[1] = E[j].position[1];
        p[2] = E[j].position[2];
    }
}

void get_entity_x_vector(int j, float v[3])
{
    if (entity_exists(j))
    {
        v[0] = E[j].basis[0][0];
        v[1] = E[j].basis[0][1];
        v[2] = E[j].basis[0][2];
    }
}

void get_entity_y_vector(int j, float v[3])
{
    if (entity_exists(j))
    {
        v[0] = E[j].basis[1][0];
        v[1] = E[j].basis[1][1];
        v[2] = E[j].basis[1][2];
    }
}

void get_entity_z_vector(int j, float v[3])
{
    if (entity_exists(j))
    {
        v[0] = E[j].basis[2][0];
        v[1] = E[j].basis[2][1];
        v[2] = E[j].basis[2][2];
    }
}

void get_entity_scale(int j, float v[3])
{
    if (entity_exists(j))
    {
        v[0] = E[j].scale[0];
        v[1] = E[j].scale[1];
        v[2] = E[j].scale[2];
    }
}

float get_entity_alpha(int j)
{
    if (entity_exists(j))
        return E[j].alpha;
    else
        return 0.0f;
}

/*---------------------------------------------------------------------------*/

static void create_clone(int j, int jd)
{
    switch (E[j].type)
    {
    case TYPE_CAMERA: clone_camera(E[j].data); break;
    case TYPE_SPRITE: clone_sprite(E[j].data); break;
    case TYPE_OBJECT: clone_object(E[j].data); break;
    case TYPE_GALAXY: clone_galaxy(E[j].data); break;
    case TYPE_LIGHT:  clone_light (E[j].data); break;
    }

    create_entity(jd, E[j].type, E[j].data);
}

int send_create_clone(int j)
{
    int jd;

    if ((jd = alloc_entity()) >= 0)
    {
        pack_event(EVENT_CREATE_CLONE);
        pack_index(j);
        pack_index(jd);

        create_clone(j, jd);
    }
    return jd;
}

void recv_create_clone(void)
{
    int j = unpack_index();
    int jd = unpack_index();

    create_clone(j, jd);
}

/*---------------------------------------------------------------------------*/

static void delete_entity(int j)
{
    if (entity_exists(j))
    {
        int pd = E[j].par, data = E[j].data;

        /* Delete all child entities. */

        while (E[j].car)
            delete_entity(E[j].car);

        /* Remove this entity from the parent's child list. */

        detach_entity(j, pd);

        /* Release any program objects. */

        if (GL_has_fragment_program)
            if (glIsProgramARB(E[j].frag_prog))
                glDeleteProgramsARB(1, &E[j].frag_prog);
                
        if (GL_has_vertex_program)
            if (glIsProgramARB(E[j].vert_prog))
                glDeleteProgramsARB(1, &E[j].vert_prog);

        /* Invoke the data delete handler. */

        switch (E[j].type)
        {
        case TYPE_CAMERA: delete_camera(data); break;
        case TYPE_SPRITE: delete_sprite(data); break;
        case TYPE_OBJECT: delete_object(data); break;
        case TYPE_GALAXY: delete_galaxy(data); break;
        case TYPE_LIGHT:  delete_light (data); break;
        }

        /* Pave it. */

        if (j) memset(E + j, 0, sizeof (struct entity));
    }
}

void send_delete_entity(int j)
{
    pack_event(EVENT_DELETE_ENTITY);
    pack_index(j);

    delete_entity(j);
}

void recv_delete_entity(void)
{
    delete_entity(unpack_index());
}

/*---------------------------------------------------------------------------*/

int get_entity_parent(int j)
{
    return get_entity(j)->par;
}

int get_entity_child(int j, int n)
{
    int m;
    int k;

    for (m = 0, k = get_entity(j)->car; k; m++, k = get_entity(k)->cdr)
        if (n == m)
            return k;

    return -1;
}

/*---------------------------------------------------------------------------*/

