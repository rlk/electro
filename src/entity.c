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
#include "matrix.h"
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

void entity_transform(int id, float frustum1[16], const float frustum0[16])
{
    float M[16];
    float I[16];

    m_init(M);
    m_init(I);

    /* Translation. */

    if (fabs(E[id].position[0]) > 0.0 ||
        fabs(E[id].position[1]) > 0.0 ||
        fabs(E[id].position[2]) > 0.0)
    {
        glTranslatef(E[id].position[0],
                     E[id].position[1],
                     E[id].position[2]);
        m_trns(M, I, E[id].position[0],
                     E[id].position[1],
                     E[id].position[2]);
    }

    /* Rotation. */

    if (fabs(E[id].rotation[0]) > 0.0)
    {
        glRotatef(E[id].rotation[0], 1.0f, 0.0f, 0.0f);
        m_xrot(M, I, E[id].rotation[0]);
    }

    if (fabs(E[id].rotation[1]) > 0.0)
    {
        glRotatef(E[id].rotation[1], 0.0f, 1.0f, 0.0f);
        m_yrot(M, I, E[id].rotation[1]);
    }

    if (fabs(E[id].rotation[2]) > 0.0)
    {
        glRotatef(E[id].rotation[2], 0.0f, 0.0f, 1.0f);
        m_zrot(M, I, E[id].rotation[2]);
    }

    /* Billboard.  TODO: fix frustum billboard. */

    if (E[id].flag & FLAG_BILLBOARD)
    {
        float M[16];

        glGetFloatv(GL_MODELVIEW_MATRIX, M);

        M[0] = 1.f;  M[4] = 0.f;  M[8]  = 0.f;
        M[1] = 0.f;  M[5] = 1.f;  M[9]  = 0.f;
        M[2] = 0.f;  M[6] = 0.f;  M[10] = 1.f;

        glLoadMatrixf(M);
    }

    /* Scale.  TODO: fix frustum scale. */

    if (fabs(E[id].scale[0] - 1.0) > 0.0 ||
        fabs(E[id].scale[1] - 1.0) > 0.0 ||
        fabs(E[id].scale[2] - 1.0) > 0.0)
    {
        glScalef(E[id].scale[0],
                 E[id].scale[1],
                 E[id].scale[2]);
    }

    /* Transform the view frustum. */

    m_pfrm(frustum1 +  0, M, frustum0 +  0);
    m_pfrm(frustum1 +  4, M, frustum0 +  4);
    m_pfrm(frustum1 +  8, M, frustum0 +  8);
    m_pfrm(frustum1 + 12, M, frustum0 + 12);
}

void entity_traversal(int id, const float V[16])
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
            case TYPE_CAMERA: camera_draw(jd, E[jd].data, V); break;
            case TYPE_SPRITE: sprite_draw(jd, E[jd].data, V); break;
            case TYPE_OBJECT: object_draw(jd, E[jd].data, V); break;
            case TYPE_GALAXY: galaxy_draw(jd, E[jd].data, V); break;
            case TYPE_LIGHT:   light_draw(jd, E[jd].data, V); break;
            case TYPE_PIVOT:   pivot_draw(jd, E[jd].data, V); break;
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
        galaxy_init();
        sprite_init();
        object_init();
        light_init();

        return 1;
    }
    return 0;
}

void entity_draw(void)
{
    float W[16];

    memset(W, 0, 16 * sizeof (float));

    if (E) entity_traversal(0, W);
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

