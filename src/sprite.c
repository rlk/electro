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

static int sprite_exists(int id)
{
    return (S && 0 <= id && id < S_max && S[id].texture);
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

void sprite_draw(int id, int sd)
{
    if (sprite_exists(sd))
    {
        glPushMatrix();
        {
            /* Apply the local coordinate system transformation. */

            entity_transform(id);

            /* Render this sprite. */

            glBindTexture(GL_TEXTURE_2D, S[sd].texture);
            glColor4f(1.0f, 1.0f, 1.0f, S[sd].a);

            glBegin(GL_QUADS);
            {
                int dx = S[sd].w / 2;
                int dy = S[sd].h / 2;

                glTexCoord2i(0, 0); glVertex2f(-dx, -dy);
                glTexCoord2i(1, 0); glVertex2f(+dx, -dy);
                glTexCoord2i(1, 1); glVertex2f(+dx, +dy);
                glTexCoord2i(0, 1); glVertex2f(-dx, +dy);
            }
            glEnd();

            opengl_check("sprite_render");

            /* Render all child entities in this coordinate system. */

            entity_traversal(id);
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/

static void sprite_create(int sd, int w, int h, int b, void *p)
{
    S[sd].texture = image_make_tex(p, w, h, b);
    S[sd].a       = 1.0f;
}

int sprite_send_create(const char *filename)
{
    int w, h, b, sd = buffer_unused(S_max, sprite_exists);
    void *p;

    if ((p = image_load_png(filename, &w, &h, &b)))
    {
        pack_event(EVENT_SPRITE_CREATE);
        pack_index(sd);
        pack_index(w);
        pack_index(h);
        pack_index(b);
        pack_alloc(p, w * h * b);

        sprite_create(sd, w, h, b, p);

        return entity_send_create(TYPE_SPRITE, sd);
    }
    return -1;
}

void sprite_recv_create(void)
{
    int  sd = unpack_index();
    int   w = unpack_index();
    int   h = unpack_index();
    int   b = unpack_index();
    void *p = unpack_alloc(w * h * b);

    sprite_create(sd, w, h, b, p);

    entity_recv_create();
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by the entity delete function. */

void sprite_delete(int sd)
{
    /* Release the sprite object. */

    if (S[sd].p) free(S[sd].p);

    if (glIsTexture(S[sd].texture))
        glDeleteTextures(1, &S[sd].texture);

    memset(S + sd, 0, sizeof (struct sprite));
}

/*---------------------------------------------------------------------------*/
