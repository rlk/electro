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
#include "shared.h"
#include "server.h"
#include "entity.h"
#include "sprite.h"

/*---------------------------------------------------------------------------*/
/* Sprite entity storage                                                     */

static struct sprite *S     = NULL;
static int            S_max =    0;

static int sprite_exists(int id)
{
    return (S && 0 <= id && id < S_max && S[id].texture);
}

/*---------------------------------------------------------------------------*/

int sprite_init(void)
{
    if ((S = (struct sprite *) calloc(32, sizeof (struct sprite))))
    {
        S_max = 32;
        return 1;
    }
    return 0;
}

static void sprite_share(struct sprite *s, const char *filename)
{
    s->texture = shared_load_texture(filename, &s->w, &s->h);
    s->a       = 1.0f;
}

int sprite_create(const char *filename)
{
    int sd;

    if (S && (sd = buffer_unused(S_max, sprite_exists)) >= 0)
    {
        /* Initialize the new sprite. */

        if (mpi_isroot())
            server_send(EVENT_SPRITE_CREATE);

        /* Syncronize the new sprite. */

        mpi_share_integer(1, &sd);

        sprite_share(S + sd, filename);
        opengl_check("sprite_create");

        /* Encapsulate this new sprite in an entity. */

        return entity_create(TYPE_SPRITE, sd);
    }
    else if ((S = buffer_expand(S, &S_max, sizeof (struct sprite))))
        return sprite_create(filename);

    return -1;
}

/*---------------------------------------------------------------------------*/

void sprite_render(int id, int sd)
{
    if (sprite_exists(sd))
    {
        opengl_check("sprite_render before");

        glPushMatrix();
        {
            /* Apply the local coordinate system transformation. */

            entity_transform(id);

            /* Render this sprite. */

            glBindTexture(GL_TEXTURE_2D, S[sd].texture);
                
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

            opengl_check("sprite_render after");

            /* Render all child entities in this coordinate system. */

            entity_traversal(id);
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by the entity delete function. */

void sprite_delete(int sd)
{
    mpi_share_integer(1, &sd);

    /* Release the sprite object. */

    if (glIsTexture(S[sd].texture))
        glDeleteTextures(1, &S[sd].texture);

    memset(S + sd, 0, sizeof (struct sprite));
}

/*---------------------------------------------------------------------------*/
