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
#include "server.h"
#include "entity.h"
#include "image.h"
#include "sprite.h"

/*---------------------------------------------------------------------------*/
/* Sprite entity storage                                                     */

#define SMAXINIT 256

static struct sprite *S;
static int            S_max;

static int sprite_exists(int sd)
{
    return (S && 0 <= sd && sd < S_max && S[sd].flag);
}

/*---------------------------------------------------------------------------*/

int sprite_init(void)
{
    if ((S = (struct sprite *) calloc(SMAXINIT, sizeof (struct sprite))))
    {
        S_max = SMAXINIT;
        return 1;
    }
    return 0;
}

void sprite_draw(int id, int sd, float a)
{
    if (sprite_exists(sd))
    {
        glPushAttrib(GL_DEPTH_BUFFER_BIT);
        glPushMatrix();
        {
            /* Apply the local coordinate system transformation. */

            entity_transform(id);

            glDisable(GL_DEPTH_TEST);

            image_draw(S[sd].image);
            glColor4f(1.0f, 1.0f, 1.0f, a);

            glBegin(GL_QUADS);
            {
                int dx = image_get_w(S[sd].image) / 2;
                int dy = image_get_h(S[sd].image) / 2;

                glTexCoord2f(S[sd].s0, S[sd].t0); glVertex2f(-dx, -dy);
                glTexCoord2f(S[sd].s1, S[sd].t0); glVertex2f(+dx, -dy);
                glTexCoord2f(S[sd].s1, S[sd].t1); glVertex2f(+dx, +dy);
                glTexCoord2f(S[sd].s0, S[sd].t1); glVertex2f(-dx, +dy);
            }
            glEnd();

            opengl_check("sprite_draw");

            /* Render all child entities in this coordinate system. */

            entity_traversal(id, a);
        }
        glPopMatrix();
        glPopAttrib();
    }
}

/*---------------------------------------------------------------------------*/

int sprite_send_create(const char *filename)
{
    int sd;

    if ((sd = buffer_unused(S_max, sprite_exists)) >= 0)
    {
        S[sd].image = image_send_create(filename);
        S[sd].flag  = 1;

        pack_event(EVENT_SPRITE_CREATE);
        pack_index(sd);
        pack_index(S[sd].image);

        S[sd].s0 = S[sd].t0 = 0.0f;
        S[sd].s1 = S[sd].t1 = 1.0f;

        return entity_send_create(TYPE_SPRITE, sd);
    }
    return -1;
}

void sprite_recv_create(void)
{
    int sd = unpack_index();

    S[sd].image = unpack_index();
    S[sd].flag  = 1;

    S[sd].s0 = S[sd].t0 = 0.0f;
    S[sd].s1 = S[sd].t1 = 1.0f;

    entity_recv_create();
}

/*---------------------------------------------------------------------------*/

void sprite_send_bounds(int sd, float s0, float s1, float t0, float t1)
{
    if (sprite_exists(sd))
    {
        pack_event(EVENT_SPRITE_BOUNDS);
        pack_index(sd);
        pack_float((S[sd].s0 = s0));
        pack_float((S[sd].s1 = s1));
        pack_float((S[sd].t0 = t0));
        pack_float((S[sd].t1 = t1));
    }
}

void sprite_recv_bounds(void)
{
    int sd   = unpack_index();

    S[sd].s0 = unpack_float();
    S[sd].s1 = unpack_float();
    S[sd].t0 = unpack_float();
    S[sd].t1 = unpack_float();
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by the entity delete function. */

void sprite_delete(int sd)
{
    if (sprite_exists(sd))
        memset(S + sd, 0, sizeof (struct sprite));
}

/*---------------------------------------------------------------------------*/
