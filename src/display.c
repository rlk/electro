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
#include "video.h"

#define swap(a, b) { double t; t = (a); (a) = (b); (b) = t; }

/*---------------------------------------------------------------------------*/

static float color0[3] = { 0.1f, 0.2f, 0.4f };
static float color1[3] = { 0.0f, 0.0f, 0.0f };

/*---------------------------------------------------------------------------*/

static vector_t tile;
static vector_t host;

static struct host *local = NULL;

static int display_x = DEFAULT_X;
static int display_y = DEFAULT_Y;
static int display_w = DEFAULT_W;
static int display_h = DEFAULT_H;

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

    display_x = l;
    display_y = b;
    display_w = r - l;
    display_h = t - b;

    /* Copy the total pixel size to all host configurations. */

    for (i = 0; i < vecnum(host); ++i)
    {
        struct host *H = (struct host *) vecget(host, i);

        H->tot_x = display_x;
        H->tot_y = display_y;
        H->tot_w = display_w;
        H->tot_h = display_h;
    }
}

/*---------------------------------------------------------------------------*/

int add_host(const char *name, int x, int y, int w, int h)
{
    int i;

    if ((i = vecadd(host)) >= 0)
    {
        struct host *H = (struct host *) vecget(host, i);

        H->flag  = 0;

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

        T->flag = 0;

        /* The rectangle defines viewport size and default ortho projection. */

        T->pix_x = T->win_x = x;
        T->pix_y = T->win_y = y;
        T->pix_w = T->win_w = w;
        T->pix_h = T->win_h = h;

        /* Set a default display configuration. */

        T->o[0] = DEFAULT_OX;
        T->o[1] = DEFAULT_OY;
        T->o[2] = DEFAULT_OZ;
        T->r[0] = DEFAULT_RX;
        T->r[1] = DEFAULT_RY;
        T->r[2] = DEFAULT_RZ;
        T->u[0] = DEFAULT_UX;
        T->u[1] = DEFAULT_UY;
        T->u[2] = DEFAULT_UZ;

        T->varrier_pitch = DEFAULT_VARRIER_PITCH;
        T->varrier_angle = DEFAULT_VARRIER_ANGLE;
        T->varrier_thick = DEFAULT_VARRIER_THICK;
        T->varrier_shift = DEFAULT_VARRIER_SHIFT;
        T->varrier_cycle = DEFAULT_VARRIER_CYCLE;

        /* Include this tile in the host and in the total display. */

        H->tile[H->n++] = j;

        bound_display();
    }
    return j;
}

/*---------------------------------------------------------------------------*/

void sync_display(void)
{
    char name[MAXNAME];
    int  rank = 0;
    int  i;

#ifdef MPI
    int num  = vecnum(host);
    int siz  = vecsiz(host);
    int j;

    struct host *H;

    assert_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    assert_mpi(MPI_Bcast(&num, 1, MPI_INTEGER, 0, MPI_COMM_WORLD));

    /* Broadcast all host definitions to all nodes. */

    for (i = 0; i < num; i++)
        if ((j = (rank) ? vecadd(host) : i) >= 0)
        {
            H = (struct host *) vecget(host, i);

            assert_mpi(MPI_Bcast(H, siz, MPI_BYTE, 0, MPI_COMM_WORLD));

            if (rank) H->n = 0;
        }
#endif

    /* Search the definition list for an entry matching this host's name */

    if (gethostname(name, MAXNAME) == 0)
        for (i = 0; i < vecnum(host); ++i)
        {
            struct host *H = (struct host *) vecget(host, i);

            if (strncmp(H->name, name,         MAXNAME) == 0 ||
                strncmp(H->name, DEFAULT_NAME, MAXNAME) == 0)
                local = H;
        }

    /* If no host definition was found, use a default. */

    if (local == NULL)
    {
        int i, j;

        i = add_host(DEFAULT_NAME, DEFAULT_X, DEFAULT_Y, DEFAULT_W, DEFAULT_H);
        j = add_tile(i,            DEFAULT_X, DEFAULT_Y, DEFAULT_W, DEFAULT_H);

        local = (struct host *) vecget(host, i);

        local->flag = HOST_FRAMED;
    }

    if (rank) set_window_pos(local->win_x, local->win_y);
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

void send_set_host_flag(int i, int flags, int state)
{
    struct host *H = (struct host *) vecget(host, i);

    /* Host flags are only used at host creation time, so there's no point   */
    /* in sending them off to already-initialized hosts.                     */

    if (state)
        H->flag = H->flag | ( flags);
    else
        H->flag = H->flag & (~flags);
}

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

    bound_display();
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

    bound_display();
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

    if (local)
    {
        local->win_w = w;

        for (i = 0; i < local->n; ++i)
            ((struct tile *) vecget(tile, local->tile[i]))->win_w = w;
    }
}

