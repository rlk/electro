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
#include "tracker.h"
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

    H->win_x = DEFAULT_X;
    H->win_y = DEFAULT_Y;
    H->win_w = DEFAULT_W;
    H->win_h = DEFAULT_H;
    H->pix_w = DEFAULT_W;
    H->pix_h = DEFAULT_H;
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

static void set_window_pos(int x, int y)
{
    char buf[32];

    /* SDL looks to the environment for window position. */

#ifdef _WIN32
    sprintf(buf, "SDL_VIDEO_WINDOW_POS=%d,%d", x, y);
    putenv(buf);
#else
    sprintf(buf, "%d,%d", x, y);
    setenv("SDL_VIDEO_WINDOW_POS", buf, 1);
#endif
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

    default_host(&Host);
}

void sync_display(void)
{
    int i, j, rank = 0;

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

    if (rank) set_window_pos(Host.win_x, Host.win_y);
}

/*---------------------------------------------------------------------------*/

int draw_ortho(struct frustum *F1, float N, float F, int i)
{
    if (i < Host.n)
    {
        /* Set the frustum planes. */

        F1->V[0][0] =  0.0f;
        F1->V[0][1] =  1.0f;
        F1->V[0][2] =  0.0f;
        F1->V[0][3] = (float) (-Host.tile[i].pix_y);

        F1->V[1][0] = -1.0f;
        F1->V[1][1] =  0.0f;
        F1->V[1][2] =  0.0f;
        F1->V[1][3] = (float) (-Host.tile[i].pix_x - Host.tile[i].pix_w);

        F1->V[2][0] =  0.0f;
        F1->V[2][1] = -1.0f;
        F1->V[2][2] =  0.0f;
        F1->V[2][3] = (float) (-Host.tile[i].pix_y - Host.tile[i].pix_h);

        F1->V[3][0] =  1.0f;
        F1->V[3][1] =  0.0f;
        F1->V[3][2] =  0.0f;
        F1->V[3][3] = (float) (-Host.tile[i].pix_x);

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

        glLoadIdentity();

        return i + 1;
    }
    return 0;
}

int draw_persp(struct frustum *F1, const float p[3], float fN, float fF, int i)
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

        float R[3];
        float U[3];
        float N[3];
        float c[3];
        float k;
        float d;

        float M[16];
        float I[16];

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

        /* Compute the projection. */

        R[0] = Host.tile[i].r[0];
        R[1] = Host.tile[i].r[1];
        R[2] = Host.tile[i].r[2];

        U[0] = Host.tile[i].u[0];
        U[1] = Host.tile[i].u[1];
        U[2] = Host.tile[i].u[2];

        v_cross(N, R, U);

        k = (float) sqrt(R[0] * R[0] + R[1] * R[1] + R[2] * R[2]);
        R[0] /= k;
        R[1] /= k;
        R[2] /= k;
        k = (float) sqrt(U[0] * U[0] + U[1] * U[1] + U[2] * U[2]);
        U[0] /= k;
        U[1] /= k;
        U[2] /= k;
        k = (float) sqrt(N[0] * N[0] + N[1] * N[1] + N[2] * N[2]);
        N[0] /= k;
        N[1] /= k;
        N[2] /= k;

        d = N[0] * (o[0] - p[0]) + 
            N[1] * (o[1] - p[1]) +
            N[2] * (o[2] - p[2]);

        c[0] = p[0] + N[0] * d;
        c[1] = p[1] + N[1] * d;
        c[2] = p[2] + N[2] * d;

        /* Apply the projection. */

        glMatrixMode(GL_PROJECTION);
        {
            GLdouble fL = -fN * (R[0] * (p0[0] - c[0]) +
                                 R[1] * (p0[1] - c[1]) +
                                 R[2] * (p0[2] - c[2])) / d;
            GLdouble fR = -fN * (R[0] * (p1[0] - c[0]) +
                                 R[1] * (p1[1] - c[1]) +
                                 R[2] * (p1[2] - c[2])) / d;
            GLdouble fB = -fN * (U[0] * (p0[0] - c[0]) +
                                 U[1] * (p0[1] - c[1]) +
                                 U[2] * (p0[2] - c[2])) / d;
            GLdouble fT = -fN * (U[0] * (p3[0] - c[0]) +
                                 U[1] * (p3[1] - c[1]) +
                                 U[2] * (p3[2] - c[2])) / d;

            glLoadIdentity();
            glFrustum(fL, fR, fB, fT, fN, fF);
        }
        glMatrixMode(GL_MODELVIEW);

        M[0] = R[0]; M[4] = U[0]; M[8]  = N[0]; M[12] = 0.0f;
        M[1] = R[1]; M[5] = U[1]; M[9]  = N[1]; M[13] = 0.0f;
        M[2] = R[2]; M[6] = U[2]; M[10] = N[2]; M[14] = 0.0f;
        M[3] = 0.0f; M[7] = 0.0f; M[11] = 0.0f; M[15] = 1.0f;

        m_invt(I, M);

        glLoadIdentity();
        glMultMatrixf(I);

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

void set_window_w(int w)
{
    Host.win_w = Host.tile[0].win_w = w;
}

void set_window_h(int h)
{
    Host.win_h = Host.tile[0].win_h = h;
}

int get_window_w(void)
{
    return Host.win_w;
}

int get_window_h(void)
{
    return Host.win_h;
}

/*---------------------------------------------------------------------------*/

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
    int i;

    /* Compute the pixel bounds of the entire display. */

    int hL = Host.pix_x;
    int hR = Host.pix_x + Host.pix_w;
    int hB = Host.pix_y;
    int hT = Host.pix_y + Host.pix_h;

    glPushAttrib(GL_ENABLE_BIT   | 
                 GL_SCISSOR_BIT  |
                 GL_VIEWPORT_BIT |
                 GL_DEPTH_BUFFER_BIT);
    {
        /* Clear this entire host's framebuffer. */

        glScissor(Host.win_x, Host.win_y,
                  Host.win_w, Host.win_h);

        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glDepthMask(GL_FALSE);

        /* Iterate over all tiles of this host. */

        for (i = 0; i < Host.n; ++i)
        {
            /* Compute the pixel bounds of this tile. */

            int tL = Host.tile[i].pix_x;
            int tR = Host.tile[i].pix_x + Host.tile[i].pix_w;
            int tB = Host.tile[i].pix_y;
            int tT = Host.tile[i].pix_y + Host.tile[i].pix_h;

            /* Confine rendering to only this tile. */

            glViewport(Host.tile[i].win_x, Host.tile[i].win_y,
                       Host.tile[i].win_w, Host.tile[i].win_h);
            glScissor (Host.tile[i].win_x, Host.tile[i].win_y,
                       Host.tile[i].win_w, Host.tile[i].win_h);

            /* Apply a projection that covers this tile. */

            glMatrixMode(GL_PROJECTION);
            {
                glPushMatrix();
                glLoadIdentity();
                glOrtho(tL, tR, tB, tT, -1.0, +1.0);
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glPushMatrix();
                glLoadIdentity();
            }

            /* Render a background quad over the entire display. */

            glBegin(GL_QUADS);
            {
                glColor3fv(color0);
                glVertex2i(hL, hB);
                glVertex2i(hR, hB);

                glColor3fv(color1);
                glVertex2i(hR, hT);
                glVertex2i(hL, hT);
            }
            glEnd();

            /* Revert the original projection. */

            glMatrixMode(GL_PROJECTION);
            {
                glPopMatrix();
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glPopMatrix();
            }
        }
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/
