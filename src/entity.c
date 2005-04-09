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
#include "entity.h"

/*---------------------------------------------------------------------------*/
/* Base entity storage                                                       */

#define EMAXINIT 256

static struct entity *E;
static int            E_max;

int entity_exists(int id)
{
    return (E && 0 <= id && id < E_max && E[id].type);
}

static int alloc_entity(void)
{
    int id = -1;

    E = (struct entity *) balloc(E, &id, &E_max,
                                 sizeof (struct entity), entity_exists);
    return id;
}

/*---------------------------------------------------------------------------*/

const char *get_entity_type_name(int id)
{
    if (entity_exists(id))
        switch (E[id].type)
        {
        case TYPE_ROOT:   return "root";
        case TYPE_CAMERA: return "camera";
        case TYPE_SPRITE: return "sprite";
        case TYPE_OBJECT: return "object";
        case TYPE_GALAXY: return "galaxy";
        case TYPE_LIGHT:  return "light";
        case TYPE_PIVOT:  return "pivot";
        }

    return "UNKNOWN";
}

const char *get_entity_debug_id(int id)
{
    static char str[64];

    sprintf(str, "%d %s[%d] (%d)", id, get_entity_type_name(id),
            E[id].data, E[id].par);

    return str;
}

/*---------------------------------------------------------------------------*/

int entity_data(int id)
{
    return entity_exists(id) ? E[id].data : -1;
}

int entity_type(int id)
{
    return entity_exists(id) ? E[id].type : -1;
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

void transform_entity(int id, struct frustum *F1,
                        const struct frustum *F0, const float d[3])
{
    float M[16];
    float I[16];
    float v[3];

    m_init(M);
    m_init(I);

    v[0] = E[id].position[0];
    v[1] = E[id].position[1];
    v[2] = E[id].position[2];

    if (E[id].type == TYPE_CAMERA)
    {
        v[0] += d[0];
        v[1] += d[1];
        v[2] += d[2];

        /* Camera rotation. */

        basis_invt(E[id].basis);
        m_basis(I, M, E[id].basis[0],
                      E[id].basis[1],
                      E[id].basis[2]);

        /* Camera position. */

        glTranslatef(-E[id].position[0] - d[0],
                     -E[id].position[1] - d[1],
                     -E[id].position[2] - d[2]);
        m_trans(M, I, v);
    }
    else
    {
        /* Entity position. */

        glTranslatef(E[id].position[0],
                     E[id].position[1],
                     E[id].position[2]);
        m_trans(M, I, E[id].position);

        /* Entity rotation. */

        basis_mult(E[id].basis);
        m_basis(M, I, E[id].basis[0],
                      E[id].basis[1],
                      E[id].basis[2]);
    }

    /* Billboard. */

    if (E[id].flag & FLAG_BILLBOARD)
    {
        float A[16];
        float B[16];

        glGetFloatv(GL_MODELVIEW_MATRIX, A);

        A[0] = 1.f;  A[4] = 0.f;  A[8]  = 0.f;
        A[1] = 0.f;  A[5] = 1.f;  A[9]  = 0.f;
        A[2] = 0.f;  A[6] = 0.f;  A[10] = 1.f;

        glLoadMatrixf(A);

        m_xpos(B, A);
        m_mult(M, M, A);
        m_mult(I, B, I);
    }

    /* Scale. */

    glScalef(E[id].scale[0],
             E[id].scale[1],
             E[id].scale[2]);
    m_scale(M, I, E[id].scale);

    /* Transform the view frustum. */

    m_pfrm(F1->V[0], M, F0->V[0]);
    m_pfrm(F1->V[1], M, F0->V[1]);
    m_pfrm(F1->V[2], M, F0->V[2]);
    m_pfrm(F1->V[3], M, F0->V[3]);
}

void draw_entity_list(int id, const struct frustum *F0, float a)
{
    int jd;

    /* Traverse the hierarchy.  Iterate the child list of this entity. */

    for (jd = E[id].car; jd; jd = E[jd].cdr)
        if ((E[jd].flag & FLAG_HIDDEN) == 0)
        {
            glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
            {
                /* Enable wireframe if specified. */

                if (E[jd].flag & FLAG_WIREFRAME)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                /* Disable lighting if requested. */

                if (E[jd].flag & FLAG_UNLIT)
                    glDisable(GL_LIGHTING);

                /* Enable line smoothing if requested. */

                if (E[jd].flag & FLAG_LINE_SMOOTH)
                    glEnable(GL_LINE_SMOOTH);

                /* Enable vertex and fragment programs if specified. */

                if (E[jd].frag_prog && GL_has_fragment_program)
                {
                    glEnable(GL_FRAGMENT_PROGRAM_ARB);
                    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, E[jd].frag_prog);
                }
                if (E[jd].vert_prog && GL_has_vertex_program)
                {
                    glEnable(GL_VERTEX_PROGRAM_ARB);
                    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
                    glBindProgramARB(GL_VERTEX_PROGRAM_ARB,   E[jd].vert_prog);
                }

                /* Draw this entity. */

                switch (E[jd].type)
                {
                case TYPE_CAMERA: draw_camera(jd, E[jd].data, F0, a); break;
                case TYPE_SPRITE: draw_sprite(jd, E[jd].data, F0, a); break;
                case TYPE_OBJECT: draw_object(jd, E[jd].data, F0, a); break;
                case TYPE_GALAXY: draw_galaxy(jd, E[jd].data, F0, a); break;
                case TYPE_LIGHT:  draw_light (jd, E[jd].data, F0, a); break;
                case TYPE_PIVOT:  draw_pivot (jd, E[jd].data, F0, a); break;
                }
            }
            glPopAttrib();

            /* Protect the space-time continuum. */

            opengl_check(get_entity_debug_id(jd));
        }
}

