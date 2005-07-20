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
#include "frustum.h"
#include "buffer.h"
#include "entity.h"
#include "brush.h"
#include "event.h"
#include "sprite.h"

/*---------------------------------------------------------------------------*/

struct sprite
{
    int   count;
    int   state;
    int   brush;
    float s0;
    float s1;
    float t0;
    float t1;
};

static vector_t sprite;

/*---------------------------------------------------------------------------*/

#define S(i) ((struct sprite *) vecget(sprite, i))

static int new_sprite(void)
{
    int i, n = vecnum(sprite);

    for (i = 0; i < n; ++i)
        if (S(i)->count == 0)
            return i;

    return vecadd(sprite);
}

/*===========================================================================*/

int send_create_sprite(int j)
{
    int i;

    if ((i = new_sprite()) >= 0)
    {
        S(i)->brush = j;
        S(i)->count = 1;
        S(i)->s0 = 0.0f;
        S(i)->t0 = 0.0f;
        S(i)->s1 = 1.0f;
        S(i)->t1 = 1.0f;

        send_event(EVENT_CREATE_SPRITE);
        send_index(S(i)->brush);

        return send_create_entity(TYPE_SPRITE, i);
    }
    return -1;
}

void recv_create_sprite(void)
{
    int i = new_sprite();
    int j = recv_index();

    S(i)->brush = j;
    S(i)->count = 1;
    S(i)->s0 = 0.0f;
    S(i)->t0 = 0.0f;
    S(i)->s1 = 1.0f;
    S(i)->t1 = 1.0f;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_sprite_brush(int i, int j)
{
    struct sprite *s = S(i);

    dupe_brush(j);
    free_brush(s->brush);

    send_event(EVENT_SET_SPRITE_BRUSH);
    send_index(i);
    send_index((s->brush = j));
}

void recv_set_sprite_brush(void)
{
    int i = recv_index();
    int j = recv_index();

    struct sprite *s = S(i);

    dupe_brush(j);
    free_brush(s->brush);

    s->brush = j;
}

/*---------------------------------------------------------------------------*/

void send_set_sprite_range(int i, float s0, float s1, float t0, float t1)
{
    send_event(EVENT_SET_SPRITE_RANGE);
    send_index(i);
    send_float((S(i)->s0 = s0));
    send_float((S(i)->s1 = s1));
    send_float((S(i)->t0 = t0));
    send_float((S(i)->t1 = t1));
}

void recv_set_sprite_range(void)
{
    int i = recv_index();

    S(i)->s0 = recv_float();
    S(i)->s1 = recv_float();
    S(i)->t0 = recv_float();
    S(i)->t1 = recv_float();
}

/*===========================================================================*/

static void draw_sprite(int j, int i, int f, float a)
{
    glPushMatrix();
    {
        int dx = get_brush_w(S(i)->brush) / 2;
        int dy = get_brush_h(S(i)->brush) / 2;

        /* Apply the local coordinate system transformation. */

        transform_entity(j);

        if (test_entity_bbox(j) >= 0)
        {
            /* Draw the sprite to the color buffer. */

            draw_brush(S(i)->brush, a * get_entity_alpha(j));

            glBegin(GL_QUADS);
            {
                glTexCoord2f(S(i)->s0, S(i)->t0); glVertex2i(-dx, -dy);
                glTexCoord2f(S(i)->s1, S(i)->t0); glVertex2i(+dx, -dy);
                glTexCoord2f(S(i)->s1, S(i)->t1); glVertex2i(+dx, +dy);
                glTexCoord2f(S(i)->s0, S(i)->t1); glVertex2i(-dx, +dy);
            }
            glEnd();

            /* Render all child entities in this coordinate system. */

            draw_entity_tree(j, f, a * get_entity_alpha(j));
        }
    }
    glPopMatrix();
}

static int bbox_sprite(int i, float bound[6])
{
    int dx = get_brush_w(S(i)->brush) / 2;
    int dy = get_brush_h(S(i)->brush) / 2;

    bound[0] = -dx;
    bound[1] = -dy;
    bound[2] =   0;
    bound[3] = +dx;
    bound[4] = +dy;
    bound[5] =   0;

    return 1;
}

/*---------------------------------------------------------------------------*/

static void dupe_sprite(int i)
{
    S(i)->count++;
}

static void free_sprite(int i)
{
    if (--S(i)->count == 0)
    {
        free_brush(S(i)->brush);
        memset(S(i), 0, sizeof (struct sprite));
    }
}

/*===========================================================================*/

static struct entity_func sprite_func = {
    "sprite",
    NULL,
    NULL,
    bbox_sprite,
    draw_sprite,
    dupe_sprite,
    free_sprite,
};

struct entity_func *startup_sprite(void)
{
    if ((sprite = vecnew(MIN_SPRITES, sizeof (struct sprite))))
        return &sprite_func;
    else
        return NULL;
}
