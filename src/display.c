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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opengl.h"
#include "utility.h"
#include "display.h"
#include "matrix.h"
#include "buffer.h"
#include "event.h"

/*---------------------------------------------------------------------------*/

#define HMAXINIT 32

static struct host  Host;
static struct host *Hi;
static struct host *Ho;
static int          H_num =  0;
static int          H_max = 64;

static float color0[3] = { 0.0f, 0.0f, 0.0f };
static float color1[3] = { 0.1f, 0.2f, 0.4f };

/*---------------------------------------------------------------------------*/

#ifdef SNIP
static void set_window_pos(int X, int Y)
{
    char buf[32];

    /* SDL looks to the environment for window position. */

#ifdef _WIN32
    sprintf(buf, "SDL_VIDEO_WINDOW_POS=%d,%d", X, Y);
    putenv(buf);
#else
    sprintf(buf, "%d,%d", X, Y);
    setenv("SDL_VIDEO_WINDOW_POS", buf, 1);
#endif
}
#endif

/*---------------------------------------------------------------------------*/

void init_display(void)
{
    int n = 0;

#ifdef MPI
    assert_mpi(MPI_Comm_size(MPI_COMM_WORLD, &n));
#endif

    if (H_max < n)
        H_max = n;

    Hi = (struct host *) calloc(H_max, sizeof (struct host));
    Ho = (struct host *) calloc(H_max, sizeof (struct host));

    H_num = 0;
}

void sync_display(void)
{
    int rank = 0;

#ifdef MPI

    if (gethostname(Host.name, MAXNAME) == 0)
    {
        size_t sz = sizeof (struct host);
        int i, j, size;

        assert_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
        assert_mpi(MPI_Comm_size(MPI_COMM_WORLD, &size));

        /* Gather all host names at the root. */

        assert_mpi(MPI_Gather(&Host, sz, MPI_BYTE,
                               Ho,   sz, MPI_BYTE, 0, MPI_COMM_WORLD));

        /* The root assigns tiles by matching host names. */

        if (rank == 0)
            for (i = 0; i < size; i++)
                for (j = 0; j < H_num; j++)
                    if (strcmp(Hi[j].name, Ho[i].name) == 0)
                    {
                        memcpy(Ho + i, Hi + j, sizeof (struct host));
                        strcpy(Hi[j].name,  "");

                        break;
                    }

        /* Scatter the assignments to all clients. */

        assert_mpi(MPI_Scatter(Ho,   sz, MPI_BYTE,
                              &Host, sz, MPI_BYTE, 0, MPI_COMM_WORLD));
    }
#endif

    if (rank == 0) memcpy(&Host, Hi, sizeof (struct host));
}

int draw_display(struct frustum *F1, const float p[3], float N, float F, int i)
{
    const float x[3] = { 1.0f, 0.0f, 0.0f };
    const float y[3] = { 0.0f, 1.0f, 0.0f };

    if (i < Host.n)
    {
        GLdouble l, r, b, t;

        float n0[3];
        float n1[3];

        n0[0] = Host.tile[i].o[0] - p[0];
        n0[1] = Host.tile[i].o[1] - p[1];
        n0[2] = Host.tile[i].o[2] - p[2];

        n1[0] = n0[0] + Host.tile[i].r[0] + Host.tile[i].u[0];
        n1[1] = n0[1] + Host.tile[i].r[1] + Host.tile[i].u[1];
        n1[2] = n0[2] + Host.tile[i].r[2] + Host.tile[i].u[2];

        /* Compute the frustum extents. */

        l = -N *  n0[0] / n0[2];
        r = -N * (n0[0] + Host.tile[i].r[0]) / (n0[2] + Host.tile[i].r[2]);
        b = -N *  n0[1] / n0[2];
        t = -N * (n0[1] + Host.tile[i].u[1]) / (n0[2] + Host.tile[i].u[2]);

        /* Compute the frustum planes. */

        v_cross(F1->V[0], x, n0);
        v_cross(F1->V[1], n1, x);
        v_cross(F1->V[2], n0, y);
        v_cross(F1->V[3], y, n1);

        F1->V[0][3] = F1->V[1][3] = F1->V[2][3] = F1->V[3][3] = 0.0f;

        /* Configure the viewport. */

        glViewport(Host.tile[i].x, Host.tile[i].y,
                   Host.tile[i].w, Host.tile[i].h);
        glScissor (Host.tile[i].x, Host.tile[i].y,
                   Host.tile[i].w, Host.tile[i].h);

        /* Apply the projection. */

        glMatrixMode(GL_PROJECTION);
        {
            glLoadIdentity();
            glFrustum(l, r, b, t, N, F);
        }
        glMatrixMode(GL_MODELVIEW);

        return i + 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static int get_host(const char *name)
{
    int hd;

    for (hd = 0; hd < H_max; hd++)
        if (strncmp(Hi[hd].name, name, MAXNAME) == 0)
            return hd;

    return -1;
}

void add_host(const char *name, int X, int Y, int W, int H)
{
    if (H_num < H_max)
    {
        strncpy(Hi[H_num].name, name, MAXNAME);

        Hi[H_num].X = X;
        Hi[H_num].Y = Y;
        Hi[H_num].W = W;
        Hi[H_num].H = H;
        Hi[H_num].n = 0;

        H_num++;
    }
}

void add_tile(const char *name, int x, int y, int w, int h, const float o[3],
                                                            const float r[3],
                                                            const float u[3])
{
    int i = get_host(name);

    if (0 <= i && i < H_num)
    {
        int n = Hi[i].n;

        Hi[i].tile[n].x    = x;
        Hi[i].tile[n].y    = y;
        Hi[i].tile[n].w    = w;
        Hi[i].tile[n].h    = h;

        Hi[i].tile[n].o[0] = o[0];
        Hi[i].tile[n].o[1] = o[1];
        Hi[i].tile[n].o[2] = o[2];

        Hi[i].tile[n].r[0] = r[0];
        Hi[i].tile[n].r[1] = r[1];
        Hi[i].tile[n].r[2] = r[2];

        Hi[i].tile[n].u[0] = u[0];
        Hi[i].tile[n].u[1] = u[1];
        Hi[i].tile[n].u[2] = u[2];

        Hi[i].n++;
    }
}

/*---------------------------------------------------------------------------*/

int get_window_w(void)
{
    return Host.W;
}

int get_window_h(void)
{
    return Host.H;
}

/*---------------------------------------------------------------------------*/

void send_set_background(const float c0[3], const float c1[3])
{
    pack_event(EVENT_SET_BACKGROUND);

    pack_float((color0[0] = c0[0]));
    pack_float((color0[1] = c0[1]));
    pack_float((color0[2] = c0[2]));

    pack_float((color1[0] = c1[0]));
    pack_float((color1[1] = c1[1]));
    pack_float((color1[2] = c1[2]));
}

void recv_set_background(void)
{
    color0[0] = unpack_float();
    color0[1] = unpack_float();
    color0[2] = unpack_float();

    color1[0] = unpack_float();
    color1[1] = unpack_float();
    color1[2] = unpack_float();
}

void draw_background(void)
{
}

/*---------------------------------------------------------------------------*/
