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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "opengl.h"
#include "buffer.h"
#include "shared.h"
#include "server.h"
#include "camera.h"
#include "sprite.h"
#include "object.h"
#include "galaxy.h"
#include "light.h"
#include "pivot.h"
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

/*---------------------------------------------------------------------------*/

const char *entity_typename(int id)
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

int entity_todata(int id)
{
    return entity_exists(id) ? E[id].data : -1;
}

int entity_istype(int id, int type)
{
    return entity_exists(id) && (E[id].type == type);
}

/*---------------------------------------------------------------------------*/

int buffer_unused(int max, int (*exists)(int))
{
    int id;

    for (id = 0; id < max; ++id)
        if (!exists(id))
            return id;

    return -1;
}

/*---------------------------------------------------------------------------*/
/* Matrix operations                                                         */

static void ident(float A[16])
{
    A[0] = 1.0f; A[4] = 0.0f; A[8]  = 0.0f; A[12] = 0.0f;
    A[1] = 0.0f; A[5] = 1.0f; A[9]  = 0.0f; A[13] = 0.0f;
    A[2] = 0.0f; A[6] = 0.0f; A[10] = 1.0f; A[14] = 0.0f;
    A[3] = 0.0f; A[7] = 0.0f; A[11] = 0.0f; A[15] = 1.0f;
}

static void copy(float A[16], const float B[16])
{
    A[0] = B[0];  A[4] = B[4];  A[8]  = B[8];  A[12] = B[12];
    A[1] = B[1];  A[5] = B[5];  A[9]  = B[9];  A[13] = B[13];
    A[2] = B[2];  A[6] = B[6];  A[10] = B[10]; A[14] = B[14];
    A[3] = B[3];  A[7] = B[7];  A[11] = B[11]; A[15] = B[15];
}

static void xpos(float A[16], const float B[16])
{
    A[0] = B[0];  A[4] = B[1];  A[8]  = B[2];  A[12] = B[3];
    A[1] = B[4];  A[5] = B[5];  A[9]  = B[6];  A[13] = B[7];
    A[2] = B[8];  A[6] = B[9];  A[10] = B[10]; A[14] = B[11];
    A[3] = B[12]; A[7] = B[13]; A[11] = B[14]; A[15] = B[15];
}

static void mult(float A[16], const float B[16], const float C[16])
{
    float T[16];

    T[0]  = B[0] * C[0]  + B[4] * C[1]  + B[8]  * C[2]  + B[12] * C[3];
    T[1]  = B[1] * C[0]  + B[5] * C[1]  + B[9]  * C[2]  + B[13] * C[3];
    T[2]  = B[2] * C[0]  + B[6] * C[1]  + B[10] * C[2]  + B[14] * C[3];
    T[3]  = B[3] * C[0]  + B[7] * C[1]  + B[11] * C[2]  + B[15] * C[3];

    T[4]  = B[0] * C[4]  + B[4] * C[5]  + B[8]  * C[6]  + B[12] * C[7];
    T[5]  = B[1] * C[4]  + B[5] * C[5]  + B[9]  * C[6]  + B[13] * C[7];
    T[6]  = B[2] * C[4]  + B[6] * C[5]  + B[10] * C[6]  + B[14] * C[7];
    T[7]  = B[3] * C[4]  + B[7] * C[5]  + B[11] * C[6]  + B[15] * C[7];

    T[8]  = B[0] * C[8]  + B[4] * C[9]  + B[8]  * C[10] + B[12] * C[11];
    T[9]  = B[1] * C[8]  + B[5] * C[9]  + B[9]  * C[10] + B[13] * C[11];
    T[10] = B[2] * C[8]  + B[6] * C[9]  + B[10] * C[10] + B[14] * C[11];
    T[11] = B[3] * C[8]  + B[7] * C[9]  + B[11] * C[10] + B[15] * C[11];

    T[12] = B[0] * C[12] + B[4] * C[13] + B[8]  * C[14] + B[12] * C[15];
    T[13] = B[1] * C[12] + B[5] * C[13] + B[9]  * C[14] + B[13] * C[15];
    T[14] = B[2] * C[12] + B[6] * C[13] + B[10] * C[14] + B[14] * C[15];
    T[15] = B[3] * C[12] + B[7] * C[13] + B[11] * C[14] + B[15] * C[15];

    copy(A, T);
}

