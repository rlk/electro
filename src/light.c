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
#include "buffer.h"
#include "shared.h"
#include "entity.h"
#include "light.h"

/*---------------------------------------------------------------------------*/
/* Light entity storage                                                     */

#define LMAXINIT 8

static struct light *L;
static int           L_max;

static int light_exists(int ld)
{
    return (L && 0 <= ld && ld < L_max && L[ld].type);
}

/*---------------------------------------------------------------------------*/

int light_init(void)
{
    if ((L = (struct light *) calloc(8, sizeof (struct light))))
    {
        L_max = 8;
        return 1;
    }
    return 0;
}

void light_draw(int id, int ld)
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
            GLenum light = GL_LIGHT0 + ld;

            glEnable(GL_LIGHTING);
            glEnable(light);
        
            glLightfv(light, GL_DIFFUSE, L[ld].d);
            glLightfv(light, GL_POSITION, pos);

            opengl_check("light_draw");

            entity_traversal(id);
        }
        glPopAttrib();
    }
}

/*---------------------------------------------------------------------------*/

int light_send_create(int type)
{
    int ld = buffer_unused(L_max, light_exists);

    pack_event(EVENT_LIGHT_CREATE);
    pack_index(ld);
    pack_index(type);

    L[ld].type = type;
    L[ld].d[0] = 1.0f;
    L[ld].d[1] = 1.0f;
    L[ld].d[2] = 1.0f;
    L[ld].d[3] = 1.0f;

    return entity_send_create(TYPE_LIGHT, ld);
}

void light_recv_create(void)
{
    int ld = unpack_index();

    L[ld].type = unpack_index();
    L[ld].d[0] = 1.0f;
    L[ld].d[1] = 1.0f;
    L[ld].d[2] = 1.0f;
    L[ld].d[3] = 1.0f;

    entity_recv_create();
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by the entity delete function. */

void light_delete(int ld)
{
    memset(L + ld, 0, sizeof (struct light));
}

/*---------------------------------------------------------------------------*/
