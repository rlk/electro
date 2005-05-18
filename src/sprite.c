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

struct sprite
{
    int   count;
    int   image;
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

int startup_sprite(void)
{
    if ((sprite = vecnew(256, sizeof (struct sprite))))
        return 1;
    else
        return 0;
}

/*===========================================================================*/

int send_create_sprite(const char *filename)
{
    int i;

    if ((i = new_sprite()) >= 0)
    {
        S(i)->image = send_create_image(filename);
        S(i)->count = 1;
        S(i)->state = 0;
        S(i)->s0 = 0.0f;
        S(i)->t0 = 0.0f;
        S(i)->s1 = 1.0f;
        S(i)->t1 = 1.0f;

        pack_event(EVENT_CREATE_SPRITE);
        pack_index(i);
        pack_index(S(i)->image);

        return send_create_entity(TYPE_SPRITE, i);
    }
    return -1;
}

void recv_create_sprite(void)
{
    int i = unpack_index();

    S(i)->image = unpack_index();
    S(i)->count = 1;
    S(i)->state = 0;
    S(i)->s0 = 0.0f;
    S(i)->t0 = 0.0f;
    S(i)->s1 = 1.0f;
    S(i)->t1 = 1.0f;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_sprite_bounds(int i, float s0, float s1, float t0, float t1)
{
    pack_event(EVENT_SET_SPRITE_BOUNDS);
    pack_index(i);
    pack_float((S(i)->s0 = s0));
    pack_float((S(i)->s1 = s1));
    pack_float((S(i)->t0 = t0));
    pack_float((S(i)->t1 = t1));
}

void recv_set_sprite_bounds(void)
{
    int i = unpack_index();

    S(i)->s0 = unpack_float();
    S(i)->s1 = unpack_float();
    S(i)->t0 = unpack_float();
    S(i)->t1 = unpack_float();
}

/*---------------------------------------------------------------------------*/

void get_sprite_p(int i, int x, int y, unsigned char p[4])
{
    get_image_p(S(i)->image, x, y, p);
}

int get_sprite_w(int i)
{
    return get_image_w(S(i)->image);
}

int get_sprite_h(int i)
{
    return get_image_h(S(i)->image);
}

/*===========================================================================*/

static void draw_sprite(int j, int i, const float M[16],
                                      const float I[16],
                                      const struct frustum *F, float a)
{
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    {
        float N[16];
        float J[16];

        /* Apply the local coordinate system transformation. */

        transform_entity(j, N, M, J, I);

        /* Draw the image to the color buffer. */

        glDepthMask(GL_FALSE);
        draw_image(S(i)->image);

        glColor4f(1.0f, 1.0f, 1.0f, a * get_entity_alpha(j));

        glBegin(GL_QUADS);
        {
            int dx = get_image_w(S(i)->image) / 2;
            int dy = get_image_h(S(i)->image) / 2;

            glTexCoord2f(S(i)->s0, S(i)->t0); glVertex2i(-dx, -dy);
            glTexCoord2f(S(i)->s1, S(i)->t0); glVertex2i(+dx, -dy);
            glTexCoord2f(S(i)->s1, S(i)->t1); glVertex2i(+dx, +dy);
            glTexCoord2f(S(i)->s0, S(i)->t1); glVertex2i(-dx, +dy);
        }
        glEnd();

        /* Render all child entities in this coordinate system. */

        draw_entity_list(j, N, J, F, a * get_entity_alpha(j));
    }
    glPopMatrix();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static void init_sprite(int i)
{
    init_image(S(i)->image);
}

static void fini_sprite(int i)
{
    fini_image(S(i)->image);
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
        free_sprite_gl(i);

        memset(S(i), 0, sizeof (struct sprite));
    }
}

/*===========================================================================*/

struct entity_func sprite_func = {
    init_sprite,
    fini_sprite,
    draw_sprite
    dupe_sprite,
    free_sprite,
};