static void xfrm(float v[4], const float A[16], const float u[4])
{
    v[0] = A[0] * u[0] + A[4] * u[1] + A[8]  * u[2] + A[12] * u[3];
    v[1] = A[1] * u[0] + A[5] * u[1] + A[9]  * u[2] + A[13] * u[3];
    v[2] = A[2] * u[0] + A[6] * u[1] + A[10] * u[2] + A[14] * u[3];
    v[3] = A[3] * u[0] + A[7] * u[1] + A[11] * u[2] + A[15] * u[3];
}

static void pfrm(float v[4], const float A[16], const float u[4])
{
    v[0] = A[0]  * u[0] + A[1]  * u[1] + A[2]  * u[2] + A[3]  * u[3];
    v[1] = A[4]  * u[0] + A[5]  * u[1] + A[6]  * u[2] + A[7]  * u[3];
    v[2] = A[8]  * u[0] + A[9]  * u[1] + A[10] * u[2] + A[11] * u[3];
    v[3] = A[12] * u[0] + A[13] * u[1] + A[14] * u[2] + A[15] * u[3];
}

/*---------------------------------------------------------------------------*/
/* Matrix compositors                                                        */

static void trns(float M[16], float I[16], float x, float y, float z)
{
    float A[16];
    float B[16];

    A[0] =  1; A[4] =  0; A[8]  =  0; A[12] =   x;
    A[1] =  0; A[5] =  1; A[9]  =  0; A[13] =   y;
    A[2] =  0; A[6] =  0; A[10] =  1; A[14] =   z;
    A[3] =  0; A[7] =  0; A[11] =  0; A[15] =   1;

    B[0] =  1; B[4] =  0; B[8]  =  0; B[12] =  -x;
    B[1] =  0; B[5] =  1; B[9]  =  0; B[13] =  -y;
    B[2] =  0; B[6] =  0; B[10] =  1; B[14] =  -z;
    B[3] =  0; B[7] =  0; B[11] =  0; B[15] =   1;

    mult(M, M, A);
    mult(I, B, I);
}

static void xrot(float M[16], float I[16], float a)
{
    const float s = (float) sin((double) (a * PI / 180.0f));
    const float c = (float) cos((double) (a * PI / 180.0f));

    float A[16];
    float B[16];

    A[0] =  1; A[4] =  0; A[8]  =  0; A[12] =  0;
    A[1] =  0; A[5] =  c; A[9]  = -s; A[13] =  0;
    A[2] =  0; A[6] =  s; A[10] =  c; A[14] =  0;
    A[3] =  0; A[7] =  0; A[11] =  0; A[15] =  1;

    xpos(B, A);
    mult(M, M, A);
    mult(I, B, I);
}

static void yrot(float M[16], float I[16], float a)
{
    const float s = (float) sin((double) (a * PI / 180.0f));
    const float c = (float) cos((double) (a * PI / 180.0f));

    float A[16];
    float B[16];

    A[0] =  c; A[4] =  0; A[8]  =  s; A[12] =  0;
    A[1] =  0; A[5] =  1; A[9]  =  0; A[13] =  0;
    A[2] = -s; A[6] =  0; A[10] =  c; A[14] =  0;
    A[3] =  0; A[7] =  0; A[11] =  0; A[15] =  1;

    xpos(B, A);
    mult(M, M, A);
    mult(I, B, I);
}

static void zrot(float M[16], float I[16], float a)
{
    const float s = (float) sin((double) (a * PI / 180.0f));
    const float c = (float) cos((double) (a * PI / 180.0f));

    float A[16];
    float B[16];

    A[0] =  c; A[4] = -s; A[8]  =  0; A[12] =  0;
    A[1] =  s; A[5] =  c; A[9]  =  0; A[13] =  0;
    A[2] =  0; A[6] =  0; A[10] =  1; A[14] =  0;
    A[3] =  0; A[7] =  0; A[11] =  0; A[15] =  1;

    xpos(B, A);
    mult(M, M, A);
    mult(I, B, I);
}

