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
#include <limits.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "opengl.h"
#include "vector.h"
#include "utility.h"
#include "tracker.h"
#include "display.h"
#include "matrix.h"
#include "buffer.h"
#include "event.h"

/*---------------------------------------------------------------------------*/

static float color0[3] = { 0.1f, 0.2f, 0.4f };
static float color1[3] = { 0.0f, 0.0f, 0.0f };

/*---------------------------------------------------------------------------*/

static vector_t tile;
static vector_t host;

static struct host *local = NULL;

/*---------------------------------------------------------------------------*/

void set_window_pos(int x, int y)
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

void set_window_siz(int d)
{
    static int siz[5][2] = {
        {  640,  480 },
        {  800,  600 },
        { 1024,  768 },
        { 1280, 1024 },
        { 1600, 1200 }
    };

    static int curr = 1;

    if (0 <= curr + d && curr + d < 5)
    {
        curr += d;

        set_window_w(siz[curr][0]);
        set_window_h(siz[curr][1]);
    }
}

/*---------------------------------------------------------------------------*/

static void bound_display(void)
{
    int l = INT_MAX;
    int r = INT_MIN;
    int b = INT_MAX;
    int t = INT_MIN;
    int i;

    /* Compute the total pixel size of all tiles. */

    for (i = 0; i < vecnum(tile); ++i)
    {
        struct tile *T = (struct tile *) vecget(tile, i);

        l = MIN(l, T->pix_x);
        r = MAX(r, T->pix_x + T->pix_w);
        b = MIN(b, T->pix_y);
        t = MAX(t, T->pix_y + T->pix_h);
    }

    /* Copy the total pixel size to all host configurations. */

    for (i = 0; i < vecnum(host); ++i)
    {
        struct host *H = (struct host *) vecget(host, i);

        H->tot_x = l;
        H->tot_y = b;
        H->tot_w = r - l;
        H->tot_h = t - b;
    }
}

/*---------------------------------------------------------------------------*/

void sync_display(void)
{
    char name[MAXNAME];
    int  i;

#ifdef MPI
    int num  = vecnum(host);
    int siz  = vecsiz(host);
    int rank = 0;
    int j;

    struct host *H;

    assert_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    assert_mpi(MPI_Bcast(&num, 1, MPI_INTEGER, 0, MPI_COMM_WORLD));

    /* Broadcast all host definitions to all nodes. */

    for (i = 1; i < num; i++)
        if ((j = (rank) ? vecadd(host) : i) >= 0)
        {
            H = (struct host *) vecget(host, i);

            assert_mpi(MPI_Bcast(H, siz, MPI_BYTE, 0, MPI_COMM_WORLD));

            if (rank) H->n = 0;
        }
#endif

    /* Search the definition list for an entry matching this host's name */

    if (gethostname(name, MAXNAME) == 0)
    {
        for (i = 0; i < vecnum(host); ++i)
        {
            struct host *H = (struct host *) vecget(host, i);

            if (strncmp(name, H->name, MAXNAME) == 0)
                local = H;
        }

        set_window_pos(local->win_x, local->win_y);
    }
}

/*---------------------------------------------------------------------------*/

int add_host(const char *name, int x, int y, int w, int h)
{
    int i;

    if ((i = vecadd(host)) >= 0)
    {
        struct host *H = (struct host *) vecget(host, i);

        /* Store the name for future host name searching. */

        strncpy(H->name, name, MAXNAME);

        /* The rectangle defines window size and default total display size. */

        H->tot_x = H->win_x = x;
        H->tot_y = H->win_y = y;
        H->tot_w = H->win_w = w;
        H->tot_h = H->win_h = h;
    }
    return i;
}

int add_tile(int i, int x, int y, int w, int h)
{
    int j = -1;

    if ((i < vecnum(host)) && (j = vecadd(tile)) >= 0)
    {
        struct host *H = (struct host *) vecget(host, i);
        struct tile *T = (struct tile *) vecget(tile, j);

        /* The rectangle defines viewport size and default ortho projection. */

        T->pix_x = T->win_x = x;
        T->pix_y = T->win_y = y;
        T->pix_w = T->win_w = w;
        T->pix_h = T->win_h = h;

        /* Compute a default perpective projection. */

        T->o[0]  = -0.5f;
        T->o[1]  = -0.5f * h / w;
        T->o[2]  = -1.0f;
        T->r[0]  =  1.0f;
        T->u[1]  =  1.0f * h / w;

        /* Include this tile in the host and in the total display. */

        H->tile[H->n++] = j;

        bound_display();
    }
    return j;
}

/*---------------------------------------------------------------------------*/

