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
#include <math.h>

#include "opengl.h"
#include "shared.h"
#include "server.h"
#include "entity.h"
#include "camera.h"

/*---------------------------------------------------------------------------*/

static float viewport_X =    0.0f;
static float viewport_Y =    0.0f;
static float viewport_x = -400.0f;
static float viewport_y = -300.0f;
static float viewport_w =  800.0f;
static float viewport_h =  600.0f;

/*---------------------------------------------------------------------------*/

static struct camera *C     = NULL;
static int            C_max =    2;

static int camera_exists(int id)
{
    return (C && 0 <= id && id < C_max && C[id].type);
}

/*---------------------------------------------------------------------------*/

int camera_create(int type)
{
    int id = -1;

    if (C && (id = buffer_unused(C_max, camera_exists)) >= 0)
    {
        /* Initialize the new camera. */

        if (mpi_isroot())
        {
            C[id].type =  type;
            C[id].dist =  0.0f;
            C[id].zoom = (type == CAMERA_ORTHO) ? 1.0f : 0.001f;

            server_send(EVENT_CAMERA_CREATE);
        }

        /* Syncronize the new camera. */

        mpi_share_integer(1, &id);
        mpi_share_integer(1, &C[id].type);
        mpi_share_float  (1, &C[id].dist);
        mpi_share_float  (1, &C[id].zoom);

        /* Encapsulate this new camera in an entity. */

        id = entity_create(TYPE_CAMERA, id);
    }
    else if ((C = buffer_expand(C, &C_max, sizeof (struct camera))))
        id = camera_create(type);

    return id;
}

void camera_delete(int id)
{
}

/*---------------------------------------------------------------------------*/

void camera_render(int id, const float pos[3], const float rot[3])
{
    double T = PI * rot[1] / 180.0;
    double P = PI * rot[0] / 180.0;

    float p[3];

    /* Compute the camera position given origin, rotation, and distance. */

    p[0] = pos[0];
    p[1] = pos[1];
    p[2] = pos[2];

    if (fabs(C[id].dist) > 0.0f)
    {
        p[0] += (float) (sin(T) * cos(P) * C[id].dist);
        p[1] -= (float) (         sin(P) * C[id].dist);
        p[2] += (float) (cos(T) * cos(P) * C[id].dist);
    }

    /* Load an off-axis projection for the current tile. */

    glMatrixMode(GL_PROJECTION);
    {
        GLdouble l =  C[id].zoom *  viewport_x;
        GLdouble r =  C[id].zoom * (viewport_x + viewport_w);
        GLdouble b = -C[id].zoom * (viewport_y + viewport_h);
        GLdouble t = -C[id].zoom *  viewport_y;

        glLoadIdentity();

        if (C[id].type == CAMERA_PERSP) glFrustum(l, r, b, t, 1.0, CAMERA_FAR);
        if (C[id].type == CAMERA_ORTHO) glOrtho  (l, r, b, t, -1.0, 1.0);
    }
    glMatrixMode(GL_MODELVIEW);

    /* Load the current camera transform. */

    glTranslatef(0, 0, -C[id].dist);

    /* Use the view configuration as vertex program parameters. */

    /* TODO: Re-enable this.
    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1, camera_magn, 0, 0, 0);
    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, p[0], p[1], p[2], 1);
    */
}

/*---------------------------------------------------------------------------*/

static void set_window_pos(int X, int Y)
{
    char buf[32];

    /* SDL looks to the environment for window position. */

    sprintf(buf, "%d,%d", X, Y);
    setenv("SDL_VIDEO_WINDOW_POS", buf, 1);
}

void viewport_set(float X, float Y, float x, float y, float w, float h)
{
    viewport_X = X;
    viewport_Y = Y;
    viewport_x = x;
    viewport_y = y;
    viewport_w = w;
    viewport_h = h;

    set_window_pos((int) X, (int) Y);
}

int viewport_get_x(void)
{
    return (int) viewport_x;
}

int viewport_get_y(void)
{
    return (int) viewport_y;
}

int viewport_get_w(void)
{
    return (int) viewport_w;
}

int viewport_get_h(void)
{
    return (int) viewport_h;
}

/*---------------------------------------------------------------------------*/

void camera_set_dist(int id, float d)
{
    if (camera_exists(id))
    {
        if (mpi_isroot())
        {
            C[id].dist = d;
            server_send(EVENT_CAMERA_DIST);
        }

        mpi_share_integer(1, &id);
        mpi_share_float(1, &C[id].dist);
    }
}

void camera_set_zoom(int id, float z)
{
    if (camera_exists(id))
    {
        if (mpi_isroot())
        {
            C[id].zoom = z;
            server_send(EVENT_CAMERA_ZOOM);
        }

        mpi_share_integer(1, &id);
        mpi_share_float(1, &C[id].zoom);
    }
}

/*---------------------------------------------------------------------------*/