/*---------------------------------------------------------------------------*/
    /* Billboard. */
    /*
    if (E[id].flag & FLAG_BILLBOARD)
    {
        float M[16];

        glGetFloatv(GL_MODELVIEW_MATRIX, M);

        M[0] = 1.f;  M[4] = 0.f;  M[8]  = 0.f;
        M[1] = 0.f;  M[5] = 1.f;  M[9]  = 0.f;
        M[2] = 0.f;  M[6] = 0.f;  M[10] = 1.f;

        glLoadMatrixf(M);
    }
    */

void entity_transform(int id, float Q[3], float W[4][4],
                              float P[3], float V[4][4])
{
    float M[16];
    float I[16];

    ident(M);
    ident(I);

    /* Translation. */

    if (fabs(E[id].position[0]) > 0.0 ||
        fabs(E[id].position[1]) > 0.0 ||
        fabs(E[id].position[2]) > 0.0)
    {
        glTranslatef(E[id].position[0],
                     E[id].position[1],
                     E[id].position[2]);
        trns(M, I,  E[id].position[0],
                    E[id].position[1],
                    E[id].position[2]);
    }

    /* Rotation. */

    if (fabs(E[id].rotation[0]) > 0.0)
    {
        glRotatef (E[id].rotation[0], 1.0f, 0.0f, 0.0f);
        xrot(M, I, E[id].rotation[0]);
    }
    if (fabs(E[id].rotation[1]) > 0.0)
    {
        glRotatef (E[id].rotation[1], 0.0f, 1.0f, 0.0f);
        yrot(M, I, E[id].rotation[1]);
    }
    if (fabs(E[id].rotation[2]) > 0.0)
    {
        glRotatef (E[id].rotation[2], 0.0f, 0.0f, 1.0f);
        zrot(M, I, E[id].rotation[2]);
    }

    /* Transform the view point avd view frustum. */

    copy(W[0], V[0]);
    copy(W[1], V[1]);
    copy(W[2], V[2]);
    copy(W[3], V[3]);

    copy(Q, P);

    /*
    pfrm(W[0], M, V[0]);
    pfrm(W[1], M, V[1]);
    pfrm(W[2], M, V[2]);
    pfrm(W[3], M, V[3]);

    xfrm(Q, I, P);
    */
    /* Scale. */
    /*
    if (fabs(E[id].scale[0] - 1.0) > 0.0 ||
        fabs(E[id].scale[1] - 1.0) > 0.0 ||
        fabs(E[id].scale[2] - 1.0) > 0.0)
    {
        glScalef(E[id].scale[0],
                 E[id].scale[1],
                 E[id].scale[2]);
    }
    */
}

void entity_traversal(int id, float P[3], float V[4][4])
{
    int jd;

    /* Traverse the hierarchy.  Iterate the child list of this entity. */

    for (jd = E[id].car; jd; jd = E[jd].cdr)
        if ((E[jd].flag & FLAG_HIDDEN) == 0)
        {
            /* Enable wireframe if specified. */

            if (E[jd].flag & FLAG_WIREFRAME)
            {
                glPushAttrib(GL_POLYGON_BIT);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }

            if (E[jd].flag & FLAG_UNLIT)
            {
                glPushAttrib(GL_ENABLE_BIT);
                glDisable(GL_LIGHTING);
            }

            /* Draw this entity. */

            switch (E[jd].type)
            {
            case TYPE_CAMERA: camera_draw(jd, E[jd].data, P, V); break;
            case TYPE_SPRITE: sprite_draw(jd, E[jd].data, P, V); break;
            case TYPE_OBJECT: object_draw(jd, E[jd].data, P, V); break;
            case TYPE_GALAXY: galaxy_draw(jd, E[jd].data, P, V); break;
            case TYPE_LIGHT:   light_draw(jd, E[jd].data, P, V); break;
            case TYPE_PIVOT:   pivot_draw(jd, E[jd].data, P, V); break;
            }

            /* Revert to previous render modes, as necessary. */

            if (E[jd].flag & FLAG_WIREFRAME)  glPopAttrib();
            if (E[jd].flag & FLAG_UNLIT)      glPopAttrib();
        }
}