int send_add_tile(int i, int x, int y, int w, int h)
{
    pack_event(EVENT_ADD_TILE);
    pack_index(i);
    pack_index(x);
    pack_index(y);
    pack_index(w);
    pack_index(h);

    return add_tile(i, x, y, w, h);
}

void recv_add_tile(void)
{
    int i = unpack_index();
    int x = unpack_index();
    int y = unpack_index();
    int w = unpack_index();
    int h = unpack_index();

    add_tile(i, x, y, w, h);
}

/*---------------------------------------------------------------------------*/

void send_set_tile_flag(int i, int flags, int state)
{
    struct tile *T = (struct tile *) vecget(tile, i);

    pack_event(EVENT_SET_TILE_FLAG);
    pack_index(i);
    pack_index(flags);
    pack_index(state);

    if (state)
        T->flag = T->flag | ( flags);
    else
        T->flag = T->flag & (~flags);
}

void send_set_tile_position(int i, const float o[3],
                                   const float r[3],
                                   const float u[3])
{
    struct tile *T = (struct tile *) vecget(tile, i);

    pack_event(EVENT_SET_TILE_POSITION);
    pack_index(i);
    pack_float((T->o[0] = o[0]));
    pack_float((T->o[1] = o[1]));
    pack_float((T->o[2] = o[2]));
    pack_float((T->r[0] = r[0]));
    pack_float((T->r[1] = r[1]));
    pack_float((T->r[2] = r[2]));
    pack_float((T->u[0] = u[0]));
    pack_float((T->u[1] = u[1]));
    pack_float((T->u[2] = u[2]));
}

void send_set_tile_viewport(int i, int x, int y, int w, int h)
{
    struct tile *T = (struct tile *) vecget(tile, i);

    pack_event(EVENT_SET_TILE_VIEWPORT);
    pack_index(i);
    pack_index((T->pix_x = x));
    pack_index((T->pix_y = y));
    pack_index((T->pix_w = w));
    pack_index((T->pix_h = h));
}

void send_set_tile_line_screen(int i, float p, float a,
                                      float t, float s, float c)
{
    struct tile *T = (struct tile *) vecget(tile, i);

    pack_event(EVENT_SET_TILE_LINE_SCREEN);
    pack_index(i);
    pack_float((T->varrier_pitch = p));
    pack_float((T->varrier_angle = a));
    pack_float((T->varrier_thick = t));
    pack_float((T->varrier_shift = s));
    pack_float((T->varrier_cycle = c));
}

void send_set_tile_view_mirror(int i, const float p[4])
{
    struct tile *T = (struct tile *) vecget(tile, i);

    pack_event(EVENT_SET_TILE_VIEW_MIRROR);
    pack_index(i);
    pack_float((T->p[0] = p[0]));
    pack_float((T->p[1] = p[1]));
    pack_float((T->p[2] = p[2]));
    pack_float((T->p[3] = p[3]));
}

void send_set_tile_view_offset(int i, const float d[3])
{
    struct tile *T = (struct tile *) vecget(tile, i);

    pack_event(EVENT_SET_TILE_VIEW_OFFSET);
    pack_index(i);
    pack_float((T->d[0] = d[0]));
    pack_float((T->d[1] = d[1]));
    pack_float((T->d[2] = d[2]));
}

/*---------------------------------------------------------------------------*/

void recv_set_tile_flag(void)
{
    struct tile *T = (struct tile *) vecget(tile, unpack_index());

    int flags = unpack_index();
    int state = unpack_index();

    if (state)
        T->flag = T->flag | ( flags);
    else
        T->flag = T->flag & (~flags);
}

void recv_set_tile_position(void)
{
    struct tile *T = (struct tile *) vecget(tile, unpack_index());

    T->o[0] = unpack_float();
    T->o[1] = unpack_float();
    T->o[2] = unpack_float();
    T->r[0] = unpack_float();
    T->r[1] = unpack_float();
    T->r[2] = unpack_float();
    T->u[0] = unpack_float();
    T->u[1] = unpack_float();
    T->u[2] = unpack_float();
}

void recv_set_tile_viewport(void)
{
    struct tile *T = (struct tile *) vecget(tile, unpack_index());

    T->pix_x = unpack_index();
    T->pix_y = unpack_index();
    T->pix_w = unpack_index();
    T->pix_h = unpack_index();
}

void recv_set_tile_line_screen(void)
{
    struct tile *T = (struct tile *) vecget(tile, unpack_index());

    T->varrier_pitch = unpack_float();
    T->varrier_angle = unpack_float();
    T->varrier_thick = unpack_float();
    T->varrier_shift = unpack_float();
    T->varrier_cycle = unpack_float();
}

