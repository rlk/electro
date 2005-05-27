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

static void push_matrices(void)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}

static void pop_matrices(void)
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

static void draw_varrier_lines(int tile, float w, float h)
{
    const float p = get_varrier_pitch(tile);
    const float a = get_varrier_angle(tile);
    const float t = get_varrier_thick(tile);
    const float s = get_varrier_shift(tile);
    const float c = get_varrier_cycle(tile);

    const float k = 1 / p;

    float dx = tan(M_RAD(a)) * h;
    float dy = sin(M_RAD(a)) * w;
    float x;

    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT |
                 GL_COLOR_BUFFER_BIT);
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glColorMask(0, 0, 0, 0);

        glColor3f(1.0f, 1.0f, 1.0f);

        glRotatef(a, 0.0f, 0.0f, 1.0f);

        glBegin(GL_QUADS);
        {
            for (x = dx; x < w - dx; x += k)
            {
                glVertex2f(x,             dy);
                glVertex2f(x + k * c,     dy);
                glVertex2f(x + k * c, h - dy);
                glVertex2f(x,         h - dy);
            }
        }
        glEnd();
    }
    glPopAttrib();
    glPopMatrix();
}

static void draw_varrier_test(int eye, float w, float h)
{
    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDepthFunc(GL_LESS);
        
        if (eye)
            glColor3f(1.0f, 0.0f, 0.0f);
        else
            glColor3f(0.0f, 0.0f, 1.0f);

        glBegin(GL_QUADS);
        {
            glVertex2f(0, 0);
            glVertex2f(w, 0);
            glVertex2f(w, h);
            glVertex2f(0, h);
        }
        glEnd();
    }
    glPopAttrib();
}

int stereo_varrier(int eye, int tile, int pass)
{
    float o[3];
    float r[3];
    float u[3];

    float w;
    float h;

    if (pass == 0)
    {
        get_tile_o(tile, o);
        get_tile_r(tile, r);
        get_tile_u(tile, u);

        w = (float) sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
        h = (float) sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);

        push_matrices();
        {
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, w, 0, h, 0, 1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            draw_varrier_lines(tile, w, h);

            if (get_tile_flag(tile) & TILE_TEST)
                draw_varrier_test(eye, w, h);
        }
        pop_matrices();

        return 1;
    }
    return 0;
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

