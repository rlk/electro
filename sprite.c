/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    TOTAL PERSPECTIVE VORTEX is free software;  you can redistribute it    */
/*    and/or modify it under the terms of the  GNU General Public License    */
/*    as published by the  Free Software Foundation;  either version 2 of    */
/*    the License, or (at your option) any later version.                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mpi.h>

#include "opengl.h"
#include "shared.h"
#include "server.h"
#include "camera.h"
#include "sprite.h"

/*---------------------------------------------------------------------------*/

static struct sprite *S;

int sprite_exists(int id)
{
    return (S && 0 <= id && id < MAXSPRITE && S[id].texture);
}

/*---------------------------------------------------------------------------*/

int sprite_init(void)
{
    if ((S = (struct sprite *) calloc(MAXSPRITE, sizeof (struct sprite))))
        return 1;
    else
        return 0;
}

void sprite_draw(void)
{
    int x = camera_get_viewport_x();
    int y = camera_get_viewport_y();
    int w = camera_get_viewport_w();
    int h = camera_get_viewport_h();
    int id;

    glPushAttrib(GL_COLOR_BUFFER_BIT);
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /* Load a 1-to-1 pixel orthogonal projection. */

        glMatrixMode(GL_PROJECTION);
        {
            glLoadIdentity();
            glOrtho(x, x + w, y + h, y, -1, 1);
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glLoadIdentity();
        }

        /* Iterate across all sprites, rendering each. */

        for (id = 0; id < MAXSPRITE; id++)
            if (S[id].texture)
            {
                glBindTexture(GL_TEXTURE_2D, S[id].texture);
                
                glPushMatrix();
                {
                    glTranslatef(S[id].pos[0], S[id].pos[1], 0.0f);
                    glScalef(S[id].size, S[id].size, 1.0f);
                    glRotatef(S[id].rot, 0.0f, 0.0f, 1.0f);
                    glColor4f(1.0f, 1.0f, 1.0f, S[id].alpha);

                    glBegin(GL_QUADS);
                    {
                        int dx = S[id].w / 2;
                        int dy = S[id].h / 2;

                        glTexCoord2i(0, 0); glVertex2f(-dx, +dy);
                        glTexCoord2i(0, 1); glVertex2f(-dx, -dy);
                        glTexCoord2i(1, 1); glVertex2f(+dx, -dy);
                        glTexCoord2i(1, 0); glVertex2f(+dx, +dy);
                    }
                    glEnd();
                }
                glPopMatrix();
            }
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

int sprite_load(const char *filename)
{
    char buf[NAMELEN];

    int id = -1;
    int err;

    if (mpi_root())
    {
        /* If this host is root, find a free sprite descriptor. */

        for (id = MAXSPRITE - 1; id >= 0; --id)
            if (S[id].texture == 0)
                break;

        strncpy(buf, filename, NAMELEN);
        server_send(EVENT_SPRITE_LOAD);
    }

    /* Broadcast the descriptor and the filename. */

    if ((err = MPI_Bcast(&id, 1, MPI_INTEGER, 0, MPI_COMM_WORLD)))
        mpi_error(err);
    if ((err = MPI_Bcast(buf, NAMELEN, MPI_CHAR, 0, MPI_COMM_WORLD)))
        mpi_error(err);

    /* Initialize the sprite object. */

    if (id >= 0)
    {
        S[id].texture = shared_load_texture(buf, &S[id].w, &S[id].h);
        S[id].pos[0]  = 0.0f;
        S[id].pos[1]  = 0.0f;
        S[id].rot     = 0.0f;
        S[id].size    = 1.0f;
        S[id].alpha   = 1.0f;
    }

    return id;
}

void sprite_free(int id)
{
    int err;

    if (mpi_root())
        server_send(EVENT_SPRITE_FREE);

    /* Broadcast the descriptor. */

    if ((err = MPI_Bcast(&id, 1, MPI_INTEGER, 0, MPI_COMM_WORLD)))
        mpi_error(err);

    /* Release the sprite object. */

    if (sprite_exists(id))
    {
        glDeleteTextures(1, &S[id].texture);
        S[id].texture = 0;
    }
}

/*---------------------------------------------------------------------------*/

void sprite_move(int id, float x, float y)
{
    int err;

    if (mpi_root())
    {
        if (sprite_exists(id))
        {
            S[id].pos[0] = x;
            S[id].pos[1] = y;
        }
        server_send(EVENT_SPRITE_MOVE);
    }

    if ((err = MPI_Bcast(&id, 1, MPI_INTEGER, 0, MPI_COMM_WORLD)))
        mpi_error(err);
    if (sprite_exists(id))
        if ((err = MPI_Bcast(S[id].pos, 2, MPI_FLOAT, 0, MPI_COMM_WORLD)))
            mpi_error(err);
}

void sprite_turn(int id, float a)
{
    int err;

    if (mpi_root())
    {
        if (sprite_exists(id))
            S[id].rot = a;

        server_send(EVENT_SPRITE_TURN);
    }

    if ((err = MPI_Bcast(&id, 1, MPI_INTEGER, 0, MPI_COMM_WORLD)))
        mpi_error(err);
    if (sprite_exists(id))
        if ((err = MPI_Bcast(&S[id].rot, 1, MPI_FLOAT, 0, MPI_COMM_WORLD)))
            mpi_error(err);
}

void sprite_size(int id, float s)
{
    int err;

    if (mpi_root())
    {
        if (sprite_exists(id))
            S[id].size = s;

        server_send(EVENT_SPRITE_SIZE);
    }

    if ((err = MPI_Bcast(&id, 1, MPI_INTEGER, 0, MPI_COMM_WORLD)))
        mpi_error(err);
    if (sprite_exists(id))
        if ((err = MPI_Bcast(&S[id].size, 1, MPI_FLOAT, 0, MPI_COMM_WORLD)))
            mpi_error(err);
}

void sprite_fade(int id, float f)
{
    int err;

    if (mpi_root())
    {
        if (sprite_exists(id))
            S[id].alpha = f;

        server_send(EVENT_SPRITE_FADE);
    }

    if ((err = MPI_Bcast(&id, 1, MPI_INTEGER, 0, MPI_COMM_WORLD)))
        mpi_error(err);
    if (sprite_exists(id))
        if ((err = MPI_Bcast(&S[id].alpha, 1, MPI_FLOAT, 0, MPI_COMM_WORLD)))
            mpi_error(err);
}

/*---------------------------------------------------------------------------*/
