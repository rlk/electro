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

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "opengl.h"
#include "shared.h"
#include "viewport.h"

/*---------------------------------------------------------------------------*/
/* Viewport configuration                                                    */

static struct viewport  Vs;
static struct viewport *Vi;
static struct viewport *Vo;
static int              V_max = 128;
static int              V_num = 0;

/*---------------------------------------------------------------------------*/

static float viewport_X = 0.0f;
static float viewport_Y = 0.0f;
static float viewport_x = DEFAULT_X;
static float viewport_y = DEFAULT_Y;
static float viewport_w = DEFAULT_W;
static float viewport_h = DEFAULT_H;

/*---------------------------------------------------------------------------*/
/* TODO: This is not strictly correct MPI type usage.  Fix. */

void viewport_init(void)
{
    int n = mpi_size();

    if (n < V_max) n = V_max;

    Vi = (struct viewport *) calloc(n, sizeof (struct viewport));
    Vo = (struct viewport *) calloc(n, sizeof (struct viewport));

    V_max = n;
    V_num = 0;

    Vs.x =  1000000;
    Vs.y =  1000000;
    Vs.w = -1000000;
    Vs.h = -1000000;
}

void viewport_draw(void)
{
    int i;

    /* Load a projection mapping display pixels onto world space units. */

    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        glOrtho(viewport_get_x(), viewport_get_x() + viewport_get_w(),
                viewport_get_y(), viewport_get_y() + viewport_get_h(), -1, 1);
    }
    glMatrixMode(GL_MODELVIEW);
    {
        glLoadIdentity();
    }

    /* Draw all viewports to the stencil buffer. */

    glPushAttrib(GL_ENABLE_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        glBegin(GL_QUADS);
        {
            /* Draw all defined client viewports. */

            for (i = 0; i < V_num; ++i)
            {
                glVertex2f(Vi[i].x,           Vi[i].y);
                glVertex2f(Vi[i].x + Vi[i].w, Vi[i].y);
                glVertex2f(Vi[i].x + Vi[i].w, Vi[i].y + Vi[i].h);
                glVertex2f(Vi[i].x,           Vi[i].y + Vi[i].h);
            }

            /* If there are no client viewports, draw the server viewport. */

            if (V_num == 0)
            {
                glVertex2f(Vs.x,        Vs.y);
                glVertex2f(Vs.x + Vs.w, Vs.y);
                glVertex2f(Vs.x + Vs.w, Vs.y + Vs.h);
                glVertex2f(Vs.x,        Vs.y + Vs.h);
            }
        }
        glEnd();
    }
    glPopAttrib();
}

void viewport_fill(float r, float g, float b)
{
    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glColor3f(r, g, b);

        glRectf(viewport_get_x(),
                viewport_get_y(),
                viewport_get_x() + viewport_get_w(),
                viewport_get_y() + viewport_get_h());
    }
    glPopAttrib();
}

void viewport_tile(const char *name, float X, float Y,
                                     float x, float y, float w, float h)
{
    if (V_num < V_max)
    {
        strncpy(Vi[V_num].name, name, NAMELEN);

        Vi[V_num].X = X;
        Vi[V_num].Y = Y;
        Vi[V_num].x = x;
        Vi[V_num].y = y;
        Vi[V_num].w = w;
        Vi[V_num].h = h;

        V_num++;
    }

    if (x < Vs.x) Vs.x = x;
    if (y < Vs.y) Vs.y = y;

    if (x + w > Vs.x + Vs.w) Vs.w = x + w - Vs.x;
    if (y + h > Vs.y + Vs.h) Vs.h = y + h - Vs.y;
}

void viewport_sync(void)
{
    static struct viewport Vt = { "", 0, 0, DEFAULT_X, DEFAULT_Y,
                                            DEFAULT_W, DEFAULT_H };
#ifdef MPI
    size_t sz = sizeof (struct viewport);

    /* Get this client's host name.  Set a default viewport. */

    gethostname(Vt.name, NAMELEN);

    /* Gather all host names at the root. */

    mpi_assert(MPI_Gather(&Vt, sz, MPI_BYTE,
                           Vo, sz, MPI_BYTE, 0, MPI_COMM_WORLD));

    /* If this is the root, assign viewports by matching host names. */

    if (mpi_isroot())
    {
        int j;
        int k;
        int n;

        MPI_Comm_size(MPI_COMM_WORLD, &n);

        for (j = 1; j < n; j++)
            for (k = 0; k < V_num; k++)
                if (strcmp(Vo[j].name, Vi[k].name) == 0)
                {
                    /* A name matches.  Copy the viewport definition. */

                    Vo[j].X = Vi[k].X;
                    Vo[j].Y = Vi[k].Y;
                    Vo[j].x = Vi[k].x;
                    Vo[j].y = Vi[k].y;
                    Vo[j].w = Vi[k].w;
                    Vo[j].h = Vi[k].h;

                    /* Destroy the name to ensure it matches at most once. */

                    strcpy(Vi[k].name, "");

                    break;
                }
    }

    /* Scatter the assignments to all clients. */

    mpi_assert(MPI_Scatter(Vo, sz, MPI_BYTE,
                          &Vt, sz, MPI_BYTE, 0, MPI_COMM_WORLD));

#endif  /* MPI */

    if (V_num == 0)
    {
        Vs.X = 0;
        Vs.Y = 0;
        Vs.x = DEFAULT_X;
        Vs.y = DEFAULT_Y;
        Vs.w = DEFAULT_W;
        Vs.h = DEFAULT_H;
    }

    /* Apply this client's viewport. */

    if (mpi_isroot())
        viewport_set(Vs.X, Vs.Y, Vs.x, Vs.y, Vs.w, Vs.h);
    else
        viewport_set(Vt.X, Vt.Y, Vt.x, Vt.y, Vt.w, Vt.h);
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

void viewport_get(float *x, float *y, float *w, float *h)
{
    if (x) *x = viewport_x;
    if (y) *y = viewport_y;
    if (w) *w = viewport_w;
    if (h) *h = viewport_h;
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

int window_get_w(void)
{
    /* Scale the server window width down to a reasonable size. */

    if (mpi_isroot())
        return (int) (viewport_w * DEFAULT_W / viewport_w);
    else
        return (int) (viewport_w);
}

int window_get_h(void)
{
    /* Scale the server window height down to a reasonable size. */

    if (mpi_isroot())
        return (int) (viewport_h * DEFAULT_W / viewport_w);
    else
        return (int) (viewport_h);
}

/*---------------------------------------------------------------------------*/
