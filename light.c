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
#include <stdio.h>

#include "opengl.h"
#include "shared.h"
#include "server.h"
#include "entity.h"
#include "light.h"

/*---------------------------------------------------------------------------*/
/* Light entity storage                                                     */

static struct light *L     = NULL;
static int           L_max =    4;

static int light_exists(int id)
{
    return (L && 0 < id && id < L_max && L[id].type);
}

/*---------------------------------------------------------------------------*/

int light_create(int type)
{
    int id = -1;

    if (L && (id = buffer_unused(L_max, light_exists)) >= 0)
    {
        /* Initialize the new light. */

        if (mpi_isroot())
        {
            L[id].type = type;
            L[id].d[0] = 1.0f;
            L[id].d[1] = 1.0f;
            L[id].d[2] = 1.0f;
            L[id].d[3] = 1.0f;
            /*
            server_send(EVENT_LIGHT_CREATE);
            */
        }
        /* Syncronize the new light. */
        /*
        mpi_share_integer(1, &id);
        */
        /* Encapsulate this new light in an entity. */

        id = entity_create(TYPE_LIGHT, id);
    }
    else if ((L = buffer_expand(L, &L_max, sizeof (struct light))))
        id = light_create(type);

    return id;
}

void light_delete(int id)
{
}

/*---------------------------------------------------------------------------*/

void light_render(int id, const float pos[3])
{
    if (light_exists(id))
    {
        GLenum light = GL_LIGHT0 + (id - 1);
        GLfloat p[4];

        glEnable (light);

        if (L[id].type == LIGHT_POSITIONAL)
        {
            p[0] = pos[0];
            p[1] = pos[1];
            p[2] = pos[2];
            p[3] =   1.0f;
        }
        if (L[id].type == LIGHT_DIRECTIONAL)
        {
            p[0] = pos[0];
            p[1] = pos[1];
            p[2] = pos[2];
            p[3] =   0.0f;
        }

        glLightfv(light, GL_DIFFUSE,  L[id].d);
        glLightfv(light, GL_POSITION, p);
    }
}

/*---------------------------------------------------------------------------*/
