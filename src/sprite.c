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

static struct sprite *get_sprite(int i)
{
    return (struct sprite *) vecget(sprite, i);
}

static int new_sprite(void)
{
    int i, n = vecnum(sprite);

    for (i = 0; i < n; ++i)
        if (get_sprite(i)->count == 0)
            return i;

    return vecadd(sprite);
}

/*===========================================================================*/

int send_create_sprite(int j)
{
    int i;

    if ((i = new_sprite()) >= 0)
    {
        struct sprite *s = get_sprite(i);

        s->brush = j;
        s->count = 1;
        s->s0 = 0.0f;
        s->t0 = 0.0f;
        s->s1 = 1.0f;
        s->t1 = 1.0f;

        send_event(EVENT_CREATE_SPRITE);
        send_index(s->brush);

        return send_create_entity(TYPE_SPRITE, i);
    }
    return -1;
}

void recv_create_sprite(void)
{
    int i = new_sprite();
    int j = recv_index();

    struct sprite *s = get_sprite(i);

    s->brush = j;
    s->count = 1;
    s->s0 = 0.0f;
    s->t0 = 0.0f;
    s->s1 = 1.0f;
    s->t1 = 1.0f;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_sprite_brush(int i, int j)
{
    struct sprite *s = get_sprite(i);

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

    struct sprite *s = get_sprite(i);

    dupe_brush(j);
    free_brush(s->brush);

    s->brush = j;
}

/*---------------------------------------------------------------------------*/

void send_set_sprite_range(int i, float s0, float s1, float t0, float t1)
{
    struct sprite *s = get_sprite(i);

    send_event(EVENT_SET_SPRITE_RANGE);
    send_index(i);
    send_float((s->s0 = s0));
    send_float((s->s1 = s1));
    send_float((s->t0 = t0));
    send_float((s->t1 = t1));
}

void recv_set_sprite_range(void)
{
    struct sprite *s = get_sprite(recv_index());

    s->s0 = recv_float();
    s->s1 = recv_float();
    s->t0 = recv_float();
    s->t1 = recv_float();
}

/*===========================================================================*/

static void draw_sprite(int j, int i, int f, float a)
{
    struct sprite *s = get_sprite(i);

    glPushMatrix();
    {
        int dx = get_brush_w(s->brush) / 2;
        int dy = get_brush_h(s->brush) / 2;

        /* Apply the local coordinate system transformation. */

        transform_entity(j);

        if (test_entity_bbox(j) >= 0)
        {
            /* Draw the sprite to the color buffer. */

            glPushAttrib(GL_DEPTH_BUFFER_BIT);
            {
                if (draw_brush(s->brush, a * get_entity_alpha(j)))
                    glDepthMask(GL_FALSE);

                glBegin(GL_QUADS);
                {
                    glTexCoord2f(s->s0, s->t0); glVertex2i(-dx, -dy);
                    glTexCoord2f(s->s1, s->t0); glVertex2i(+dx, -dy);
                    glTexCoord2f(s->s1, s->t1); glVertex2i(+dx, +dy);
                    glTexCoord2f(s->s0, s->t1); glVertex2i(-dx, +dy);
                }
                glEnd();
            }
            glPopAttrib();

            /* Render all child entities in this coordinate system. */

            draw_entity_tree(j, f, a * get_entity_alpha(j));
        }
    }
    glPopMatrix();
}

static int bbox_sprite(int i, float bound[6])
{
    struct sprite *s = get_sprite(i);

    int dx = get_brush_w(s->brush) / 2;
    int dy = get_brush_h(s->brush) / 2;

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
    get_sprite(i)->count++;
}

static void free_sprite(int i)
{
    struct sprite *s = get_sprite(i);

    if (--s->count == 0)
    {
        free_brush(s->brush);
        memset(s, 0, sizeof (struct sprite));
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
