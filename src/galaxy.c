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
#include <math.h>

#include "opengl.h"
#include "viewport.h"
#include "buffer.h"
#include "shared.h"
#include "entity.h"
#include "galaxy.h"
#include "star.h"
#include "node.h"

/*---------------------------------------------------------------------------*/

static float mag = 1.0f;

/*---------------------------------------------------------------------------*/

void galaxy_foo(void)
{
    node_init();
    prep_init();
    prep_file_hip("../hip_main.dat");
    prep_sort();
}

int galaxy_send_create(const char *nearfile, const char *farfile)
{
    /*
    if (nearfile)
    {
        if (strcmp(nearfile + strlen(nearfile) - 4, ".bin") == 0)
            star_read_near_bin(nearfile);
        else
            star_read_near_hip(nearfile);
    }
    if (farfile)
    {
        if (strcmp(farfile + strlen(farfile) - 4, ".bin") == 0)
            star_read_far_bin(farfile);
        else
            star_read_far_tyc(farfile);
    }
    */
    pack_event(EVENT_GALAXY_CREATE);
    galaxy_foo();
    /*
    star_send_create();
    */
    return entity_send_create(TYPE_GALAXY, 0);
}

void galaxy_recv_create(void)
{
    galaxy_foo();
    /*
    star_recv_create();
    */
    entity_recv_create();
}

/*---------------------------------------------------------------------------*/

void galaxy_send_magn(int gd, float m)
{
    pack_event(EVENT_GALAXY_MAGN);
    pack_float(m);

    mag = m * viewport_scale();
    mag = m;
}

void galaxy_recv_magn(void)
{
    mag = unpack_float();
}

/*---------------------------------------------------------------------------*/

void galaxy_draw(int id, int gd, float P[3], float V[4][4])
{
    float Q[3], W[4][4];

    glPushMatrix();
    {
        /* Apply the local coordinate system transformation. */
        /*
        entity_transform(id, Q, W, P, V);
        */
        glPushAttrib(GL_ENABLE_BIT  |
                     GL_TEXTURE_BIT |
                     GL_COLOR_BUFFER_BIT);
        {
            /* Set up the GL state for star rendering. */

            glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
            glEnable(GL_POINT_SPRITE_ARB);
            glEnable(GL_COLOR_MATERIAL);

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);

            glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
            glBlendFunc(GL_ONE, GL_ONE);

            glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1, mag, 0, 0, 0);

            /* Render all stars. */
            /*
            star_draw();
            */
            node_draw(P, V);
        }
        glPopAttrib();

        opengl_check("galaxy_draw");

        /* Render all child entities in this coordinate system. */

        entity_traversal(id, Q, W);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

void galaxy_delete(int gd)
{
    /*
    star_delete();
    */
}

/*---------------------------------------------------------------------------*/