void set_window_h(int h)
{
    int i;

    if (local)
    {
        local->win_h = h;

        for (i = 0; i < local->n; ++i)
            ((struct tile *) vecget(tile, local->tile[i]))->win_h = h;
    }
}

int get_window_w(void)   { return local ? local->win_w : DEFAULT_W; }
int get_window_h(void)   { return local ? local->win_h : DEFAULT_H; }

int get_window_full(void)
{
	return local ? (local->flag & HOST_FULL) : 0;
}

int get_window_stereo(void)
{
	return local ? (local->flag & HOST_STEREO) : 0;
}

int get_window_framed(void)
{
	return local ? (local->flag & HOST_FRAMED) : 1;
}

/*---------------------------------------------------------------------------*/

int get_viewport_x(void) { return local ? local->tot_x : display_x; }
int get_viewport_y(void) { return local ? local->tot_y : display_y; }
int get_viewport_w(void) { return local ? local->tot_w : display_w; }
int get_viewport_h(void) { return local ? local->tot_h : display_h; }
int get_tile_count(void) { return local ? local->n : 1; }

void get_tile_o(int i, float o[3])
{
    if (local)
    {
        struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

        o[0] = T->o[0];
        o[1] = T->o[1];
        o[2] = T->o[2];
    }
    else
    {
        o[0] = DEFAULT_OX;
        o[1] = DEFAULT_OY;
        o[2] = DEFAULT_OZ;
    }
}

void get_tile_r(int i, float r[3])
{
    if (local)
    {
        struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

        r[0] = T->r[0];
        r[1] = T->r[1];
        r[2] = T->r[2];
    }
    else
    {
        r[0] = DEFAULT_RX;
        r[1] = DEFAULT_RY;
        r[2] = DEFAULT_RZ;
    }
}

void get_tile_u(int i, float u[3])
{
    if (local)
    {
        struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

        u[0] = T->u[0];
        u[1] = T->u[1];
        u[2] = T->u[2];
    }
    else
    {
        u[0] = DEFAULT_UX;
        u[1] = DEFAULT_UY;
        u[2] = DEFAULT_UZ;
    }
}

void get_tile_n(int i, float n[3])
{
    float r[3];
    float u[3];

    get_tile_r(i, r);
    get_tile_u(i, u);

    cross(n, r, u);
    normalize(n);
}

int get_tile_flag(int i)
{
    return local ? ((struct tile *) vecget(tile, local->tile[i]))->flag : 0;
}

float get_varrier_pitch(int i)
{
    if (local)
        return ((struct tile *) vecget(tile, local->tile[i]))->varrier_pitch;
    else
        return DEFAULT_VARRIER_PITCH;
}

float get_varrier_angle(int i)
{
    if (local)
        return ((struct tile *) vecget(tile, local->tile[i]))->varrier_angle;
    else
        return DEFAULT_VARRIER_ANGLE;
}

float get_varrier_thick(int i)
{
    if (local)
        return ((struct tile *) vecget(tile, local->tile[i]))->varrier_thick;
    else
        return DEFAULT_VARRIER_THICK;
}

float get_varrier_shift(int i)
{
    if (local)
        return ((struct tile *) vecget(tile, local->tile[i]))->varrier_shift;
    else
        return DEFAULT_VARRIER_SHIFT;
}

float get_varrier_cycle(int i)
{
    if (local)
        return ((struct tile *) vecget(tile, local->tile[i]))->varrier_cycle;
    else
        return DEFAULT_VARRIER_CYCLE;
}

/*---------------------------------------------------------------------------*/

int draw_ortho(int i, float N, float F)
{
    if (local && i < local->n)
    {
        struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

        GLdouble fL = T->pix_x;
        GLdouble fR = T->pix_x + T->pix_w;
        GLdouble fB = T->pix_y;
        GLdouble fT = T->pix_y + T->pix_h;

        /* Flip the projection if requested. */

        if (T->flag & TILE_FLIP_X) swap(fL, fR);
        if (T->flag & TILE_FLIP_Y) swap(fB, fT);

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

        /* Rewind polygons if necessary. */

        if (((T->flag & TILE_FLIP_X) ? 1 : 0) ^
            ((T->flag & TILE_FLIP_Y) ? 1 : 0))
            glFrontFace(GL_CW);
        else
            glFrontFace(GL_CCW);

        return i + 1;
    }
    return 0;
}