void recv_set_tile_view_mirror(void)
{
    struct tile *T = (struct tile *) vecget(tile, unpack_index());

    T->p[0] = unpack_float();
    T->p[1] = unpack_float();
    T->p[2] = unpack_float();
    T->p[3] = unpack_float();
}

void recv_set_tile_view_offset(void)
{
    struct tile *T = (struct tile *) vecget(tile, unpack_index());

    T->d[0] = unpack_float();
    T->d[1] = unpack_float();
    T->d[2] = unpack_float();
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

/*---------------------------------------------------------------------------*/

void set_window_w(int w)
{
    int i;

    local->win_w = w;

    for (i = 0; i < local->n; ++i)
        ((struct tile *) vecget(tile, local->tile[i]))->win_w = w;
}

void set_window_h(int h)
{
    int i;

    local->win_h = h;

    for (i = 0; i < local->n; ++i)
        ((struct tile *) vecget(tile, local->tile[i]))->win_h = h;
}

int get_window_w(void) { return local->win_w; }
int get_window_h(void) { return local->win_h; }

/*---------------------------------------------------------------------------*/

int get_viewport_x(void) { return local->tot_x; }
int get_viewport_y(void) { return local->tot_y; }
int get_viewport_w(void) { return local->tot_w; }
int get_viewport_h(void) { return local->tot_h; }

/*---------------------------------------------------------------------------*/

int view_ortho(int i, struct frustum *F)
{
    if (i < local->n)
    {
        struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

        /* Set the frustum planes. */

        F->V[0][0] =  0.0f;
        F->V[0][1] =  1.0f;
        F->V[0][2] =  0.0f;
        F->V[0][3] = (float) (-T->pix_y);

        F->V[1][0] = -1.0f;
        F->V[1][1] =  0.0f;
        F->V[1][2] =  0.0f;
        F->V[1][3] = (float) (-T->pix_x - T->pix_w);

        F->V[2][0] =  0.0f;
        F->V[2][1] = -1.0f;
        F->V[2][2] =  0.0f;
        F->V[2][3] = (float) (-T->pix_y - T->pix_h);

        F->V[3][0] =  1.0f;
        F->V[3][1] =  0.0f;
        F->V[3][2] =  0.0f;
        F->V[3][3] = (float) (-T->pix_x);

        F->p[0]    =  0.0f;
        F->p[1]    =  0.0f;
        F->p[2]    =  0.0f;
        F->p[3]    =  1.0f;

        return i + 1;
    }
    return 0;
}

int view_persp(int i, struct frustum *F, const float p[3])
{
    if (i < local->n)
    {
        struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

        float p0[3];
        float p1[3];
        float p2[3];
        float p3[3];

        /* Compute the frustum planes. */

        p0[0] = T->o[0];
        p0[1] = T->o[1];
        p0[2] = T->o[2];

        p1[0] = T->r[0] + p0[0];
        p1[1] = T->r[1] + p0[1];
        p1[2] = T->r[2] + p0[2];

        p2[0] = T->u[0] + p1[0];
        p2[1] = T->u[1] + p1[1];
        p2[2] = T->u[2] + p1[2];

        p3[0] = T->u[0] + p0[0];
        p3[1] = T->u[1] + p0[1];
        p3[2] = T->u[2] + p0[2];

        v_plane(F->V[0], p, p1, p0);
        v_plane(F->V[1], p, p2, p1);
        v_plane(F->V[2], p, p3, p2);
        v_plane(F->V[3], p, p0, p3);

        F->p[0] = 0.0f;
        F->p[1] = 0.0f;
        F->p[2] = 0.0f;
        F->p[3] = 1.0f;

        return i + 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

int draw_ortho(int i, float N, float F)
{
    if (i < local->n)
    {
        struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

        /* Configure the viewport. */

        glViewport(T->win_x, T->win_y, T->win_w, T->win_h);
        glScissor (T->win_x, T->win_y, T->win_w, T->win_h);

        /* Apply the projection. */

        glMatrixMode(GL_PROJECTION);
        {
            glLoadIdentity();
            glOrtho(T->pix_x, T->pix_x + T->pix_w,
                    T->pix_y, T->pix_y + T->pix_h, N, F);
        }
        glMatrixMode(GL_MODELVIEW);

        glLoadIdentity();

        return i + 1;
    }
    return 0;
}

int draw_persp(int i, float N, float F, const float p[3])
{
    if (i < local->n)
    {
        struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

        float r[3];
        float u[3];
        float n[3];
        float c[3];
        float k;
        float d;

        float M[16];
        float I[16];

        float p0[3];
        float p1[3];
        float p3[3];

        p0[0] = T->o[0];
        p0[1] = T->o[1];
        p0[2] = T->o[2];

        p1[0] = T->r[0] + p0[0];
        p1[1] = T->r[1] + p0[1];
        p1[2] = T->r[2] + p0[2];

        p3[0] = T->u[0] + p0[0];
        p3[1] = T->u[1] + p0[1];
        p3[2] = T->u[2] + p0[2];

        /* Configure the viewport. */

        glViewport(T->win_x, T->win_y, T->win_w, T->win_h);
        glScissor (T->win_x, T->win_y, T->win_w, T->win_h);

        /* Compute the projection. */

        r[0] = T->r[0];
        r[1] = T->r[1];
        r[2] = T->r[2];

        u[0] = T->u[0];
        u[1] = T->u[1];
        u[2] = T->u[2];

        v_cross(n, r, u);

        k = (float) sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
        r[0] /= k;
        r[1] /= k;
        r[2] /= k;
        k = (float) sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
        u[0] /= k;
        u[1] /= k;
        u[2] /= k;
        k = (float) sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        n[0] /= k;
        n[1] /= k;
        n[2] /= k;

        d = n[0] * (T->o[0] - p[0]) + 
            n[1] * (T->o[1] - p[1]) +
            n[2] * (T->o[2] - p[2]);

        c[0] = p[0] + n[0] * d;
        c[1] = p[1] + n[1] * d;
        c[2] = p[2] + n[2] * d;

        /* Apply the projection. */

        glMatrixMode(GL_PROJECTION);
        {
            GLdouble fL = -N * (r[0] * (p0[0] - c[0]) +
                                r[1] * (p0[1] - c[1]) +
                                r[2] * (p0[2] - c[2])) / d;
            GLdouble fR = -N * (r[0] * (p1[0] - c[0]) +
                                r[1] * (p1[1] - c[1]) +
                                r[2] * (p1[2] - c[2])) / d;
            GLdouble fB = -N * (u[0] * (p0[0] - c[0]) +
                                u[1] * (p0[1] - c[1]) +
                                u[2] * (p0[2] - c[2])) / d;
            GLdouble fT = -N * (u[0] * (p3[0] - c[0]) +
                                u[1] * (p3[1] - c[1]) +
                                u[2] * (p3[2] - c[2])) / d;

            glLoadIdentity();
            glFrustum(fL, fR, fB, fT, N, F);
        }
        glMatrixMode(GL_MODELVIEW);

        M[0] = r[0]; M[4] = u[0]; M[8]  = n[0]; M[12] = 0.0f;
        M[1] = r[1]; M[5] = u[1]; M[9]  = n[1]; M[13] = 0.0f;
        M[2] = r[2]; M[6] = u[2]; M[10] = n[2]; M[14] = 0.0f;
        M[3] = 0.0f; M[7] = 0.0f; M[11] = 0.0f; M[15] = 1.0f;

        m_invt(I, M);

        glLoadIdentity();
        glMultMatrixf(I);

        return i + 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void draw_background(void)
{
    int i;

    /* Compute the pixel bounds of the entire display. */

    int hL = local->tot_x;
    int hR = local->tot_x + local->tot_w;
    int hB = local->tot_y;
    int hT = local->tot_y + local->tot_h;

    glPushAttrib(GL_ENABLE_BIT   | 
                 GL_SCISSOR_BIT  |
                 GL_VIEWPORT_BIT |
                 GL_DEPTH_BUFFER_BIT);
    {
        /* Clear this entire host's framebuffer. */

        glScissor(local->win_x, local->win_y,
                  local->win_w, local->win_h);

        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glDepthMask(GL_FALSE);

        /* Iterate over all tiles of this host. */

        for (i = 0; i < local->n; ++i)
        {
            struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

            /* Confine rendering to only this tile. */

            glViewport(T->win_x, T->win_y, T->win_w, T->win_h);
            glScissor (T->win_x, T->win_y, T->win_w, T->win_h);

            /* Apply a projection that covers this tile. */

            glMatrixMode(GL_PROJECTION);
            {
                glPushMatrix();
                glLoadIdentity();
                glOrtho(T->pix_x, T->pix_x + T->pix_w,
                        T->pix_y, T->pix_y + T->pix_h, -1.0, +1.0);
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

int startup_display(void)
{
    int i, j;

    tile = vecnew(32, sizeof (struct tile));
    host = vecnew(32, sizeof (struct host));

    if (tile && host)
    {
        i = add_host("default", DEFAULT_X, DEFAULT_Y, DEFAULT_W, DEFAULT_H);
        j = add_tile(i,         DEFAULT_X, DEFAULT_Y, DEFAULT_W, DEFAULT_H);

        local = (struct host *) vecget(host, i);

        return 1;
    }
    return 0;
}
