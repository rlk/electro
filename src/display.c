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

static float color0[3] = { 0.1f, 0.2f, 0.4f };
static float color1[3] = { 0.0f, 0.0f, 0.0f };

/*---------------------------------------------------------------------------*/

static void default_host(struct host *H)
{
    float a  = (float) DEFAULT_H / (float) DEFAULT_W;

    /* Set a default configuration to be used in case of config failure. */

    H->win_w = DEFAULT_W;
    H->win_h = DEFAULT_H;
    H->n     = 1;

    H->tile[0].win_w = DEFAULT_W;
    H->tile[0].win_h = DEFAULT_H;
    H->tile[0].pix_w = DEFAULT_W;
    H->tile[0].pix_h = DEFAULT_H;

    H->tile[0].o[0]  = -0.5f;
    H->tile[0].o[1]  = -0.5f * a;
    H->tile[0].o[2]  = -1.0f;
    H->tile[0].r[0]  =  1.0f;
    H->tile[0].u[1]  =  1.0f * a;
}

/*---------------------------------------------------------------------------*/

void init_display(void)
{
    int n = 0;

#ifdef MPI
    assert_mpi(MPI_Comm_size(MPI_COMM_WORLD, &n));
#endif

    if (H_max < n)
        H_max = n;

    /* Allocate incoming and outgoing host config structures. */

    Hi = (struct host *) calloc(H_max, sizeof (struct host));
    Ho = (struct host *) calloc(H_max, sizeof (struct host));

    H_num = 0;
}

void sync_display(void)
{
    int i, j, rank = 0;

    default_host(&Host);

    /* Find the union of all host exents.  Copy to all hosts. */

    for (i = 0; i < H_num - 1; i++)
        for (j = i + 1; j < H_num; j++)
        {
            int r = Hi[j].pix_x + Hi[j].pix_w;
            int t = Hi[j].pix_y + Hi[j].pix_h;

            Hi[i].pix_x = Hi[j].pix_x = MIN(Hi[i].pix_x,     Hi[j].pix_x);
            Hi[i].pix_y = Hi[j].pix_y = MIN(Hi[i].pix_y,     Hi[j].pix_y);
            Hi[i].pix_w = Hi[j].pix_w = MAX(Hi[i].pix_w, r - Hi[i].pix_x);
            Hi[i].pix_h = Hi[j].pix_h = MAX(Hi[i].pix_h, t - Hi[i].pix_y);

        }

#ifdef MPI
    if (gethostname(Host.name, MAXNAME) == 0)
    {
        size_t sz = sizeof (struct host);
        int size;

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
                        break;
                    }

        /* Scatter the assignments to all clients. */

        assert_mpi(MPI_Scatter(Ho,   sz, MPI_BYTE,
                              &Host, sz, MPI_BYTE, 0, MPI_COMM_WORLD));
    }
#endif

    /* Assume the first host configuration is the server's. */

    if (rank == 0 && H_num > 0)
        memcpy(&Host, Hi, sizeof (struct host));
}

/*---------------------------------------------------------------------------*/

int draw_ortho(struct frustum *F1, const float p[3], float N, float F, int i)
{
    if (i < Host.n)
    {
        /* Set the frustum planes. */

        F1->V[0][0] =  0.0f;
        F1->V[0][1] =  1.0f;
        F1->V[0][2] =  0.0f;

        F1->V[1][0] = -1.0f;
        F1->V[1][1] =  0.0f;
        F1->V[1][2] =  0.0f;

        F1->V[2][0] =  0.0f;
        F1->V[2][1] = -1.0f;
        F1->V[2][2] =  0.0f;

        F1->V[3][0] =  1.0f;
        F1->V[3][1] =  0.0f;
        F1->V[3][2] =  0.0f;

        /* Configure the viewport. */

        glViewport(Host.tile[i].win_x, Host.tile[i].win_y,
                   Host.tile[i].win_w, Host.tile[i].win_h);
        glScissor (Host.tile[i].win_x, Host.tile[i].win_y,
                   Host.tile[i].win_w, Host.tile[i].win_h);

        /* Apply the projection. */

        glMatrixMode(GL_PROJECTION);
        {
            glLoadIdentity();
            glOrtho(Host.tile[i].pix_x,
                    Host.tile[i].pix_x + Host.tile[i].pix_w,
                    Host.tile[i].pix_y,
                    Host.tile[i].pix_y + Host.tile[i].pix_h, N, F);
        }
        glMatrixMode(GL_MODELVIEW);

        return i + 1;
    }
    return 0;
}

