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
static int            S_max =   64;

static int sprite_exists(int id)
{
    return (S && 0 < id && id < S_max && S[id].texture);
}

/*---------------------------------------------------------------------------*/

int sprite_create(const char *filename)
{
    int id = -1;

    if (S && (id = buffer_unused(S_max, sprite_exists)) >= 0)
    {
        /* Initialize the new sprite. */

        if (mpi_isroot())
            server_send(EVENT_SPRITE_CREATE);

        /* Syncronize the new sprite. */

        mpi_share_integer(1, &id);

        S[id].texture = shared_load_texture(filename, &S[id].w, &S[id].h);
        S[id].a       = 1.0f;

        /* Encapsulate this new sprite in an entity. */

        id = entity_create(TYPE_SPRITE, id);
    }
    else if ((S = buffer_expand(S, &S_max, sizeof (struct sprite))))
        id = sprite_create(filename);

    return id;
}

/*---------------------------------------------------------------------------*/

void sprite_render(int id, int sd)
{
    if (sprite_exists(sd))
    {
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

            /* Render all child entities in this coordinate system. */
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by the entity delete function. */

void sprite_delete(int id)
{
    /* Assume delete was triggered by entity delete.  Share the descriptor. */

    mpi_share_integer(1, &id);

    /* Release the sprite object. */

    if (glIsTexture(S[id].texture))
        glDeleteTextures(1, &S[id].texture);

    memset(S + id, 0, sizeof (struct sprite));
}

/*---------------------------------------------------------------------------*/
