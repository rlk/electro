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

#include <math.h>

#include "opengl.h"
#include "stereo.h"
#include "matrix.h"
#include "display.h"

/*---------------------------------------------------------------------------*/

static void get_varrier_tile(int tile, float M[16],
                                       float c[3], float *w, float *h)
{
    float o[3];
    float r[3];
    float u[3];
    float n[3];

    /* Find the center and extent of the tile. */

    get_tile_o(tile, o);
    get_tile_r(tile, r);
    get_tile_u(tile, u);

    c[0] = o[0] + (r[0] + u[0]) / 2;
    c[1] = o[1] + (r[1] + u[1]) / 2;
    c[2] = o[2] + (r[2] + u[2]) / 2;

    *w = (float) sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
    *h = (float) sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);

    /* Compute the basis and transform for the tile coordinate system. */

    v_cross(n, r, u);
    v_normal(r, r);
    v_normal(u, u);
    v_normal(n, n);

    M[0] = r[0]; M[4] = u[0]; M[8]  = n[0]; M[12] = 0.0f;
    M[1] = r[1]; M[5] = u[1]; M[9]  = n[1]; M[13] = 0.0f;
    M[2] = r[2]; M[6] = u[2]; M[10] = n[2]; M[14] = 0.0f;
    M[3] = 0.0f; M[7] = 0.0f; M[11] = 0.0f; M[15] = 1.0f;
}

/*---------------------------------------------------------------------------*/

static void draw_varrier_lines(int tile, const float M[16],
                                         const float o[3],
                                         float w, float h, float d)
{
    float p = get_varrier_pitch(tile);
    float a = get_varrier_angle(tile);
    float t = get_varrier_thick(tile);
    float s = get_varrier_shift(tile) + d;
    float c = get_varrier_cycle(tile);

    float  k = 1 / p;
    int i, n = w / k;

    int dx = tan(M_RAD(a)) * h / k;
    int dy = sin(M_RAD(a)) * w / k;

    glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT);
    glPushMatrix();
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glDrawBuffer(GL_NONE);
        glDepthRange(0, 0);

        /* Transform the line screen into position. */

        glTranslatef(o[0], o[1], o[2]);
        glMultMatrixf(M);

        /* Draw the line screen. */

        glTranslatef(s, 0, t);
        glRotatef(a, 0, 0, 1);

        glBegin(GL_QUADS);
        {
            glColor3f(1.0f, 1.0f, 1.0f);

            for (i = dx - n / 2; i < n / 2 - dx; ++i)
            {
                glVertex2f(k * i,         dy - h / 2);
                glVertex2f(k * i + k * c, dy - h / 2);
                glVertex2f(k * i + k * c, h / 2 - dy);
                glVertex2f(k * i,         h / 2 - dy);
            }
        }
        glEnd();

        glDrawBuffer(GL_BACK);
    }
    glPopMatrix();
    glPopAttrib();
}

static void draw_varrier_test(int eye, const float M[16],
                                       const float c[3], float w, float h)
{
    glPushAttrib(GL_ENABLE_BIT);
    glPushMatrix();
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDepthFunc(GL_LESS);
        
        glTranslatef(c[0], c[1], c[2]);
        glMultMatrixf(M);

        if (eye)
            glColor3f(0.0f, 1.0f, 0.0f);
        else
            glColor3f(0.0f, 0.0f, 1.0f);

        glBegin(GL_QUADS);
        {
            glVertex2f(-w / 2, -h / 2);
            glVertex2f(+w / 2, -h / 2);
            glVertex2f(+w / 2, +h / 2);
            glVertex2f(-w / 2, +h / 2);
        }
        glEnd();
    }
    glPopMatrix();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

int stereo_varrier_11(int eye, int tile, int pass)
{
    if (pass == 0)
    {
        float M[16];
        float c[3];
        float w, h;

        get_varrier_tile(tile, M, c, &w, &h);

        /* Draw the line screen into the depth buffer. */

        if (eye == 0)
        {
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            glColorMask(0, 0, 0, 0);
            draw_varrier_lines(tile, M, c, w, h, 0);
            glColorMask(1, 1, 1, 1);
        }
        else
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(0, 0, 0, 0);
            draw_varrier_lines(tile, M, c, w, h, 0);
            glColorMask(1, 1, 1, 1);
        }

        /* Draw the test pattern, if requested. */

        if (get_tile_flag(tile) & TILE_TEST)
        {
            draw_varrier_test(eye, M, c, w, h);
            return 0;
        }
        return 1;
    }
    return 0;
}

int stereo_varrier_33(int eye, int tile, int pass)
{
    float M[16];
    float c[3];
    float w, h, d = 0.00025;

    int next = -1;

    get_varrier_tile(tile, M, c, &w, &h);

    if (eye == 0)
    {
        switch (pass)
        {
        case 0:
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            glColorMask(1, 0, 0, 0);
            draw_varrier_lines(tile, M, c, w, h, +d);
            next = 1;
            break;

        case 1:
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(0, 1, 0, 0);
            draw_varrier_lines(tile, M, c, w, h,  0);
            next = 2;
            break;

        case 2:
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(0, 0, 1, 0);
            draw_varrier_lines(tile, M, c, w, h, -d);
            next = 3;
            break;

        case 3:
            glColorMask(1, 1, 1, 1);
            next = 0;
        }
    }
    else
    {
        switch (pass)
        {
        case 0:
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(1, 0, 0, 0);
            draw_varrier_lines(tile, M, c, w, h, +d);
            next = 1;
            break;

        case 1:
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(0, 1, 0, 0);
            draw_varrier_lines(tile, M, c, w, h,  0);
            next = 2;
            break;

        case 2:
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(0, 0, 1, 0);
            draw_varrier_lines(tile, M, c, w, h, -d);
            next = 3;
            break;

        case 3:
            glColorMask(1, 1, 1, 1);
            next = 0;
        }
    }

    /* Draw the test pattern, if requested. */

    if (next > 0 && get_tile_flag(tile) & TILE_TEST)
        draw_varrier_test(eye, M, c, w, h);

    return next;
}

/*---------------------------------------------------------------------------*/

int stereo_quad(int eye, int tile, int pass)
{
    if (pass == 0)
    {
        if (eye)
            glDrawBuffer(GL_BACK_RIGHT);
        else
            glDrawBuffer(GL_BACK_LEFT);
        return 1;
    }
    else
        return 0;
}

int stereo_red_blue(int eye, int tile, int pass)
{
    if (pass == 0)
    {
        if (eye)
        {
            glColorMask(0, 0, 1, 0);
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        else
        {
            glColorMask(1, 0, 0, 0);
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        return 1;
    }
    else
    {
        glColorMask(1, 1, 1, 1);
        return 0;
    }
}

/*---------------------------------------------------------------------------*/

