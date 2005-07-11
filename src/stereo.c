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
#include <stdlib.h>
#include <string.h>

#include "opengl.h"
#include "stereo.h"
#include "matrix.h"
#include "display.h"
#include "video.h"

/*---------------------------------------------------------------------------*/

static void get_varrier_tile(int tile, float M[16],
                                       float c[3],
                                       float n[3], float *w, float *h)
{
    float o[3];
    float r[3];
    float u[3];

    /* Find the center and extent of the tile. */

    get_tile_o(tile, o);
    get_tile_r(tile, r);
    get_tile_u(tile, u);
    get_tile_n(tile, n);

    c[0] = o[0] + (r[0] + u[0]) / 2;
    c[1] = o[1] + (r[1] + u[1]) / 2;
    c[2] = o[2] + (r[2] + u[2]) / 2;

    *w = (float) sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
    *h = (float) sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);

    /* Compute the basis and transform for the tile coordinate system. */

    normalize(r);
    normalize(u);

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

    float f = 1 / p;
    int i;
    int n = (int) ceil(w / f);

    /* TODO: Compute out the proper scale of the line screen. */

    int dx = (int) ceil(n);
    int dy = (int) ceil(h);

    glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glDepthRange(0, 0);

        /* Transform the line screen into position. */

        glTranslatef(o[0], o[1], o[2]);
        glMultMatrixf(M);

        /* Draw the line screen. */

        glTranslatef(s, 0, t);
        glRotatef(a, 0, 0, 1);

        glBegin(GL_QUADS);
        {
            glColor3f(0.0f, 0.0f, 0.0f);

            for (i = -n / 2 - dx; i < n / 2 + dx; ++i)
            {
                glVertex2f(f * i,         -h / 2 - dy);
                glVertex2f(f * i + f * c, -h / 2 - dy);
                glVertex2f(f * i + f * c,  h / 2 + dy);
                glVertex2f(f * i,          h / 2 + dy);
            }
        }
        glEnd();
    }
    glPopMatrix();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

#define LINESZ 128

static GLuint line_object[3] = { 0, 0, 0 };

static void init_line_texture(int tile, int chan)
{
    static const GLubyte color[3][4] = {
        { 0x00, 0xFF, 0xFF, 0x00 },
        { 0xFF, 0x00, 0xFF, 0x00 },
        { 0xFF, 0xFF, 0x00, 0x00 },
    };

    GLubyte *p;

    /* If the requested texture object already exists, bind it. */

    if (glIsTexture(line_object[chan]))
        glBindTexture(GL_TEXTURE_2D, line_object[chan]);

    else if ((p = (GLubyte *) calloc(LINESZ * 2, 4)))
    {
        float c = get_varrier_cycle(tile);
        int i;
        int j;

        /* Generate a new texture object */

        glGenTextures(1, line_object + chan);
        glBindTexture(GL_TEXTURE_2D, line_object[chan]);

        /* Fill it with the line screen pattern for the given channel. */

        memset(p, 0xFF, LINESZ * 8);

        for (i = 0; i < LINESZ * c; ++i)
            for (j = 0; j < 2; ++j)
            {
                p[(j * LINESZ + i) * 4 + 0] = color[chan][0];
                p[(j * LINESZ + i) * 4 + 1] = color[chan][1];
                p[(j * LINESZ + i) * 4 + 2] = color[chan][2];
                p[(j * LINESZ + i) * 4 + 3] = color[chan][3];
            }

        /* Configure the texture and specify the pixel buffer. */

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, LINESZ, 2, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, p);
        free(p);
    }
}

static void move_line_texture(int tile, const float v[3], float px)
{
    float p = get_varrier_pitch(tile);
    float a = get_varrier_angle(tile);
    float t = get_varrier_thick(tile);
    float s = get_varrier_shift(tile);

    float M[16];
    float c[3];
    float n[3];
    float w, h;
    float nn, pp;
    float dx, dy;

    get_varrier_tile(tile, M, c, n, &w, &h);

    /* Find the distance to the display. */

    nn = ((v[0] - c[0]) * n[0] +
          (v[1] - c[1]) * n[1] +
          (v[2] - c[2]) * n[2]);

    /* Compute the parallax offset due to optical thickness. */

    dx = (v[0] - n[0] * nn - c[0]) * t / nn;
    dy = (v[1] - n[1] * nn - c[1]) * t / nn;

    /* Compute the pitch reduction due to optical thickness. */

    pp = p * (nn - t) / nn;

    /* Transform the line screen texture into position. */

    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();

        glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0,
                                   1.0 / 800.0, 1.0 / 600.0, 0.0, 0.0);

        glScalef(pp, pp, 1.0);               /* Pitch in feet.    */
        glTranslatef(-s + dx - px, dy, 0);   /* Shift in feet.    */
        glRotatef(-a, 0, 0, 1);              /* Angle.            */
        glScalef(0.5 * w, 0.5 * h, 1.0);     /* Scale to feet.    */
    }
    glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------------*/

