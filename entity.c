/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    TOTAL PERSPECTIVE VORTEX is free software;  you can redistribute it    */
/*    and/or modify it under the terms of the  GNU General Public License    */
/*    as published by the  Free Software Foundation;  either version 2 of    */
/*    the License, or (at your option) any later version.                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "opengl.h"
#include "shared.h"
#include "server.h"
#include "sprite.h"
#include "entity.h"

/*---------------------------------------------------------------------------*/

static struct entity *E     = NULL;
static int            E_max =   64;

/*---------------------------------------------------------------------------*/

int entity_exists(int id)
{
    return (E && ((id == 0) || (0 < id && id < E_max && E[id].type)));
}

int entity_istype(int id, int type)
{
    return entity_exists(id) && (E[id].type == type);
}

/*---------------------------------------------------------------------------*/

static int unused_entity(void)
{
    int id = 0;

    if (E)
        for (id = 1; id < E_max; ++id)
            if (E[id].type == 0)
                break;

    return id;
}

static int expand_entity(void)
{
    struct entity *F;

    if (E == NULL)
    {
        if ((E = (struct entity *) calloc(E_max, sizeof (struct entity))))
            return 1;
    }

    if ((F = (struct entity *) realloc(E, E_max * 2 * sizeof (struct entity))))
    {
        memset(F + E_max, 0, E_max * sizeof (struct entity));
        E_max *= 2;
        E      = F;

        return 1;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static void entity_transform(int id)
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

    if (fabs(E[id].rotation[0] > 0.0))
        glRotatef(E[id].rotation[0], 1.0f, 0.0f, 0.0f);

    if (fabs(E[id].rotation[1] > 0.0))
        glRotatef(E[id].rotation[1], 0.0f, 1.0f, 0.0f);

    if (fabs(E[id].rotation[2] > 0.0))
        glRotatef(E[id].rotation[2], 0.0f, 0.0f, 1.0f);
}

static void entity_traverse(int id)
{
    int jd;

    glPushMatrix();
    {
        entity_transform(id);

        /* Invoke the entity render function. */

        switch (E[id].type)
        {
        case TYPE_SPRITE: sprite_render(E[id].data); break;
        }

        /* Render any child entities. */

        for (jd = E[id].car; jd; jd = E[jd].cdr)
            entity_traverse(jd);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by object creation functions. */

int entity_create(int type, int data)
{
    int id;

    /* If the existing buffer still has room... */

    if ((id = unused_entity()))
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

        return id;
    }

    /* If the buffer is full, double its size.  Retry. */

    if (expand_entity())
        return entity_create(type, data);

    /* If the buffer cannot be doubled, fail. */

    return -1;
}

void entity_parent(int cd, int pd)
{
    int id;
    int jd;
    int od;

    /* Trigger the parent operation and sync the arguments. */

    if (mpi_isroot())
    {
        server_send(EVENT_ENTITY_PARENT);

        mpi_share_integer(1, &cd);
        mpi_share_integer(1, &pd);
    }

    if (entity_exists(pd) && entity_exists(cd))
    {
        od = E[cd].par;

        /* Remove the child from its previous parent's child list. */

        for (jd = 0, id = E[od].car; id; jd = id, id = E[id].cdr)
            if (id == cd)
            {
                if (jd)
                    E[jd].cdr = E[id].cdr;
                else
                    E[od].car = E[id].cdr;
            }

        /* Insert the child into its new parent's child list. */

        E[cd].par = pd;
        E[cd].cdr = E[pd].car;
        E[pd].car = cd;
    }
}

void entity_delete(int id)
{
    /* Trigger the delete operation and share the descriptor. */

    if (mpi_isroot())
        server_send(EVENT_ENTITY_DELETE);

    mpi_share_integer(1, &id);

    /* Invoke the data delete handler. */

    switch (E[id].type)
    {
    case TYPE_SPRITE: sprite_delete(E[id].data); break;
    }

    memset(E + id, 0, sizeof (struct entity));
}

void entity_render(void)
{
    if (E) entity_traverse(0);
}

/*---------------------------------------------------------------------------*/

void entity_position(int id, float x, float y, float z)
{
    if (entity_exists(id))
    {
        if (mpi_isroot())
        {
            E[id].position[0] = x;
            E[id].position[1] = y;
            E[id].position[2] = z;

            server_send(EVENT_ENTITY_MOVE);
        }

        mpi_share_integer(1, &id);
        mpi_share_float(3, E[id].position);
    }
}

void entity_rotation(int id, float x, float y, float z)
{
    if (entity_exists(id))
    {
        if (mpi_isroot())
        {
            E[id].rotation[0] = x;
            E[id].rotation[1] = y;
            E[id].rotation[2] = z;

            server_send(EVENT_ENTITY_TURN);
        }

        mpi_share_integer(1, &id);
        mpi_share_float(3, E[id].rotation);
    }
}

void entity_scale(int id, float x, float y, float z)
{
    if (entity_exists(id))
    {
        if (mpi_isroot())
        {
            E[id].scale[0] = x;
            E[id].scale[1] = y;
            E[id].scale[2] = z;

            server_send(EVENT_ENTITY_SIZE);
        }

        mpi_share_integer(1, &id);
        mpi_share_float(3, E[id].scale);
    }
}

/*---------------------------------------------------------------------------*/

