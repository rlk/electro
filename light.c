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

static int light_exists(int ld)
{
    return (L && ((ld == 0) || (0 < ld && ld < L_max && L[ld].type)));
}

/*---------------------------------------------------------------------------*/

int light_create(int type)
{
    int ld;

    if (L && (ld = buffer_unused(L_max, light_exists)) >= 0)
    {
        /* Initialize the new light. */

        if (mpi_isroot())
        {
            L[ld].type = type;
            server_send(EVENT_LIGHT_CREATE);
        }

        L[ld].d[0] = 1.0f;
        L[ld].d[1] = 1.0f;
        L[ld].d[2] = 1.0f;
        L[ld].d[3] = 1.0f;

        /* Syncronize the new light. */

        mpi_share_integer(1, &ld);
        mpi_share_integer(1, &L[ld].type);

        /* Encapsulate this new light in an entity. */

        return entity_create(TYPE_LIGHT, ld);
    }
    else if ((L = buffer_expand(L, &L_max, sizeof (struct light))))
        return light_create(type);

    return -1;
}

void light_delete(int ld)
{
}

/*---------------------------------------------------------------------------*/

void light_render(int id, int ld)
{
    if (light_exists(ld))
    {
        GLfloat pos[4];

        /* Determine the homogenous coordinate lightsource position. */

        entity_get_position(id, pos + 0, pos + 1, pos + 2);

        if (L[ld].type == LIGHT_POSITIONAL)  pos[3] = 1.0f;
        if (L[ld].type == LIGHT_DIRECTIONAL) pos[3] = 0.0f;

        /* Enable this light and render all child entities. */

        glPushAttrib(GL_ENABLE_BIT);
        {
            GLenum light = GL_LIGHT0 + (ld - 1);

            glEnable(light);
        
            glLightfv(light, GL_DIFFUSE, L[ld].d);
            glLightfv(light, GL_POSITION, pos);

            entity_traversal(id);
        }
        glPopAttrib();
    }
}

/*---------------------------------------------------------------------------*/
