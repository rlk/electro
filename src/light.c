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

void light_draw(int id, int ld, float P[3], float V[4][4])
{
    float Q[3], W[4][4];

    if (light_exists(ld))
    {
        glPushAttrib(GL_ENABLE_BIT);
        glPushMatrix();
        {
            GLenum light = GL_LIGHT0 + ld;
            GLfloat p[4];

            entity_transform(id, Q, W, P, V);

            /* Determine the homogenous coordinate lightsource position. */

            entity_get_position(id, p + 0, p + 1, p + 2);

            if (L[ld].type == LIGHT_POSITIONAL)  p[3] = 1.0f;
            if (L[ld].type == LIGHT_DIRECTIONAL) p[3] = 0.0f;

            /* Enable this light and render all child entities. */

            glEnable(GL_LIGHTING);
            glEnable(light);
        
            glLightfv(light, GL_DIFFUSE, L[ld].d);
            glLightfv(light, GL_POSITION, p);

            /* Render all child entities in this coordinate system. */

            entity_traversal(id, Q, W);
        }
        glPopMatrix();
        glPopAttrib();
    }
}

/*---------------------------------------------------------------------------*/

int light_send_create(int type)
{
    int ld;

    if ((ld = buffer_unused(L_max, light_exists)) >= 0)
    {
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
    return -1;
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

void light_send_color(int ld, float r, float g, float b)
{
    pack_event(EVENT_LIGHT_COLOR);
    pack_index(ld);

    pack_float((L[ld].d[0] = r));
    pack_float((L[ld].d[1] = g));
    pack_float((L[ld].d[2] = b));
}

void light_recv_color(void)
{
    int ld = unpack_index();

    L[ld].d[0] = unpack_float();
    L[ld].d[1] = unpack_float();
    L[ld].d[2] = unpack_float();
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by the entity delete function. */

void light_delete(int ld)
{
    memset(L + ld, 0, sizeof (struct light));
}

/*---------------------------------------------------------------------------*/