int draw_persp(int i, float N, float F, const float p[3])
{
    if (local && i < local->n)
    {
        struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

        float P[3];
        float r[3];
        float u[3];
        float n[3];
        float c[3];
        float k;

        float M[16];
        float I[16];

        float p0[3];
        float p1[3];
        float p3[3];

        /* Compute the view position. */

        P[0]  = T->d[0] + p[0];
        P[1]  = T->d[1] + p[1];
        P[2]  = T->d[2] + p[2];

        /* Optionally reflect the view position across the mirror. */

        if (T->flag & TILE_MIRROR)
        {
            float k = (P[0] * T->p[0] +
                       P[1] * T->p[1] +
                       P[2] * T->p[2]) - T->p[3];

            P[0] -= T->p[0] * k * 2;
            P[1] -= T->p[1] * k * 2;
            P[2] -= T->p[2] * k * 2;
        }

        /* Compute the screen corners. */

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

        cross(n, r, u);
        normalize(r);
        normalize(u);
        normalize(n);

        k = n[0] * (T->o[0] - P[0]) + 
            n[1] * (T->o[1] - P[1]) +
            n[2] * (T->o[2] - P[2]);

        c[0] = P[0] + n[0] * k;
        c[1] = P[1] + n[1] * k;
        c[2] = P[2] + n[2] * k;

        glMatrixMode(GL_PROJECTION);
        {
            double fL = N * (r[0] * (P[0] - p0[0]) +
                             r[1] * (P[1] - p0[1]) +
                             r[2] * (P[2] - p0[2])) / k;
            double fR = N * (r[0] * (P[0] - p1[0]) +
                             r[1] * (P[1] - p1[1]) +
                             r[2] * (P[2] - p1[2])) / k;
            double fB = N * (u[0] * (P[0] - p0[0]) +
                             u[1] * (P[1] - p0[1]) +
                             u[2] * (P[2] - p0[2])) / k;
            double fT = N * (u[0] * (P[0] - p3[0]) +
                             u[1] * (P[1] - p3[1]) +
                             u[2] * (P[2] - p3[2])) / k;

            /* Flip the projection if requested. */

            if (T->flag & TILE_FLIP_X) swap(fL, fR);
            if (T->flag & TILE_FLIP_Y) swap(fB, fT);

            /* Apply the projection. */

            glLoadIdentity();
            glFrustum(fL, fR, fB, fT, N, F);
        }
        glMatrixMode(GL_MODELVIEW);

        M[0] = r[0]; M[4] = u[0]; M[8]  = n[0]; M[12] = 0.0f;
        M[1] = r[1]; M[5] = u[1]; M[9]  = n[1]; M[13] = 0.0f;
        M[2] = r[2]; M[6] = u[2]; M[10] = n[2]; M[14] = 0.0f;
        M[3] = 0.0f; M[7] = 0.0f; M[11] = 0.0f; M[15] = 1.0f;

        load_inv(I, M);

        glLoadIdentity();
        glMultMatrixf(I);
        glTranslatef(-T->d[0], -T->d[1], -T->d[2]);

        /* Rewind polygons if necessary. */

        if (((T->flag & TILE_FLIP_X) ? 1 : 0) ^
            ((T->flag & TILE_FLIP_Y) ? 1 : 0))
            glFrontFace(GL_CW);
        else
            glFrontFace(GL_CCW);

        return i + 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void draw_tile_background(int i)
{
    /* Incur this fill penalty only if necessary. */

    if (color0[0] != CLEAR_R || color1[0] != CLEAR_R ||
        color0[1] != CLEAR_G || color1[1] != CLEAR_G ||
        color0[2] != CLEAR_B || color1[2] != CLEAR_B)
    {
        struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

        /* Compute the beginning and end of this tile's gradiant. */

        float k0 = (float) (T->pix_y            - local->tot_y) / local->tot_h;
        float k1 = (float) (T->pix_y + T->pix_h - local->tot_y) / local->tot_h;

        /* Confine rendering to this tile. */

        glViewport(T->win_x, T->win_y, T->win_w, T->win_h);
        glScissor (T->win_x, T->win_y, T->win_w, T->win_h);

        /* Map the tile onto the unit cube. */

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 1, 0, 1, 0, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        /* Fill the tile at the far plane using the computed gradient. */

        glPushAttrib(GL_ENABLE_BIT);
        {
            float l = (T->flag & TILE_FLIP_X) ? 1 : 0;
            float r = (T->flag & TILE_FLIP_X) ? 0 : 1;
            float b = (T->flag & TILE_FLIP_Y) ? 1 : 0;
            float t = (T->flag & TILE_FLIP_Y) ? 0 : 1;

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_CULL_FACE);

            set_texture_coordinates();

            glBegin(GL_QUADS);
            {
                glColor3f(color0[0] * (1 - k0) + color1[0] * k0,
                          color0[1] * (1 - k0) + color1[1] * k0,
                          color0[2] * (1 - k0) + color1[2] * k0);
                glVertex3f(l, b, -1);
                glVertex3f(r, b, -1);
                
                glColor3f(color0[0] * (1 - k1) + color1[0] * k1,
                          color0[1] * (1 - k1) + color1[1] * k1,
                          color0[2] * (1 - k1) + color1[2] * k1);
                glVertex3f(r, t, -1);
                glVertex3f(l, t, -1);
            }
            glEnd();
        }
        glPopAttrib();

        /* Revert to the previous transformation. */

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
}

void draw_host_background(void)
{
    int i;

    glViewport(local->win_x, local->win_y,
               local->win_w, local->win_h);
    glScissor (local->win_x, local->win_y,
               local->win_w, local->win_h);

    glClear(GL_DEPTH_BUFFER_BIT |
            GL_COLOR_BUFFER_BIT);

    for (i = 0; i < local->n; ++i)
        draw_tile_background(i);
}

/*---------------------------------------------------------------------------*/

static void set_active_texture_coordinates(const float S[4],
                                           const float T[4],
                                           const float R[4],
                                           const float Q[4])
{
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);

#ifdef TEXGEN_EYE
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

    glTexGenfv(GL_S, GL_EYE_PLANE, S);
    glTexGenfv(GL_T, GL_EYE_PLANE, T);
    glTexGenfv(GL_R, GL_EYE_PLANE, R);
    glTexGenfv(GL_Q, GL_EYE_PLANE, Q);
#else
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    glTexGenfv(GL_S, GL_OBJECT_PLANE, S);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, T);
    glTexGenfv(GL_R, GL_OBJECT_PLANE, R);
    glTexGenfv(GL_Q, GL_OBJECT_PLANE, Q);
#endif
}

