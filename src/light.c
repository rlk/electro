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

static struct light *get_light(int i)
{
    return (struct light *) vecget(light, i);
}

static int new_light(void)
{
    int i, n = vecnum(light);

    for (i = 0; i < n; ++i)
        if (get_light(i)->count == 0)
            return i;

    return vecadd(light);
}

/*===========================================================================*/

int send_create_light(int type)
{
    int i;

    if ((i = new_light()) >= 0)
    {
        struct light *l = get_light(i);

        send_event(EVENT_CREATE_LIGHT);
        send_index(type);

        l->count = 1;
        l->type  = type;
        l->d[0]  = 1.0f;
        l->d[1]  = 1.0f;
        l->d[2]  = 1.0f;
        l->d[3]  = 1.0f;

        return send_create_entity(TYPE_LIGHT, i);
    }
    return -1;
}

void recv_create_light(void)
{
    struct light *l = get_light(new_light());

    l->count = 1;
    l->type  = recv_index();
    l->d[0]  = 1.0f;
    l->d[1]  = 1.0f;
    l->d[2]  = 1.0f;
    l->d[3]  = 1.0f;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_light_color(int i, float r, float g, float b)
{
    struct light *l = get_light(i);

    send_event(EVENT_SET_LIGHT_COLOR);
    send_index(i);

    send_float((l->d[0] = r));
    send_float((l->d[1] = g));
    send_float((l->d[2] = b));
}

void recv_set_light_color(void)
{
    struct light *l = get_light(recv_index());

    l->d[0] = recv_float();
    l->d[1] = recv_float();
    l->d[2] = recv_float();
}

/*===========================================================================*/

static void draw_light(int j, int i, int f, float a)
{
    struct light *l = get_light(i);

    glPushAttrib(GL_ENABLE_BIT);
    glPushMatrix();
    {
        float p[4];

        GLenum o = GL_LIGHT0 + i;

/*      transform_entity(j); */

        /* Determine the homogenous coordinate lightsource position. */

        get_entity_position(j, p);

        if (l->type == LIGHT_POSITIONAL)  p[3] = 1.0f;
        if (l->type == LIGHT_DIRECTIONAL) p[3] = 0.0f;

        /* Enable this light and render all child entities. */

        glEnable(GL_LIGHTING);
        glEnable(o);
        
        glLightfv(o, GL_DIFFUSE,  l->d);
        glLightfv(o, GL_POSITION, p);

        draw_entity_tree(j, f, a * get_entity_alpha(j));
    }
    glPopMatrix();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static void dupe_light(int i)
{
    get_light(i)->count++;
}

static void free_light(int i)
{
    struct light *l = get_light(i);

    if (l->count > 0)
    {
        l->count--;
    
        if (l->count == 0)
            memset(l, 0, sizeof (struct light));
    }
}

/*===========================================================================*/

static struct entity_func light_func = {
    "light",
    NULL,
    NULL,
    NULL,
    draw_light,
    dupe_light,
    free_light,
};

struct entity_func *startup_light(void)
{
    if ((light = vecnew(MIN_LIGHTS, sizeof (struct light))))
        return &light_func;
    else
        return NULL;
}

