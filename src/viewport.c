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
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "opengl.h"
#include "shared.h"
#include "viewport.h"

#ifdef MPI
#include <mpi.h>
#endif

/*---------------------------------------------------------------------------*/
/* Viewport configuration                                                    */

static struct viewport  Vtotal;      /* Total display viewport               */
static struct viewport  Vlocal;      /* Local display viewport               */
static struct viewport *Vin;         /* Viewports input from cluster config  */
static struct viewport *Vout;        /* Viewports output to render nodes     */

static int V_max =  32;
static int V_num =   0;

/*---------------------------------------------------------------------------*/

static void set_window_pos(int X, int Y)
{
    char buf[32];

    /* SDL looks to the environment for window position. */

    sprintf(buf, "%d,%d", X, Y);
    setenv("SDL_VIDEO_WINDOW_POS", buf, 1);
}

/*---------------------------------------------------------------------------*/

void viewport_init(void)
{
    int n = mpi_size();

    if (V_max < n)
        V_max = n;

    Vin  = (struct viewport *) calloc(V_max, sizeof (struct viewport));
    Vout = (struct viewport *) calloc(V_max, sizeof (struct viewport));

    Vtotal.X = 0;
    Vtotal.Y = 0;
    Vtotal.x = INT_MAX;
    Vtotal.y = INT_MAX;
    Vtotal.w = INT_MIN;
    Vtotal.h = INT_MIN;

    V_num = 0;
}

void viewport_draw(void)
{
    int i;

    /* Load a projection mapping display pixels onto world space units. */

    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        glOrtho(Vtotal.x, Vtotal.x + Vtotal.w,
                Vtotal.y, Vtotal.y + Vtotal.h, -1, 1);
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
                glVertex2f(Vin[i].x,            Vin[i].y);
                glVertex2f(Vin[i].x + Vin[i].w, Vin[i].y);
                glVertex2f(Vin[i].x + Vin[i].w, Vin[i].y + Vin[i].h);
                glVertex2f(Vin[i].x,            Vin[i].y + Vin[i].h);
            }

            /* If there are no client viewports, draw the total viewport. */

            if (V_num == 0)
            {
                glVertex2f(Vtotal.x,            Vtotal.y);
                glVertex2f(Vtotal.x + Vtotal.w, Vtotal.y);
                glVertex2f(Vtotal.x + Vtotal.w, Vtotal.y + Vtotal.h);
                glVertex2f(Vtotal.x,            Vtotal.y + Vtotal.h);
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

        /* Fill the entire viewport with color R,G,B. */

        glColor3f(r, g, b);

        glRectf(Vtotal.x,
                Vtotal.y,
                Vtotal.x + Vtotal.w,
                Vtotal.y + Vtotal.h);
    }
    glPopAttrib();
}

void viewport_tile(const char *name, float X, float Y,
                                     float x, float y, float w, float h)
{
    /* Note a new incoming render node viewport definition. */

    if (V_num < V_max)
    {
        strncpy(Vin[V_num].name, name, NAMELEN);

        Vin[V_num].X = X;
        Vin[V_num].Y = Y;
        Vin[V_num].x = x;
        Vin[V_num].y = y;
        Vin[V_num].w = w;
        Vin[V_num].h = h;

        V_num++;
    }

    /* Expand the total viewport to include the new local viewport. */

    if (Vtotal.x > x) Vtotal.x = x;
    if (Vtotal.y > y) Vtotal.y = y;

    if (Vtotal.x + Vtotal.w < x + w) Vtotal.w = x + w - Vtotal.x;
    if (Vtotal.y + Vtotal.h < y + h) Vtotal.h = y + h - Vtotal.y;
}