void set_texture_coordinates(void)
{
    if (GL_has_multitexture)
    {
        float P[16], M[16], X[16], S[4], T[4], R[4], Q[4];

        /* Supply the product of the projection and modelview matrices as    */
        /* plane coefficients.  This will transform vertices to normalized   */
        /* device coordinates, which we can apply as screen-space texture    */
        /* coordinates.                                                      */

        glGetFloatv(GL_PROJECTION_MATRIX, P);
        glGetFloatv(GL_MODELVIEW_MATRIX,  M);

        mult_mat_mat(X, P, M);

#ifdef TEXGEN_EYE
        S[0] = X[0];  S[1] = X[1];  S[2] = X[2];  S[3] = X[3];
        T[0] = X[4];  T[1] = X[5];  T[2] = X[6];  T[3] = X[7];
        R[0] = X[8];  R[1] = X[9];  R[2] = X[10]; R[3] = X[11];
        Q[0] = X[12]; Q[1] = X[13]; Q[2] = X[14]; Q[3] = X[15];
#else
        S[0] = X[0];  S[1] = X[4];  S[2] = X[8];  S[3] = X[12];
        T[0] = X[1];  T[1] = X[5];  T[2] = X[9];  T[3] = X[13];
        R[0] = X[2];  R[1] = X[6];  R[2] = X[10]; R[3] = X[14];
        Q[0] = X[3];  Q[1] = X[7];  Q[2] = X[11]; Q[3] = X[15];
#endif

        glActiveTextureARB(GL_TEXTURE3_ARB);
        set_active_texture_coordinates(S, T, R, Q);
        glActiveTextureARB(GL_TEXTURE2_ARB);
        set_active_texture_coordinates(S, T, R, Q);
        glActiveTextureARB(GL_TEXTURE1_ARB);
        set_active_texture_coordinates(S, T, R, Q);
        glActiveTextureARB(GL_TEXTURE0_ARB);
    }
}

/*---------------------------------------------------------------------------*/

int startup_display(void)
{
    tile = vecnew(32, sizeof (struct tile));
    host = vecnew(32, sizeof (struct host));

    if (tile && host)
        return 1;
    else
        return 0;
}
