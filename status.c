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
#include <unistd.h>

#include "opengl.h"

/*---------------------------------------------------------------------------*/

static float camera_pos[4];
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

void status_init(void)
{
    camera_pos[0] =    0.0f;
    camera_pos[1] =   15.5f;
    camera_pos[2] = 9200.0f;
    camera_pos[3] =    1.0f;

    camera_rot[0] =    0.0f;
    camera_rot[1] =    0.0f;
    camera_rot[2] =    0.0f;

    camera_dist   = 1000.0f;
    camera_magn   =  128.0f;
    camera_zoom   =    0.5f;

    viewport_X =    0.0f;
    viewport_Y =    0.0f;
    viewport_x = -400.0f;
    viewport_y = -300.0f;
    viewport_w =  800.0f;
    viewport_h =  600.0f;
}

void status_draw_camera(void)
{
    /* Load an off-axis projection for the current tile. */

    printf("%f %f %f %f\n", viewport_x, viewport_y, viewport_w, viewport_h);

    glMatrixMode(GL_PROJECTION);
    {
        GLdouble l = camera_zoom *  viewport_x;
        GLdouble r = camera_zoom * (viewport_x + viewport_w);
        GLdouble b = camera_zoom *  viewport_y;
        GLdouble t = camera_zoom * (viewport_y + viewport_h);

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
        
        glTranslatef(-camera_pos[0], -camera_pos[1], -camera_pos[2]);
    }

    /* Use the view configuration as vertex program parameters. */

    glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, 0, camera_pos);
    glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, 1, camera_magn);
}

/*---------------------------------------------------------------------------*/

void status_set_viewport(float X, float Y, float x, float y, float w, float h)
{
    char buf[32];

    printf("set %f %f %f %f %f\n", X, Y, x, y, w, h);

    viewport_X = X;
    viewport_Y = Y;
    viewport_x = x;
    viewport_y = y;
    viewport_w = w;
    viewport_h = h;

    /* SDL looks to the environment for window placement. */
    /*
    sprintf(buf, "%d, %d", (int) X, (int) Y);
    setenv("SDL_VIDEO_WINDOW_POS", buf, 1);
    */
}

void status_set_camera_pos(float x, float y, float z)
{
    camera_pos[0] = x;
    camera_pos[1] = y;
    camera_pos[2] = z;
}

void status_set_camera_rot(float x, float y, float z)
{
    camera_rot[0] = x;
    camera_rot[1] = y;
    camera_rot[2] = z;
}

void status_set_camera_dist(float d)
{
    camera_dist = d;
}

void status_set_camera_magn(float d)
{
    camera_magn = d;
}

void status_set_camera_zoom(float d)
{
    camera_zoom = d;
}

/*---------------------------------------------------------------------------*/

int status_get_viewport_w(void)
{
    return (int) viewport_w;
}

int status_get_viewport_h(void)
{
    return (int) viewport_h;
}

void status_get_camera_pos(float *x, float *y, float *z)
{
    *x = camera_pos[0];
    *y = camera_pos[1];
    *z = camera_pos[2];
}

void status_get_camera_rot(float *x, float *y, float *z)
{
    *x = camera_rot[0];
    *y = camera_rot[1];
    *z = camera_rot[2];
}

float status_get_camera_dist(void)
{
    return camera_dist;
}

float status_get_camera_magn(void)
{
    return camera_magn;
}

float status_get_camera_zoom(void)
{
    return camera_zoom;
}

/*---------------------------------------------------------------------------*/
