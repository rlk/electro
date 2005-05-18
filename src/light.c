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
#include "vector.h"
#include "buffer.h"
#include "entity.h"
#include "event.h"
#include "utility.h"
#include "light.h"

/*---------------------------------------------------------------------------*/

struct light
{
    int   count;
    int   type;
    float d[4];
};

static vector_t light;

/*---------------------------------------------------------------------------*/

#define L(i) ((struct light *) vecget(light, i))

static int new_light(void)
{
    int i, n = vecnum(light);

    for (i = 0; i < n; ++i)
        if (L(i)->count == 0)
            return i;

    return vecadd(light);
}

int startup_light(void)
{
    if ((light = vecnew(8, sizeof (struct light))))
        return 1;
    else
        return 0;
}

/*===========================================================================*/

int send_create_light(int type)
{
    int i;

    if ((i = new_light()) >= 0)
    {
        pack_event(EVENT_CREATE_LIGHT);
        pack_index(i);
        pack_index(type);

        L(i)->count = 1;
        L(i)->type  = type;
        L(i)->d[0]  = 1.0f;
        L(i)->d[1]  = 1.0f;
        L(i)->d[2]  = 1.0f;
        L(i)->d[3]  = 1.0f;

        return send_create_entity(TYPE_LIGHT, i);
    }
    return -1;
}

void recv_create_light(void)
{
    int i = unpack_index();

    L(i)->count = 1;
    L(i)->type  = unpack_index();
    L(i)->d[0]  = 1.0f;
    L(i)->d[1]  = 1.0f;
    L(i)->d[2]  = 1.0f;
    L(i)->d[3]  = 1.0f;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_light_color(int i, float r, float g, float b)
{
    pack_event(EVENT_SET_LIGHT_COLOR);
    pack_index(i);

    pack_float((L(i)->d[0] = r));
    pack_float((L(i)->d[1] = g));
    pack_float((L(i)->d[2] = b));
}

void recv_set_light_color(void)
{
    int i = unpack_index();

    L(i)->d[0] = unpack_float();
    L(i)->d[1] = unpack_float();
    L(i)->d[2] = unpack_float();
}

/*===========================================================================*/

static void draw_light(int j, int i, const float M[16],
                                     const float I[16],
                                     const struct frustum *F, float a)
{
    glPushAttrib(GL_ENABLE_BIT);
    glPushMatrix();
    {
        float N[16];
        float J[16];
        float p[4];

        GLenum o = GL_LIGHT0 + i;

        transform_entity(j, N, M, J, I);

        /* Determine the homogenous coordinate lightsource position. */

        get_entity_position(j, p);

        if (L(i)->type == LIGHT_POSITIONAL)  p[3] = 1.0f;
        if (L(i)->type == LIGHT_DIRECTIONAL) p[3] = 0.0f;

        /* Enable this light and render all child entities. */

        glEnable(GL_LIGHTING);
        glEnable(o);
        
        glLightfv(o, GL_DIFFUSE,  L(i)->d);
        glLightfv(o, GL_POSITION, p);

        draw_entity_list(j, N, J, F, a * get_entity_alpha(j));
    }
    glPopMatrix();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static void dupe_light(int i)
{
    L(i)->count++;
}

static void free_light(int i)
{
    if (--L(i)->count == 0)
        memset(L(i), 0, sizeof (struct light));
}

/*===========================================================================*/

struct entity_func light_func = {
    NULL,
    NULL,
    draw_light,
    dupe_light,
    free_light,
};
