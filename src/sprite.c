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
#include "image.h"
#include "event.h"
#include "sprite.h"

/*---------------------------------------------------------------------------*/

static vector_t S;

#define get_sprite(i) ((struct sprite *) vecget(S, i))

static int new_sprite(void)
{
    int n = vecnum(S);
    int i;

    for (i = 0; i < n; ++i)
        if (get_sprite(i)->count == 0)
            return i;

    return vecadd(S);
}

/*---------------------------------------------------------------------------*/

int init_sprite(void)
{
    if ((S = vecnew(256, sizeof (struct sprite))))
        return 1;
    else
        return 0;
}

void draw_sprite(int j, int i, const float M[16],
                               const float I[16],
                               const struct frustum *F, float a)
{
    struct sprite *s = get_sprite(i);

    init_sprite_gl(i);

    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    {
        float N[16];
        float J[16];

        /* Apply the local coordinate system transformation. */

        transform_entity(j, N, M, J, I);

        /* Draw the image to the color buffer. */

        glDepthMask(GL_FALSE);
        draw_image(s->image);

        glColor4f(1.0f, 1.0f, 1.0f, a * get_entity_alpha(j));

        glBegin(GL_QUADS);
        {
            int dx = get_image_w(s->image) / 2;
            int dy = get_image_h(s->image) / 2;

            glTexCoord2f(s->s0, s->t0); glVertex2i(-dx, -dy);
            glTexCoord2f(s->s1, s->t0); glVertex2i(+dx, -dy);
            glTexCoord2f(s->s1, s->t1); glVertex2i(+dx, +dy);
            glTexCoord2f(s->s0, s->t1); glVertex2i(-dx, +dy);
        }
        glEnd();

        /* Render all child entities in this coordinate system. */

        draw_entity_list(j, N, J, F, a * get_entity_alpha(j));
    }
    glPopMatrix();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

void init_sprite_gl(int i)
{
    struct sprite *s = get_sprite(i);

    if (s->state == 0)
    {
        init_image_gl(s->image);
        s->state  = 1;
    }
}

void free_sprite_gl(int i)
{
    struct sprite *s = get_sprite(i);

    if (s->state == 1)
    {
        free_image_gl(s->image);
        s->state  = 0;
    }
}

/*---------------------------------------------------------------------------*/

int send_create_sprite(const char *filename)
{
    int i;

    if ((i = new_sprite()) >= 0)
    {
        struct sprite *s = get_sprite(i);

        s->image = send_create_image(filename);
        s->count = 1;
        s->state = 0;
        s->s0 = 0.0f;
        s->t0 = 0.0f;
        s->s1 = 1.0f;
        s->t1 = 1.0f;

        pack_event(EVENT_CREATE_SPRITE);
        pack_index(i);
        pack_index(s->image);

        return send_create_entity(TYPE_SPRITE, i);
    }
    return -1;
}

void recv_create_sprite(void)
{
    struct sprite *s = get_sprite(unpack_index());

    s->image = unpack_index();
    s->count = 1;
    s->state = 0;
    s->s0 = 0.0f;
    s->t0 = 0.0f;
    s->s1 = 1.0f;
    s->t1 = 1.0f;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_sprite_bounds(int i, float s0, float s1, float t0, float t1)
{
    struct sprite *s = get_sprite(i);

    pack_event(EVENT_SET_SPRITE_BOUNDS);
    pack_index(i);
    pack_float((s->s0 = s0));
    pack_float((s->s1 = s1));
    pack_float((s->t0 = t0));
    pack_float((s->t1 = t1));
}

void recv_set_sprite_bounds(void)
{
    struct sprite *s = get_sprite(unpack_index());

    s->s0 = unpack_float();
    s->s1 = unpack_float();
    s->t0 = unpack_float();
    s->t1 = unpack_float();
}

/*---------------------------------------------------------------------------*/

void get_sprite_p(int i, int x, int y, unsigned char p[4])
{
    get_image_p(get_sprite(i)->image, x, y, p);
}

int get_sprite_w(int i)
{
    return get_image_w(get_sprite(i)->image);
}

int get_sprite_h(int i)
{
    return get_image_h(get_sprite(i)->image);
}

/*---------------------------------------------------------------------------*/
/* These may only be called by create_clone and delete_entity, respectively. */

void clone_sprite(int i)
{
    get_sprite(i)->count++;
}

void delete_sprite(int i)
{
    if (--get_sprite(i)->count == 0)
        free_sprite_gl(i);
}

/*---------------------------------------------------------------------------*/