/*---------------------------------------------------------------------------*/

int init_entity(void)
{
    if ((E = (struct entity *) calloc(EMAXINIT, sizeof (struct entity))))
    {
        E_max     = EMAXINIT;
        E[0].type = TYPE_ROOT;

        init_camera();
        init_galaxy();
        init_sprite();
        init_object();
        init_light();

        return 1;
    }
    return 0;
}

void draw_entity(void)
{
    struct frustum F;

    memset(&F, 0, sizeof (struct frustum));

    if (E) draw_entity_list(0, &F, 1.0f);
}

/*---------------------------------------------------------------------------*/

void detach_entity(int cd, int pd)
{
    /* Never allow the root entity to be used as a child. */

    if (cd && entity_exists(pd) && entity_exists(cd))
    {
        int id;
        int jd;
        int od = E[cd].par;

        /* Remove the child from its parent's child list. */

        for (jd = 0, id = E[od].car; id; jd = id, id = E[id].cdr)
            if (id == cd)
            {
                if (jd)
                    E[jd].cdr = E[id].cdr;
                else
                    E[od].car = E[id].cdr;
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

static void create_entity(int id, int type, int data)
{
    E[id].type = type;
    E[id].data = data;
    E[id].flag = 0;

    E[id].basis[0][0] = 1.0f;
    E[id].basis[1][1] = 1.0f;
    E[id].basis[2][2] = 1.0f;
    E[id].scale[0]    = 1.0f;
    E[id].scale[1]    = 1.0f;
    E[id].scale[2]    = 1.0f;

    E[id].alpha       = 1.0f;

    attach_entity(id, 0);
}

int send_create_entity(int type, int data)
{
    int id;

    if (E && (id = alloc_entity()) >= 0)
    {
        pack_index(id);
        pack_index(type);
        pack_index(data);
    
        create_entity(id, type, data);

        return id;
    }
    return -1;
}

void recv_create_entity(void)
{
    int id   = unpack_index();
    int type = unpack_index();
    int data = unpack_index();

    create_entity(id, type, data);
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

static void set_entity_frag_prog(int id, const char *txt)
{
    if (GL_has_fragment_program)
    {
        int len = strlen(txt);

        glGenProgramsARB(1, &E[id].frag_prog);
        glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, E[id].frag_prog);

        glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
                           GL_PROGRAM_FORMAT_ASCII_ARB, len, txt);
    }
    else E[id].frag_prog = 0;
}

static void set_entity_vert_prog(int id, const char *txt)
{
    if (GL_has_vertex_program)
    {
        int len = strlen(txt);

        glGenProgramsARB(1, &E[id].vert_prog);
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB, E[id].vert_prog);

        glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                           GL_PROGRAM_FORMAT_ASCII_ARB, len, txt);
    }
    else E[id].vert_prog = 0;
}

