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
#include "shared.h"
#include "server.h"
#include "camera.h"
#include "sprite.h"
#include "object.h"
#include "light.h"
#include "pivot.h"
#include "entity.h"

/*---------------------------------------------------------------------------*/
/* Base entity storage                                                       */

static struct entity *E     = NULL;
static int            E_max =    0;

const char *entity_typename(int type)
{
    switch (type)
    {
    case TYPE_ROOT:   return "root";
    case TYPE_CAMERA: return "camera";
    case TYPE_SPRITE: return "sprite";
    case TYPE_OBJECT: return "object";
    case TYPE_LIGHT:  return "light";
    case TYPE_PIVOT:  return "pivot";
    }

    return "UNKNOWN";
}

int entity_exists(int id)
{
    return (E && 0 <= id && id < E_max && E[id].type);
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

void *buffer_expand(void *buf, int *len, int siz)
{
    void *ptr = NULL;

    /* Reallocate the buffer and initilize the second half. */

    if ((ptr = realloc(buf, *len * siz * 2)))
    {
        memset(ptr + *len, 0, *len * siz);
        *len *= 2;
    }
    else ptr = buf;

    return ptr;
}

/*---------------------------------------------------------------------------*/

void entity_transform(int id)
{
    /* Translation. */

    if (fabs(E[id].position[0]) > 0.0 ||
        fabs(E[id].position[1]) > 0.0 ||
        fabs(E[id].position[2]) > 0.0)
    {
        glTranslatef(E[id].position[0],
                     E[id].position[1],
                     E[id].position[2]);
    }

    /* Scale. */

    if (fabs(E[id].scale[0] - 1.0) > 0.0 ||
        fabs(E[id].scale[1] - 1.0) > 0.0 ||
        fabs(E[id].scale[2] - 1.0) > 0.0)
    {
        glScalef(E[id].scale[0],
                 E[id].scale[1],
                 E[id].scale[2]);
    }

    /* Rotation. */

    if (fabs(E[id].rotation[0]) > 0.0)
        glRotatef(E[id].rotation[0], 1.0f, 0.0f, 0.0f);

    if (fabs(E[id].rotation[1]) > 0.0)
        glRotatef(E[id].rotation[1], 0.0f, 1.0f, 0.0f);

    if (fabs(E[id].rotation[2]) > 0.0)
        glRotatef(E[id].rotation[2], 0.0f, 0.0f, 1.0f);
}

void entity_traversal(int id)
{
    int jd;

    /* Traverse the child list, recursively invoking render functions. */

    for (jd = E[id].car; jd; jd = E[jd].cdr)
        switch (E[jd].type)
        {
        case TYPE_CAMERA: camera_render(jd, E[jd].data); break;
        case TYPE_SPRITE: sprite_render(jd, E[jd].data); break;
        case TYPE_OBJECT: object_render(jd, E[jd].data); break;
        case TYPE_LIGHT:   light_render(jd, E[jd].data); break;
        case TYPE_PIVOT:   pivot_render(jd, E[jd].data); break;
        }
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

int entity_init(void)
{
    if ((E = (struct entity *) calloc(64, sizeof (struct entity))))
    {
        E_max     = 64;
        E[0].type = TYPE_ROOT;

        camera_init();
        sprite_init();
        object_init();
        light_init();

        return 1;
    }
    return 0;
}

/* This function should be called only by object creation functions. */

int entity_create(int type, int data)
{
    int id;

    if (E && (id = buffer_unused(E_max, entity_exists)) >= 0)
    {
        /* Initialize the new entity. */

        if (mpi_isroot())
        {
            E[id].type = type;
            E[id].data = data;
        }

        /* Syncronize the new entity. */

        mpi_share_integer(1, &id);
        mpi_share_integer(1, &E[id].type);
        mpi_share_integer(1, &E[id].data);

        E[id].scale[0] = 1.0f;
        E[id].scale[1] = 1.0f;
        E[id].scale[2] = 1.0f;

        /* Insert the new entity into the root child list. */

        entity_attach(id, 0);

        return id;
    }
    else if ((E = buffer_expand(E, &E_max, sizeof (struct entity))))
        return entity_create(type, data);

    return -1;
}

void entity_parent(int cd, int pd)
{
    /* Trigger the parent operation and sync the arguments. */

    if (mpi_isroot())
        server_send(EVENT_ENTITY_PARENT);

    mpi_share_integer(1, &cd);
    mpi_share_integer(1, &pd);

    entity_detach(cd, E[cd].par);
    entity_attach(cd, pd);
}

void entity_render(void)
{
    if (E) entity_traversal(0);
}

void entity_delete(int id)
{
    int pd = E[id].par;

    /* Trigger the delete operation and share the descriptor. */

    if (mpi_isroot())
        server_send(EVENT_ENTITY_DELETE);

    mpi_share_integer(1, &id);

    /* Let the parent inherit any children. */

    while (E[id].car)
        entity_attach(E[id].car, pd);

    /* Remove the entity from the parent's child list. */

    entity_detach(id, pd);

    /* Invoke the data delete handler. */

    switch (E[id].type)
    {
    case TYPE_CAMERA: camera_delete(E[id].data); break;
    case TYPE_SPRITE: sprite_delete(E[id].data); break;
    case TYPE_OBJECT: object_delete(E[id].data); break;
    case TYPE_LIGHT:   light_delete(E[id].data); break;
    }

    memset(E + id, 0, sizeof (struct entity));
}

/*---------------------------------------------------------------------------*/

void entity_position(int id, float x, float y, float z)
{
    if (mpi_isroot())
    {
        E[id].position[0] = x;
        E[id].position[1] = y;
        E[id].position[2] = z;

        server_send(EVENT_ENTITY_MOVE);

        mpi_share_integer(1, &id);
        mpi_share_float(3, E[id].position);
    }
    else
    {
        mpi_share_integer(1, &id);
        mpi_share_float(3, E[id].position);
    }
}

void entity_rotation(int id, float x, float y, float z)
{
    if (mpi_isroot())
    {
        E[id].rotation[0] = x;
        E[id].rotation[1] = y;
        E[id].rotation[2] = z;

        server_send(EVENT_ENTITY_TURN);

        mpi_share_integer(1, &id);
        mpi_share_float(3, E[id].rotation);
    }
    else
    {
        mpi_share_integer(1, &id);
        mpi_share_float(3, E[id].rotation);
    }
}

void entity_scale(int id, float x, float y, float z)
{
    if (mpi_isroot())
    {
        E[id].scale[0] = x;
        E[id].scale[1] = y;
        E[id].scale[2] = z;

        server_send(EVENT_ENTITY_SIZE);

        mpi_share_integer(1, &id);
        mpi_share_float(3, E[id].scale);
    }
    else
    {
        mpi_share_integer(1, &id);
        mpi_share_float(3, E[id].scale);
    }
}

/*---------------------------------------------------------------------------*/

void entity_get_position(int id, float *x, float *y, float *z)
{
    if (entity_exists(id))
    {
        *x = E[id].position[0];
        *y = E[id].position[1];
        *z = E[id].position[2];
    }
}

void entity_get_rotation(int id, float *x, float *y, float *z)
{
    if (entity_exists(id))
    {
        *x = E[id].rotation[0];
        *y = E[id].rotation[1];
        *z = E[id].rotation[2];
    }
}

void entity_get_scale(int id, float *x, float *y, float *z)
{
    if (entity_exists(id))
    {
        *x = E[id].scale[0];
        *y = E[id].scale[1];
        *z = E[id].scale[2];
    }
}

/*---------------------------------------------------------------------------*/