/*---------------------------------------------------------------------------*/

int entity_init(void)
{
    if ((E = (struct entity *) calloc(EMAXINIT, sizeof (struct entity))))
    {
        E_max     = EMAXINIT;
        E[0].type = TYPE_ROOT;

        camera_init();
        sprite_init();
        object_init();
        light_init();

        return 1;
    }
    return 0;
}

void entity_draw(void)
{
    float Q[3];
    float W[4][4];

    memset(Q,    0, 3 * sizeof (float));
    memset(W[0], 0, 4 * sizeof (float));
    memset(W[1], 0, 4 * sizeof (float));
    memset(W[2], 0, 4 * sizeof (float));
    memset(W[3], 0, 4 * sizeof (float));

    if (E) entity_traversal(0, Q, W);
}

/*---------------------------------------------------------------------------*/

void entity_detach(int cd, int pd)
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

void entity_attach(int cd, int pd)
{
    /* Never allow the root entity to be used as a child. */

    if (cd && entity_exists(pd) && entity_exists(cd))
    {
        /* Insert the child into the new parent's child list. */

        E[cd].par = pd;
        E[cd].cdr = E[pd].car;
        E[pd].car = cd;
    }
}

/*---------------------------------------------------------------------------*/

static void entity_create(int id, int type, int data)
{
    E[id].type = type;
    E[id].data = data;
    E[id].flag = 0;

    E[id].scale[0] = 1.0f;
    E[id].scale[1] = 1.0f;
    E[id].scale[2] = 1.0f;
    E[id].alpha    = 1.0f;

    entity_attach(id, 0);
}

int entity_send_create(int type, int data)
{
    int id;

    if ((id = buffer_unused(E_max, entity_exists)) >= 0)
    {
        pack_index(id);
        pack_index(type);
        pack_index(data);
    
        entity_create(id, type, data);

        return id;
    }
    return -1;
}

void entity_recv_create(void)
{
    int id   = unpack_index();
    int type = unpack_index();
    int data = unpack_index();

    entity_create(id, type, data);
}

/*---------------------------------------------------------------------------*/

void entity_send_parent(int cd, int pd)
{
    pack_event(EVENT_ENTITY_PARENT);
    pack_index(cd);
    pack_index(pd);

    entity_detach(cd, E[cd].par);
    entity_attach(cd, pd);
}

void entity_recv_parent(void)
{
    int cd = unpack_index();
    int pd = unpack_index();

    entity_detach(cd, E[cd].par);
    entity_attach(cd, pd);
}

/*---------------------------------------------------------------------------*/

static void entity_delete(int id)
{
    if (entity_exists(id))
    {
        int jd, pd = E[id].par, data = E[id].data;

        /* Let the parent inherit any children. */

        while (E[id].car)
            entity_attach(E[id].car, pd);

        /* Remove the entity from the parent's child list. */

        entity_detach(id, pd);

        memset(E + id, 0, sizeof (struct entity));

        /* TODO: Fix.  Search for a clone data reference. */

        for (jd = 0; jd < E_max; ++jd)
            if (E[jd].data == data)
                return;

        /* Invoke the data delete handler. */

        switch (E[id].type)
        {
        case TYPE_CAMERA: camera_delete(data); break;
        case TYPE_SPRITE: sprite_delete(data); break;
        case TYPE_OBJECT: object_delete(data); break;
        case TYPE_GALAXY: galaxy_delete(data); break;
        case TYPE_LIGHT:   light_delete(data); break;
        }
    }
}

void entity_send_delete(int id)
{
    pack_event(EVENT_ENTITY_DELETE);
    pack_index(id);

    entity_delete(id);
}