/*---------------------------------------------------------------------------*/

void send_set_entity_rotation(int id, const float r[3])
{
    float M[16], I[16], e[3][3];

    float f[3][3] = {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f }
    };

    if (entity_exists(id))
    {
        /* Compose a transformation matrix. */

        m_init(M);
        m_init(I);

        m_xrot(M, I, r[0]);
        m_yrot(M, I, r[1]);
        m_zrot(M, I, r[2]);

        /* Transform the basis. */

        m_xfrm(e[0], M, f[0]);
        m_xfrm(e[1], M, f[1]);
        m_xfrm(e[2], M, f[2]);

        send_set_entity_basis(id, e[0], e[1], e[2]);
    }
}

void send_set_entity_position(int id, const float p[3])
{
    pack_event(EVENT_SET_ENTITY_POSITION);
    pack_index(id);

    pack_float((E[id].position[0] = p[0]));
    pack_float((E[id].position[1] = p[1]));
    pack_float((E[id].position[2] = p[2]));
}

void send_set_entity_basis(int id, const float e0[3],
                                   const float e1[3],
                                   const float e2[3])
{
    pack_event(EVENT_SET_ENTITY_BASIS);
    pack_index(id);

    pack_float((E[id].basis[0][0] = e0[0]));
    pack_float((E[id].basis[0][1] = e0[1]));
    pack_float((E[id].basis[0][2] = e0[2]));

    pack_float((E[id].basis[1][0] = e1[0]));
    pack_float((E[id].basis[1][1] = e1[1]));
    pack_float((E[id].basis[1][2] = e1[2]));

    pack_float((E[id].basis[2][0] = e2[0]));
    pack_float((E[id].basis[2][1] = e2[1]));
    pack_float((E[id].basis[2][2] = e2[2]));
}

void send_set_entity_scale(int id, const float v[3])
{
    pack_event(EVENT_SET_ENTITY_SCALE);
    pack_index(id);

    pack_float((E[id].scale[0] = v[0]));
    pack_float((E[id].scale[1] = v[1]));
    pack_float((E[id].scale[2] = v[2]));
}

void send_set_entity_alpha(int id, float a)
{
    pack_event(EVENT_SET_ENTITY_ALPHA);
    pack_index(id);

    pack_float((E[id].alpha = a));
}

void send_set_entity_flag(int id, int flags, int state)
{
    pack_event(EVENT_SET_ENTITY_FLAG);
    pack_index(id);
    pack_index(flags);
    pack_index(state);

    if (state)
        E[id].flag = E[id].flag | (flags);
    else
        E[id].flag = E[id].flag & (~flags);
}

void send_set_entity_frag_prog(int id, const char *txt)
{
    int len = strlen(txt) + 1;

    pack_event(EVENT_SET_ENTITY_FRAG_PROG);
    pack_index(id);
    pack_index(len);
    pack_alloc(len, txt);

    set_entity_frag_prog(id, txt);
}

void send_set_entity_vert_prog(int id, const char *txt)
{
    int len = strlen(txt) + 1;

    pack_event(EVENT_SET_ENTITY_VERT_PROG);
    pack_index(id);
    pack_index(len);
    pack_alloc(len, txt);

    set_entity_vert_prog(id, txt);
}