void viewport_sync(void)
{
#ifdef MPI
    size_t sz = sizeof (struct viewport);
#endif

    struct viewport Vtemp;
    int root = mpi_isroot();

    /* Get this client's host name.  Set a default viewport. */

    gethostname(Vtemp.name, NAMELEN);

    Vtemp.X = 0;
    Vtemp.Y = 0;
    Vtemp.x = DEFAULT_X;
    Vtemp.y = DEFAULT_Y;
    Vtemp.w = DEFAULT_W;
    Vtemp.h = DEFAULT_H;

    /* If there are no local viewports, ensure the total viewport is valid. */

    if (V_num == 0)
    {
        Vtotal.X = 0;
        Vtotal.Y = 0;
        Vtotal.x = DEFAULT_X;
        Vtotal.y = DEFAULT_Y;
        Vtotal.w = DEFAULT_W;
        Vtotal.h = DEFAULT_H;
    }

#ifdef MPI

    /* Gather all host names at the root. */

    mpi_assert(MPI_Gather(&Vtemp, sz, MPI_BYTE,
                           Vout,  sz, MPI_BYTE, 0, MPI_COMM_WORLD));

    /* If this is the root, assign viewports by matching host names. */

    if (mpi_isroot())
    {
        int j;
        int k;
        int n;

        MPI_Comm_size(MPI_COMM_WORLD, &n);

        for (j = 1; j < n; j++)
            for (k = 0; k < V_num; k++)
                if (strcmp(Vout[j].name, Vin[k].name) == 0)
                {
                    /* A name matches.  Copy the viewport definition. */

                    Vout[j].X = Vin[k].X;
                    Vout[j].Y = Vin[k].Y;
                    Vout[j].x = Vin[k].x;
                    Vout[j].y = Vin[k].y;
                    Vout[j].w = Vin[k].w;
                    Vout[j].h = Vin[k].h;

                    /* Destroy the name to ensure it matches at most once. */

                    strcpy(Vin[k].name, "");

                    break;
                }
    }

    /* Scatter the assignments to all clients.  Broadcast the total.  */

    mpi_assert(MPI_Scatter(Vout,  sz, MPI_BYTE,
                          &Vtemp, sz, MPI_BYTE, 0, MPI_COMM_WORLD));
    mpi_assert(MPI_Bcast(&Vtotal, sz, MPI_BYTE, 0, MPI_COMM_WORLD));

#endif  /* MPI */

    /* Apply this node's viewport. */

    Vlocal.X = root ? Vtotal.X : Vtemp.X;
    Vlocal.Y = root ? Vtotal.Y : Vtemp.Y;
    Vlocal.x = root ? Vtotal.x : Vtemp.x;
    Vlocal.y = root ? Vtotal.y : Vtemp.y;
    Vlocal.w = root ? Vtotal.w : Vtemp.w;
    Vlocal.h = root ? Vtotal.h : Vtemp.h;

    set_window_pos((int) Vlocal.X, (int) Vlocal.Y);
}

/*---------------------------------------------------------------------------*/

float viewport_total_x(void)
{
    return Vtotal.x;
}

float viewport_total_y(void)
{
    return Vtotal.y;
}

float viewport_total_w(void)
{
    return Vtotal.w;
}

float viewport_total_h(void)
{
    return Vtotal.h;
}

/*---------------------------------------------------------------------------*/

float viewport_local_x(void)
{
    return Vlocal.x;
}

float viewport_local_y(void)
{
    return Vlocal.y;
}

float viewport_local_w(void)
{
    return Vlocal.w;
}

float viewport_local_h(void)
{
    return Vlocal.h;
}

/*---------------------------------------------------------------------------*/

float viewport_scale(void)
{
    /* Scale the server window width down to a reasonable size. */

    if (mpi_isroot())
        return (float) DEFAULT_W / Vtotal.w;
    else
        return 1.0f;
}

int window_w(void)
{
    return (int) (Vlocal.w * viewport_scale());
}

int window_h(void)
{
    return (int) (Vlocal.h * viewport_scale());
}

/*---------------------------------------------------------------------------*/