void entity_recv_delete(void)
{
    entity_delete(unpack_index());
}

/*---------------------------------------------------------------------------*/

int entity_send_clone(int id)
{
    int jd = buffer_unused(E_max, entity_exists);

    pack_event(EVENT_ENTITY_CLONE);
    pack_index(id);
    pack_index(jd);

    entity_create(jd, E[id].type, E[id].data);

    return jd;
}

void entity_recv_clone(void)
{
    int id = unpack_index();
    int jd = unpack_index();

    entity_create(jd, E[id].type, E[id].data);
}

/*---------------------------------------------------------------------------*/

void entity_send_flag(int id, int flags, int state)
{
    pack_event(EVENT_ENTITY_FLAG);
    pack_index(id);
    pack_index(flags);
    pack_index(state);

    if (state)
        E[id].flag = E[id].flag | (flags);
    else
        E[id].flag = E[id].flag & (~flags);
}

void entity_recv_flag(void)
{
    int id    = unpack_index();
    int flags = unpack_index();
    int state = unpack_index();

    if (state)
        E[id].flag = E[id].flag | (flags);
    else
        E[id].flag = E[id].flag & (~flags);
}

/*---------------------------------------------------------------------------*/

void entity_send_position(int id, float x, float y, float z)
{
    pack_event(EVENT_ENTITY_MOVE);
    pack_index(id);

    pack_float((E[id].position[0] = x));
    pack_float((E[id].position[1] = y));
    pack_float((E[id].position[2] = z));
}

void entity_send_rotation(int id, float x, float y, float z)
{
    pack_event(EVENT_ENTITY_TURN);
    pack_index(id);

    pack_float((E[id].rotation[0] = x));
    pack_float((E[id].rotation[1] = y));
    pack_float((E[id].rotation[2] = z));
}

void entity_send_scale(int id, float x, float y, float z)
{
    pack_event(EVENT_ENTITY_SIZE);
    pack_index(id);

    pack_float((E[id].scale[0] = x));
    pack_float((E[id].scale[1] = y));
    pack_float((E[id].scale[2] = z));
}

void entity_send_alpha(int id, float a)
{
    pack_event(EVENT_ENTITY_FADE);
    pack_index(id);

    pack_float((E[id].alpha = a));
}

/*---------------------------------------------------------------------------*/

void entity_recv_position(void)
{
    int id = unpack_index();

    E[id].position[0] = unpack_float();
    E[id].position[1] = unpack_float();
    E[id].position[2] = unpack_float();
}

void entity_recv_rotation(void)
{
    int id = unpack_index();

    E[id].rotation[0] = unpack_float();
    E[id].rotation[1] = unpack_float();
    E[id].rotation[2] = unpack_float();
}

void entity_recv_scale(void)
{
    int id = unpack_index();

    E[id].scale[0] = unpack_float();
    E[id].scale[1] = unpack_float();
    E[id].scale[2] = unpack_float();
}

void entity_recv_alpha(void)
{
    int id = unpack_index();

    E[id].alpha = unpack_float();
}

/*---------------------------------------------------------------------------*/

void entity_get_position(int id, float *x, float *y, float *z)
{
    if (entity_exists(id))
    {
        if (x) *x = E[id].position[0];
        if (y) *y = E[id].position[1];
        if (z) *z = E[id].position[2];
    }
}

void entity_get_rotation(int id, float *x, float *y, float *z)
{
    if (entity_exists(id))
    {
        if (x) *x = E[id].rotation[0];
        if (y) *y = E[id].rotation[1];
        if (z) *z = E[id].rotation[2];
    }
}

void entity_get_scale(int id, float *x, float *y, float *z)
{
    if (entity_exists(id))
    {
        if (x) *x = E[id].scale[0];
        if (y) *y = E[id].scale[1];
        if (z) *z = E[id].scale[2];
    }
}

float entity_get_alpha(int id)
{
    if (entity_exists(id))
        return E[id].alpha;
    else
        return 0.0f;
}

/*---------------------------------------------------------------------------*/