/*---------------------------------------------------------------------------*/

void send_move_entity(int id, const float v[3])
{
    float p[3];

    if (entity_exists(id))
    {
        p[0] = E[id].position[0] + (E[id].basis[0][0] * v[0] +
                                    E[id].basis[1][0] * v[1] +
                                    E[id].basis[2][0] * v[2]);
        p[1] = E[id].position[1] + (E[id].basis[0][1] * v[0] +
                                    E[id].basis[1][1] * v[1] +
                                    E[id].basis[2][1] * v[2]);
        p[2] = E[id].position[2] + (E[id].basis[0][2] * v[0] +
                                    E[id].basis[1][2] * v[1] +
                                    E[id].basis[2][2] * v[2]);

        send_set_entity_position(id, p);
    }
}

void send_turn_entity(int id, const float r[3])
{
    float M[16], I[16], e[3][3];

    if (entity_exists(id))
    {
        /* Compose a transformation matrix. */

        m_init(M);
        m_init(I);

        m_rotat(M, I, E[id].basis[0], r[0]);
        m_rotat(M, I, E[id].basis[1], r[1]);
        m_rotat(M, I, E[id].basis[2], r[2]);

        /* Transform the basis. */

        m_xfrm(e[0], M, E[id].basis[0]);
        m_xfrm(e[1], M, E[id].basis[1]);
        m_xfrm(e[2], M, E[id].basis[2]);

        /* Re-orthogonalize the basis. */

        v_cross(e[2], e[0], e[1]);
        v_cross(e[1], e[2], e[0]);
        v_cross(e[0], e[1], e[2]);

        /* Re-normalize the basis. */

        v_normal(e[0], e[0]);
        v_normal(e[1], e[1]);
        v_normal(e[2], e[2]);

        send_set_entity_basis(id, e[0], e[1], e[2]);
    }
}

/*---------------------------------------------------------------------------*/

void recv_set_entity_position(void)
{
    int id = unpack_index();

    E[id].position[0] = unpack_float();
    E[id].position[1] = unpack_float();
    E[id].position[2] = unpack_float();
}

void recv_set_entity_basis(void)
{
    int id = unpack_index();

    E[id].basis[0][0] = unpack_float();
    E[id].basis[0][1] = unpack_float();
    E[id].basis[0][2] = unpack_float();

    E[id].basis[1][0] = unpack_float();
    E[id].basis[1][1] = unpack_float();
    E[id].basis[1][2] = unpack_float();

    E[id].basis[2][0] = unpack_float();
    E[id].basis[2][1] = unpack_float();
    E[id].basis[2][2] = unpack_float();
}

void recv_set_entity_scale(void)
{
    int id = unpack_index();

    E[id].scale[0] = unpack_float();
    E[id].scale[1] = unpack_float();
    E[id].scale[2] = unpack_float();
}

void recv_set_entity_alpha(void)
{
    int id = unpack_index();

    E[id].alpha = unpack_float();
}

void recv_set_entity_flag(void)
{
    int id    = unpack_index();
    int flags = unpack_index();
    int state = unpack_index();

    if (state)
        E[id].flag = E[id].flag | (flags);
    else
        E[id].flag = E[id].flag & (~flags);
}

void recv_set_entity_frag_prog(void)
{
    int         id  = unpack_index();
    int         len = unpack_index();
    const char *txt = unpack_alloc(len);

    set_entity_frag_prog(id, txt);
}

void recv_set_entity_vert_prog(void)
{
    int         id  = unpack_index();
    int         len = unpack_index();
    const char *txt = unpack_alloc(len);

    set_entity_vert_prog(id, txt);
}

/*---------------------------------------------------------------------------*/

void get_entity_position(int id, float p[3])
{
    if (entity_exists(id))
    {
        p[0] = E[id].position[0];
        p[1] = E[id].position[1];
        p[2] = E[id].position[2];
    }
}

