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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "opengl.h"
#include "viewport.h"
#include "shared.h"
#include "server.h"
#include "entity.h"
#include "camera.h"

/*---------------------------------------------------------------------------*/

static struct camera *C     = NULL;
static int            C_max =    0;

static int camera_exists(int cd)
{
    return (C && 0 <= cd && cd < C_max && C[cd].type);
}

/*---------------------------------------------------------------------------*/

int camera_init(void)
{
    if ((C = (struct camera *) calloc(2, sizeof (struct camera))))
    {
        C_max = 2;
        return 1;
    }
    return 0;
}

int camera_create(int type)
{
    int cd;

    if (C && (cd = buffer_unused(C_max, camera_exists)) >= 0)
    {
        /* Initialize the new camera. */

        if (mpi_isroot())
        {
            C[cd].type =  type;
            server_send(EVENT_CAMERA_CREATE);
        }

        C[cd].dist = 0.0f;
        C[cd].zoom = 1.0f;

        /* Syncronize the new camera. */

        mpi_share_integer(1, &cd);
        mpi_share_integer(1, &C[cd].type);

        /* Encapsulate this new camera in an entity. */

        return entity_create(TYPE_CAMERA, cd);
    }
    else if ((C = buffer_expand(C, &C_max, sizeof (struct camera))))
        return camera_create(type);

    return -1;
}

/*---------------------------------------------------------------------------*/

void camera_render(int id, int cd)
{
    float p[3];
    float r[3];

    entity_get_position(id, p + 0, p + 1, p + 2);
    entity_get_rotation(id, r + 0, r + 1, r + 2);

    if (camera_exists(cd))
    {
        int viewport_x0 = viewport_get_x();
        int viewport_x1 = viewport_get_x() + viewport_get_w();
        int viewport_y0 = viewport_get_y();
        int viewport_y1 = viewport_get_y() + viewport_get_h();

        double T = PI * r[1] / 180.0;
        double P = PI * r[0] / 180.0;

        /* Compute the camera position given origin, rotation, and distance. */

        if (fabs(C[cd].dist) > 0.0f)
        {
            p[0] += (float) (sin(T) * cos(P) * C[cd].dist);
            p[1] -= (float) (         sin(P) * C[cd].dist);
            p[2] += (float) (cos(T) * cos(P) * C[cd].dist);
        }

        /* Load projection and modelview matrices for this camera. */

        glMatrixMode(GL_PROJECTION);
        {
            GLdouble l =  C[cd].zoom * viewport_x0;
            GLdouble r =  C[cd].zoom * viewport_x1;
            GLdouble b = -C[cd].zoom * viewport_y1;
            GLdouble t = -C[cd].zoom * viewport_y0;
            GLdouble f =  CAMERA_FAR;

            glLoadIdentity();

            if (C[cd].type == CAMERA_PERSP) glFrustum(l, r, b, t, 1.0, f);
            if (C[cd].type == CAMERA_ORTHO) glOrtho  (l, r, b, t,  -f, f);
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glLoadIdentity();
            glTranslatef(0, 0, -C[cd].dist);

            entity_transform(id);
        }

        opengl_check("camera_render");

        /* Render all children using this camera. */

        entity_traversal(id);
    }

    /* Use the view configuration as vertex program parameters. */

    /* TODO: Re-enable this.
    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1, camera_magn, 0, 0, 0);
    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, p[0], p[1], p[2], 1);
    */
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by the entity delete function. */

void camera_delete(int cd)
{
    mpi_share_integer(1, &cd);

    memset(C + cd, 0, sizeof (struct camera));
}

/*---------------------------------------------------------------------------*/

void camera_set_dist(int cd, float d)
{
    if (mpi_isroot())
    {
        C[cd].dist = d;

        server_send(EVENT_CAMERA_DIST);

        mpi_share_integer(1, &cd);
        mpi_share_float(1, &C[cd].dist);
    }
    else
    {
        mpi_share_integer(1, &cd);
        mpi_share_float(1, &C[cd].dist);
    }
}

void camera_set_zoom(int cd, float z)
{
    if (mpi_isroot())
    {
        C[cd].zoom = z;

        server_send(EVENT_CAMERA_ZOOM);

        mpi_share_integer(1, &cd);
        mpi_share_float(1, &C[cd].zoom);
    }
    else
    {
        mpi_share_integer(1, &cd);
        mpi_share_float(1, &C[cd].zoom);
    }
}

/*---------------------------------------------------------------------------*/
