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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "opengl.h"
#include "shared.h"
#include "server.h"
#include "camera.h"

/*---------------------------------------------------------------------------*/

static float camera_pos[3];
static float camera_org[3];
static float camera_rot[3];

static float camera_dist;
static float camera_magn;
static float camera_zoom;

static float viewport_X;
static float viewport_Y;
static float viewport_x;
static float viewport_y;
static float viewport_w;
static float viewport_h;

/*---------------------------------------------------------------------------*/

void camera_init(void)
{
    camera_org[0] =    0.0f;
    camera_org[1] =   15.5f;
    camera_org[2] = 9200.0f;

    camera_rot[0] =    0.0f;
    camera_rot[1] =    0.0f;
    camera_rot[2] =    0.0f;

    camera_dist   =    0.0f;
    camera_magn   =  128.0f;
    camera_zoom   =    0.001f;

    viewport_X    =    0.0f;
    viewport_Y    =    0.0f;
    viewport_x    = -400.0f;
    viewport_y    = -300.0f;
    viewport_w    =  800.0f;
    viewport_h    =  600.0f;

    camera_set_pos();
}

void camera_draw(void)
{
    /* Load an off-axis projection for the current tile. */

    glMatrixMode(GL_PROJECTION);
    {
        GLdouble l =  camera_zoom *  viewport_x;
        GLdouble r =  camera_zoom * (viewport_x + viewport_w);
        GLdouble b = -camera_zoom * (viewport_y + viewport_h);
        GLdouble t = -camera_zoom *  viewport_y;

        glLoadIdentity();

        glFrustum(l, r, b, t, 1.0, 1000000.0);
    }

    /* Load the current camera transform. */

    glMatrixMode(GL_MODELVIEW);
    {
        glLoadIdentity();

        glTranslatef(0, 0, -camera_dist);

        glRotatef(-camera_rot[0], 1, 0, 0);
        glRotatef(-camera_rot[1], 0, 1, 0);
        glRotatef(-camera_rot[2], 0, 0, 1);
        
        glTranslatef(-camera_org[0], -camera_org[1], -camera_org[2]);
    }

    /* Use the view configuration as vertex program parameters. */

    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1, camera_magn, 0, 0, 0);
    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, camera_pos[0],
                                                         camera_pos[1],
                                                         camera_pos[2], 1);
}

/*---------------------------------------------------------------------------*/

static void camera_set_window_pos(int X, int Y)
{
    char buf[32];

    /* SDL looks to the environment for window position. */

    sprintf(buf, "%d,%d", X, Y);
    setenv("SDL_VIDEO_WINDOW_POS", buf, 1);
}

void camera_set_viewport(float X, float Y, float x, float y, float w, float h)
{
    viewport_X = X;
    viewport_Y = Y;
    viewport_x = x;
    viewport_y = y;
    viewport_w = w;
    viewport_h = h;

    camera_set_window_pos((int) X, (int) Y);
}

int camera_get_viewport_w(void)
{
    return (int) viewport_w;
}

int camera_get_viewport_h(void)
{
    return (int) viewport_h;
}

/*---------------------------------------------------------------------------*/

void camera_set_org(float x, float y, float z)
{
    int err;

    if (mpi_root())
    {
        camera_org[0] = x;
        camera_org[1] = y;
        camera_org[2] = z;
        server_send(EVENT_CAMERA_MOVE);
    }

    if ((err = MPI_Bcast(camera_org, 3, MPI_FLOAT, 0, MPI_COMM_WORLD)))
        mpi_error(err);

    camera_set_pos();
}

void camera_set_rot(float x, float y, float z)
{
    int err;

    if (mpi_root())
    {
        camera_rot[0] = x;
        camera_rot[1] = y;
        camera_rot[2] = z;
        server_send(EVENT_CAMERA_TURN);
    }

    if ((err = MPI_Bcast(camera_rot, 3, MPI_FLOAT, 0, MPI_COMM_WORLD)))
        mpi_error(err);

    camera_set_pos();
}

void camera_set_dist(float d)
{
    int err;

    if (mpi_root())
    {
        camera_dist = d;
        server_send(EVENT_CAMERA_DIST);
    }

    if ((err = MPI_Bcast(&camera_dist, 1, MPI_FLOAT, 0, MPI_COMM_WORLD)))
        mpi_error(err);

    camera_set_pos();
}

void camera_set_magn(float m)
{
    int err;

    if (mpi_root())
    {
        camera_magn = m;
        server_send(EVENT_CAMERA_MAGN);
    }

    if ((err = MPI_Bcast(&camera_magn, 1, MPI_FLOAT, 0, MPI_COMM_WORLD)))
        mpi_error(err);
}

void camera_set_zoom(float z)
{
    int err;

    if (mpi_root())
    {
        camera_zoom = z;
        server_send(EVENT_CAMERA_ZOOM);
    }

    if ((err = MPI_Bcast(&camera_zoom, 1, MPI_FLOAT, 0, MPI_COMM_WORLD)))
        mpi_error(err);
}

void camera_set_pos(void)
{
    double T = PI * camera_rot[1] / 180.0;
    double P = PI * camera_rot[0] / 180.0;

    /* Compute the camera position given origin, rotation, and distance. */

    camera_pos[0] = camera_org[0] + sin(T) * cos(P) * camera_dist;
    camera_pos[1] = camera_org[1] -          sin(P) * camera_dist;
    camera_pos[2] = camera_org[2] + cos(T) * cos(P) * camera_dist;
}

/*---------------------------------------------------------------------------*/