void get_entity_x_vector(int id, float v[3])
{
    if (entity_exists(id))
    {
        v[0] = E[id].basis[0][0];
        v[1] = E[id].basis[0][1];
        v[2] = E[id].basis[0][2];
    }
}

void get_entity_y_vector(int id, float v[3])
{
    if (entity_exists(id))
    {
        v[0] = E[id].basis[1][0];
        v[1] = E[id].basis[1][1];
        v[2] = E[id].basis[1][2];
    }
}

void get_entity_z_vector(int id, float v[3])
{
    if (entity_exists(id))
    {
        v[0] = E[id].basis[2][0];
        v[1] = E[id].basis[2][1];
        v[2] = E[id].basis[2][2];
    }
}

void get_entity_scale(int id, float v[3])
{
    if (entity_exists(id))
    {
        v[0] = E[id].scale[0];
        v[1] = E[id].scale[1];
        v[2] = E[id].scale[2];
    }
}

float get_entity_alpha(int id)
{
    if (entity_exists(id))
        return E[id].alpha;
    else
        return 0.0f;
}

/*---------------------------------------------------------------------------*/

static void create_clone(int id, int jd)
{
    switch (E[id].type)
    {
    case TYPE_CAMERA: clone_camera(E[id].data); break;
    case TYPE_SPRITE: clone_sprite(E[id].data); break;
    case TYPE_OBJECT: clone_object(E[id].data); break;
    case TYPE_GALAXY: clone_galaxy(E[id].data); break;
    case TYPE_LIGHT:  clone_light (E[id].data); break;
    }

    create_entity(jd, E[id].type, E[id].data);
}

int send_create_clone(int id)
{
    int jd;

    if ((jd = alloc_entity()) >= 0)
    {
        pack_event(EVENT_CREATE_CLONE);
        pack_index(id);
        pack_index(jd);

        create_clone(id, jd);
    }
    return jd;
}

void recv_create_clone(void)
{
    int id = unpack_index();
    int jd = unpack_index();

    create_clone(id, jd);
}

/*---------------------------------------------------------------------------*/

static void delete_entity(int id)
{
    if (entity_exists(id))
    {
        int pd = E[id].par, data = E[id].data;

        /* Delete all child entities. */

        while (E[id].car)
            delete_entity(E[id].car);

        /* Remove this entity from the parent's child list. */

        detach_entity(id, pd);

        /* Release any program objects. */

        if (GL_has_fragment_program)
            if (glIsProgramARB(E[id].frag_prog))
                glDeleteProgramsARB(1, &E[id].frag_prog);
                
        if (GL_has_vertex_program)
            if (glIsProgramARB(E[id].vert_prog))
                glDeleteProgramsARB(1, &E[id].vert_prog);

        /* Invoke the data delete handler. */

        switch (E[id].type)
        {
        case TYPE_CAMERA: delete_camera(data); break;
        case TYPE_SPRITE: delete_sprite(data); break;
        case TYPE_OBJECT: delete_object(data); break;
        case TYPE_GALAXY: delete_galaxy(data); break;
        case TYPE_LIGHT:  delete_light (data); break;
        }

        /* Pave it. */

        if (id) memset(E + id, 0, sizeof (struct entity));
    }
}

void send_delete_entity(int id)
{
    pack_event(EVENT_DELETE_ENTITY);
    pack_index(id);

    delete_entity(id);
}

void recv_delete_entity(void)
{
    delete_entity(unpack_index());
}

/*---------------------------------------------------------------------------*/

int get_entity_parent(int id)
{
    if (entity_exists(id))
    {
        return E[id].par;
    }
    return -1;
}

int get_entity_child(int id, int i)
{
    if (entity_exists(id))
    {
        int j, jd;

        for (j = 0, jd = E[id].car; jd; j++, jd = E[jd].cdr)
            if (i == j)
                return jd;
    }
    return -1;
}

/*---------------------------------------------------------------------------*/