int draw_persp(struct frustum *F1, const float p[3], float N, float F, int i)
{
    if (i < Host.n)
    {
        const float *o = Host.tile[i].o;
        const float *r = Host.tile[i].r;
        const float *u = Host.tile[i].u;

        float p0[3];
        float p1[3];
        float p2[3];
        float p3[3];
        float n[3];

        n[0] = o[0] - p[0];
        n[1] = o[1] - p[1];
        n[2] = o[2] - p[2];

        /* Compute the frustum planes. */

        p0[0] = o[0];
        p0[1] = o[1];
        p0[2] = o[2];

        p1[0] = r[0] + p0[0];
        p1[1] = r[1] + p0[1];
        p1[2] = r[2] + p0[2];

        p2[0] = u[0] + p1[0];
        p2[1] = u[1] + p1[1];
        p2[2] = u[2] + p1[2];

        p3[0] = u[0] + p0[0];
        p3[1] = u[1] + p0[1];
        p3[2] = u[2] + p0[2];

        v_plane(F1->V[0], p, p1, p0);
        v_plane(F1->V[1], p, p2, p1);
        v_plane(F1->V[2], p, p3, p2);
        v_plane(F1->V[3], p, p0, p3);

        /* Configure the viewport. */

        glViewport(Host.tile[i].win_x, Host.tile[i].win_y,
                   Host.tile[i].win_w, Host.tile[i].win_h);
        glScissor (Host.tile[i].win_x, Host.tile[i].win_y,
                   Host.tile[i].win_w, Host.tile[i].win_h);

        /* Apply the projection. */

        glMatrixMode(GL_PROJECTION);
        {
            GLdouble L = -N * (n[0]         /  n[2]);
            GLdouble R = -N * (n[0] + r[0]) / (n[2] + r[2]);
            GLdouble B = -N * (n[1]         /  n[2]);
            GLdouble T = -N * (n[1] + u[1]) / (n[2] + u[2]);

            glLoadIdentity();
            glFrustum(L, R, B, T, N, F);
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

void add_host(const char *name, int x, int y, int w, int h)
{
    if (H_num < H_max)
    {
        strncpy(Hi[H_num].name, name, MAXNAME);

        Hi[H_num].win_x = x;
        Hi[H_num].win_y = y;
        Hi[H_num].win_w = w;
        Hi[H_num].win_h = h;
        Hi[H_num].n     = 0;

        H_num++;
    }
}

void add_tile(const char *name, int x, int y, int w, int h,
                                int X, int Y, int W, int H, float p[3][3])
{
    int i = get_host(name);

    if (0 <= i && i < H_num)
    {
        int n = Hi[i].n++;

        /* Add a new tile to this host using the given configuration. */

        Hi[i].tile[n].win_x = x;
        Hi[i].tile[n].win_y = y;
        Hi[i].tile[n].win_w = w;
        Hi[i].tile[n].win_h = h;

        Hi[i].tile[n].o[0]  = p[0][0];
        Hi[i].tile[n].o[1]  = p[0][1];
        Hi[i].tile[n].o[2]  = p[0][2];

        Hi[i].tile[n].r[0]  = p[1][0];
        Hi[i].tile[n].r[1]  = p[1][1];
        Hi[i].tile[n].r[2]  = p[1][2];

        Hi[i].tile[n].u[0]  = p[2][0];
        Hi[i].tile[n].u[1]  = p[2][1];
        Hi[i].tile[n].u[2]  = p[2][2];

        Hi[i].tile[n].pix_x = X;
        Hi[i].tile[n].pix_y = Y;
        Hi[i].tile[n].pix_w = W;
        Hi[i].tile[n].pix_h = H;

        /* Compute the total pixel size of all tiles of this host. */

        if (n == 0)
        {
            Hi[i].pix_x = X;
            Hi[i].pix_y = Y;
            Hi[i].pix_w = W;
            Hi[i].pix_h = H;
        }
        else
        {
            Hi[i].pix_x = MIN(Hi[i].pix_x, X);
            Hi[i].pix_y = MIN(Hi[i].pix_y, Y);
            Hi[i].pix_w = MAX(Hi[i].pix_w, X + W - Hi[i].pix_x);
            Hi[i].pix_h = MAX(Hi[i].pix_h, Y + H - Hi[i].pix_y);
        }
    }
}

/*---------------------------------------------------------------------------*/

int get_window_w(void)
{
    return Host.win_w;
}

int get_window_h(void)
{
    return Host.win_h;
}

int get_viewport_x(void)
{
    return Host.pix_x;
}

int get_viewport_y(void)
{
    return Host.pix_y;
}

int get_viewport_w(void)
{
    return Host.pix_w;
}

int get_viewport_h(void)
{
    return Host.pix_h;
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
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glDepthMask(GL_FALSE);

        glMatrixMode(GL_PROJECTION);
        {
            glPushMatrix();
            glLoadIdentity();
            glOrtho(Host.pix_x, Host.pix_x + Host.pix_w,
                    Host.pix_y, Host.pix_y + Host.pix_h, -1.0, +1.0);
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPushMatrix();
            glLoadIdentity();
        }

        glBegin(GL_QUADS);
        {
            glColor3fv(color0);
            glVertex2i(Host.win_x,              Host.win_y);
            glVertex2i(Host.win_x + Host.win_w, Host.win_y);

            glColor3fv(color1);
            glVertex2i(Host.win_x + Host.win_w, Host.win_y + Host.win_h);
            glVertex2i(Host.win_x,              Host.win_y + Host.win_h);
        }
        glEnd();

        glMatrixMode(GL_PROJECTION);
        {
            glPopMatrix();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPopMatrix();
        }
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/
