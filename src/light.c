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

#include "opengl.h"
#include "buffer.h"
#include "entity.h"
#include "event.h"
#include "utility.h"
#include "light.h"

/*---------------------------------------------------------------------------*/
/* Light entity storage                                                     */

#define LMAXINIT 8

static struct light *L;
static int           L_max;

static int light_exists(int ld)
{
    return (L && 0 <= ld && ld < L_max && L[ld].count);
}

static int alloc_light(void)
{
    return balloc((void **) &L, &L_max, sizeof (struct light), light_exists);
}

/*---------------------------------------------------------------------------*/

int init_light(void)
{
    if ((L = (struct light *) calloc(8, sizeof (struct light))))
    {
        L_max = 8;
        return 1;
    }
    return 0;
}

void draw_light(int id, int ld, const float V[16], float a)
{
    float W[16];

    if (light_exists(ld))
    {
        glPushAttrib(GL_ENABLE_BIT);
        glPushMatrix();
        {
            GLenum light = GL_LIGHT0 + ld;
            GLfloat p[4];

            transform_entity(id, W, V);

            /* Determine the homogenous coordinate lightsource position. */

            get_entity_position(id, p + 0, p + 1, p + 2);

            if (L[ld].type == LIGHT_POSITIONAL)  p[3] = 1.0f;
            if (L[ld].type == LIGHT_DIRECTIONAL) p[3] = 0.0f;

            /* Enable this light and render all child entities. */

            glEnable(GL_LIGHTING);
            glEnable(light);
        
            glLightfv(light, GL_DIFFUSE, L[ld].d);
            glLightfv(light, GL_POSITION, p);

            draw_entity_list(id, W, a * get_entity_alpha(id));
        }
        glPopMatrix();
        glPopAttrib();
    }
}

/*---------------------------------------------------------------------------*/

int send_create_light(int type)
{
    int ld;

    if (L && (ld = alloc_light()) >= 0)
    {
        pack_event(EVENT_CREATE_LIGHT);
        pack_index(ld);
        pack_index(type);

        L[ld].count = 1;
        L[ld].type  = type;
        L[ld].d[0]  = 1.0f;
        L[ld].d[1]  = 1.0f;
        L[ld].d[2]  = 1.0f;
        L[ld].d[3]  = 1.0f;

        return send_create_entity(TYPE_LIGHT, ld);
    }
    return -1;
}

void recv_create_light(void)
{
    int ld = unpack_index();

    L[ld].count = 1;
    L[ld].type  = unpack_index();
    L[ld].d[0]  = 1.0f;
    L[ld].d[1]  = 1.0f;
    L[ld].d[2]  = 1.0f;
    L[ld].d[3]  = 1.0f;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_light_color(int ld, float r, float g, float b)
{
    pack_event(EVENT_SET_LIGHT_COLOR);
    pack_index(ld);

    pack_float((L[ld].d[0] = r));
    pack_float((L[ld].d[1] = g));
    pack_float((L[ld].d[2] = b));
}

void recv_set_light_color(void)
{
    int ld = unpack_index();

    L[ld].d[0] = unpack_float();
    L[ld].d[1] = unpack_float();
    L[ld].d[2] = unpack_float();
}

/*---------------------------------------------------------------------------*/
/* These may only be called by create_clone and delete_entity, respectively. */

void clone_light(int ld)
{
    if (light_exists(ld))
        L[ld].count++;
}

void delete_light(int ld)
{
    if (light_exists(ld))
    {
        L[ld].count--;

        if (L[ld].count == 0)
            memset(L + ld, 0, sizeof (struct light));
    }
}

/*---------------------------------------------------------------------------*/