static int stereo_varrier_01(int eye, int tile, int pass, const float v[3])
{
    float px = 0.00025f;

    if (pass == 0)
    {
        if (eye == 0)
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        else
            glClear(GL_DEPTH_BUFFER_BIT);

        /* Set up the line screen texture environments. */

        if (GL_has_multitexture)
        {
            /* TU0 modulates the material RGB against the base texture,      */
            /* giving the pixel RGB, and sums (and clamps) the red and       */
            /* green line screen alpha values.                               */

            glActiveTextureARB(GL_TEXTURE0_ARB);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,      GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,      GL_TEXTURE0);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,    GL_TEXTURE1);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,    GL_TEXTURE2);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_ADD);

            /* TU1 modulates the pixel RGB against the red line screen and   */
            /* sums (and clamps) the blue linescreen alpha value with the    */
            /* red and green alpha values.                                   */

            glActiveTextureARB(GL_TEXTURE1_ARB);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,      GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,      GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,    GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,    GL_TEXTURE3);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_ADD);
            init_line_texture(tile, 0);
            move_line_texture(tile, v, +px);

            /* TU2 modulates the pixel color against the green line screen   */
            /* and modulates the accumulated line screen alpha against the   */
            /* material alpha value.                                         */

            glActiveTextureARB(GL_TEXTURE2_ARB);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,      GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,      GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,    GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,    GL_PRIMARY_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_MODULATE);
            init_line_texture(tile, 1);
            move_line_texture(tile, v, 0);

            /* TU3 modulates the pixel color against the blue line screen    */
            /* and modulates the accumulated line screen alpha against the   */
            /* base texture alpha value.                                     */

            glActiveTextureARB(GL_TEXTURE3_ARB);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,      GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,      GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,    GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,    GL_TEXTURE0);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_MODULATE);
            init_line_texture(tile, 2);
            move_line_texture(tile, v, -px);

            glActiveTextureARB(GL_TEXTURE0_ARB);
        }

        draw_tile_background(tile, DRAW_VARRIER_TEXGEN);

        return 1;
    }
    else
    {
        if (GL_has_multitexture)
        {
            glActiveTextureARB(GL_TEXTURE3_ARB);
            glDisable(GL_TEXTURE_2D);
            glActiveTextureARB(GL_TEXTURE2_ARB);
            glDisable(GL_TEXTURE_2D);
            glActiveTextureARB(GL_TEXTURE1_ARB);
            glDisable(GL_TEXTURE_2D);
            glActiveTextureARB(GL_TEXTURE0_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
    }
    return 0;
}

static int stereo_varrier_11(int eye, int tile, int pass)
{
    if (pass == 0)
    {
        float M[16];
        float c[3];
        float n[3];
        float w, h;

        get_varrier_tile(tile, M, c, n, &w, &h);

        /* Draw the line screen into the depth buffer. */

        if (eye == 0)
        {
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            glColorMask(0, 0, 0, 0);
            draw_varrier_lines(tile, M, c, w, h, 0);
            glColorMask(1, 1, 1, 1);
            draw_tile_background(tile, 0);
        }
        else
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(0, 0, 0, 0);
            draw_varrier_lines(tile, M, c, w, h, 0);
            glColorMask(1, 1, 1, 1);
            draw_tile_background(tile, 0);
        }

        return 1;
    }
    return 0;
}

static int stereo_varrier_33(int eye, int tile, int pass)
{
    float M[16];
    float c[3];
    float n[3];
    float w, h, d = 0.00025f;

    int next = 0;

    get_varrier_tile(tile, M, c, n, &w, &h);

    switch (pass)
    {
    case 0:
        if (eye == 0)
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        else
            glClear(GL_DEPTH_BUFFER_BIT);

        glColorMask(0, 0, 0, 0);
        draw_varrier_lines(tile, M, c, w, h, +d);
        glColorMask(1, 0, 0, 0);
        draw_tile_background(tile, 0);
        next = 1;
        break;
        
    case 1:
        glClear(GL_DEPTH_BUFFER_BIT);
        glColorMask(0, 0, 0, 0);
        draw_varrier_lines(tile, M, c, w, h,  0);
        glColorMask(0, 1, 0, 0);
        draw_tile_background(tile, 0);
        next = 2;
        break;
        
    case 2:
        glClear(GL_DEPTH_BUFFER_BIT);
        glColorMask(0, 0, 0, 0);
        draw_varrier_lines(tile, M, c, w, h, -d);
        glColorMask(0, 0, 1, 0);
        draw_tile_background(tile, 0);
        next = 3;
        break;
        
    case 3:
        glColorMask(1, 1, 1, 1);
        next = 0;
    }

    return next;
}

/*---------------------------------------------------------------------------*/

static int stereo_none(int eye, int tile, int pass)
{
    if (pass == 0)
        return 1;
    else
        return 0;
}

static int stereo_quad(int eye, int tile, int pass)
{
    if (pass == 0)
    {
        if (quad_stereo_status())
        {
            if (eye == 0)
                glDrawBuffer(GL_BACK_LEFT);
            else
                glDrawBuffer(GL_BACK_RIGHT);
        }

        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);
        draw_tile_background(tile, 0);

        return 1;
    }
    else
        return 0;
}

static int stereo_red_blue(int eye, int tile, int pass)
{
    if (pass == 0)
    {
        if (eye == 0)
        {
            glClear(GL_COLOR_BUFFER_BIT |
                    GL_DEPTH_BUFFER_BIT);
            draw_tile_background(tile, 0);
            glColorMask(1, 0, 0, 0);
        }
        else
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(0, 0, 1, 0);
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

int draw_pass(int mode, int eye, int tile, int pass, const float v[3])
{
    /* If stereo rendering is enabled, handle it. */

    switch (mode)
    {
    case STEREO_NONE:       return stereo_none      (eye, tile, pass);
    case STEREO_QUAD:       return stereo_quad      (eye, tile, pass);
    case STEREO_RED_BLUE:   return stereo_red_blue  (eye, tile, pass);
    case STEREO_VARRIER_01: return stereo_varrier_01(eye, tile, pass, v);
    case STEREO_VARRIER_11: return stereo_varrier_11(eye, tile, pass);
    case STEREO_VARRIER_33: return stereo_varrier_33(eye, tile, pass);
    }

    return 0;
}

