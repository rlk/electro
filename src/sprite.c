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
#include "image.h"
#include "event.h"
#include "utility.h"
#include "sprite.h"

/*---------------------------------------------------------------------------*/
/* Sprite entity storage                                                     */

#define SMAXINIT 256

static struct sprite *S;
static int            S_max;

static int sprite_exists(int sd)
{
    return (S && 0 <= sd && sd < S_max && S[sd].count);
}

static int alloc_sprite(void)
{
    int sd = -1;

    S = (struct sprite *) balloc(S, &sd, &S_max,
                                 sizeof (struct sprite), sprite_exists);
    return sd;
}

/*---------------------------------------------------------------------------*/

int init_sprite(void)
{
    if ((S = (struct sprite *) calloc(SMAXINIT, sizeof (struct sprite))))
    {
        S_max = SMAXINIT;
        return 1;
    }
    return 0;
}

void draw_sprite(int id, int sd, const struct frustum *F0, float a)
{
    struct frustum F1;

    if (sprite_exists(sd))
    {
        glPushAttrib(GL_DEPTH_BUFFER_BIT);
        glPushMatrix();
        {
            const float d[3] = { 0.0f, 0.0f, 0.0f };

            /* Apply the local coordinate system transformation. */

            transform_entity(id, &F1, F0, d);

            glDepthMask(GL_FALSE);
            draw_image(S[sd].image);

            glColor4f(1.0f, 1.0f, 1.0f, a * get_entity_alpha(id));

            glBegin(GL_QUADS);
            {
                int dx = get_image_w(S[sd].image) / 2;
                int dy = get_image_h(S[sd].image) / 2;

                glTexCoord2f(S[sd].s0, S[sd].t0); glVertex2i(-dx, -dy);
                glTexCoord2f(S[sd].s1, S[sd].t0); glVertex2i(+dx, -dy);
                glTexCoord2f(S[sd].s1, S[sd].t1); glVertex2i(+dx, +dy);
                glTexCoord2f(S[sd].s0, S[sd].t1); glVertex2i(-dx, +dy);
            }
            glEnd();

            /* Render all child entities in this coordinate system. */

            draw_entity_list(id, &F1, a * get_entity_alpha(id));
        }
        glPopMatrix();
        glPopAttrib();
    }
}

/*---------------------------------------------------------------------------*/

int send_create_sprite(const char *filename)
{
    int sd;

    if (S && (sd = alloc_sprite()) >= 0)
    {
        S[sd].image = send_create_image(filename);
        S[sd].count = 1;

        pack_event(EVENT_CREATE_SPRITE);
        pack_index(sd);
        pack_index(S[sd].image);

        S[sd].s0 = S[sd].t0 = 0.0f;
        S[sd].s1 = S[sd].t1 = 1.0f;

        return send_create_entity(TYPE_SPRITE, sd);
    }
    return -1;
}

void recv_create_sprite(void)
{
    int sd = unpack_index();

    S[sd].image = unpack_index();
    S[sd].count = 1;

    S[sd].s0 = S[sd].t0 = 0.0f;
    S[sd].s1 = S[sd].t1 = 1.0f;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_sprite_bounds(int sd, float s0, float s1, float t0, float t1)
{
    if (sprite_exists(sd))
    {
        pack_event(EVENT_SET_SPRITE_BOUNDS);
        pack_index(sd);
        pack_float((S[sd].s0 = s0));
        pack_float((S[sd].s1 = s1));
        pack_float((S[sd].t0 = t0));
        pack_float((S[sd].t1 = t1));
    }
}

void recv_set_sprite_bounds(void)
{
    int sd   = unpack_index();

    S[sd].s0 = unpack_float();
    S[sd].s1 = unpack_float();
    S[sd].t0 = unpack_float();
    S[sd].t1 = unpack_float();
}

/*---------------------------------------------------------------------------*/

void get_sprite_p(int sd, int x, int y, unsigned char p[4])
{
    if (sprite_exists(sd))
        get_image_p(S[sd].image, x, y, p);
}

int get_sprite_w(int sd)
{
    return sprite_exists(sd) ? get_image_w(S[sd].image) : 0;
}

int get_sprite_h(int sd)
{
    return sprite_exists(sd) ? get_image_h(S[sd].image) : 0;
}

/*---------------------------------------------------------------------------*/
/* These may only be called by create_clone and delete_entity, respectively. */

void clone_sprite(int sd)
{
    if (sprite_exists(sd))
        S[sd].count++;
}

void delete_sprite(int sd)
{
    if (sprite_exists(sd))
    {
        S[sd].count--;

        if (S[sd].count == 0)
            memset(S + sd, 0, sizeof (struct sprite));
    }
}

/*---------------------------------------------------------------------------*/
