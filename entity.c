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
#include "shared.h"
#include "server.h"
#include "camera.h"
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

int buffer_unused(int max, int (*exists)(int))
{
    int id = -1;

    for (id = 1; id < max; ++id)
        if (!exists(id))
            break;

    return id;
}

void *buffer_expand(void *buf, int *len, int siz)
{
    void *ptr = NULL;

    /* If the buffer does not exist, ... */

    if (buf == NULL)
    {
        /* ... allocate and initialize it, ... */

        if ((ptr = malloc(*len * siz)))
        {
            memset(ptr, 0, *len * siz);
        }
        else ptr = buf;
    }
    else
    {
        /* ... otherwise, reallocate it and initilize the second half. */

        if ((ptr = realloc(buf, *len * siz * 2)))
        {
            memset(ptr + *len, 0, *len * siz);
            *len *= 2;
        }
        else ptr = buf;
    }

    return ptr;
}

/*---------------------------------------------------------------------------*/

static void entity_transform(int id)
{
    if (id)
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
        case TYPE_SPRITE: sprite_render(E[id].data);                 break;
        case TYPE_CAMERA: camera_render(E[id].data, E[id].position,
                                                    E[id].rotation); break;
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
    int id = -1;
    int rd =  0;

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

        E[id].cdr = E[rd].car;
        E[rd].car = id;
        E[id].par = rd;
    }
    else if ((E = buffer_expand(E, &E_max, sizeof (struct entity))))
        id = entity_create(type, data);

    return id;
}

void entity_parent(int cd, int pd)
{
    int id;
    int jd;
    int od;

    /* Trigger the parent operation and sync the arguments. */

    if (mpi_isroot())
        server_send(EVENT_ENTITY_PARENT);

    mpi_share_integer(1, &cd);
    mpi_share_integer(1, &pd);

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
    if (E)
    {
        glPushAttrib(GL_ENABLE_BIT);
        {
            glEnable(GL_TEXTURE_2D);
            entity_traverse(0);
        }
        glPopAttrib();
    }
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

