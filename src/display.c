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

/*---------------------------------------------------------------------------*/

#define swap(a, b) { double t; t = (a); (a) = (b); (b) = t; }

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
    int l = 0;
    int r = 0;
    int b = 0;
    int t = 0;
    int i, n = vecnum(tile);

    /* Compute the total pixel size of all tiles. */

    if (n > 0)
    {
        struct tile *T = (struct tile *) vecget(tile, 0);

        l = T->pix_x;
        r = T->pix_x + T->pix_w;
        b = T->pix_y;
        t = T->pix_y + T->pix_h;
    }

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

        H->flags = 0;

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

static int add_tile(int i, int x, int y, int w, int h)
{
    int j = -1;

    if ((i < vecnum(host)) && (j = vecadd(tile)) >= 0)
    {
        struct host *H = (struct host *) vecget(host, i);
        struct tile *T = (struct tile *) vecget(tile, j);

        T->flags = 0;

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

#ifdef CONF_MPI
    int num  = vecnum(host);
    int siz  = vecsiz(host);
    int j;

    struct host *H;

    assert_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    assert_mpi(MPI_Bcast(&num, 1, MPI_INT, 0, MPI_COMM_WORLD));

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
        i = add_host(DEFAULT_NAME, DEFAULT_X, DEFAULT_Y, DEFAULT_W, DEFAULT_H);
            add_tile(i,            DEFAULT_X, DEFAULT_Y, DEFAULT_W, DEFAULT_H);

        local = (struct host *) vecget(host, i);

        local->flags = HOST_FRAMED;
    }

    if (rank || (local->flags & HOST_FRAMED) == 0)
        set_window_pos(local->win_x, local->win_y);
}

/*---------------------------------------------------------------------------*/

int send_add_tile(int i, int x, int y, int w, int h)
{
    send_event(EVENT_ADD_TILE);
    send_index(i);
    send_index(x);
    send_index(y);
    send_index(w);
    send_index(h);

    return add_tile(i, x, y, w, h);
}

void recv_add_tile(void)
{
    int i = recv_index();
    int x = recv_index();
    int y = recv_index();
    int w = recv_index();
    int h = recv_index();

    add_tile(i, x, y, w, h);
}

/*---------------------------------------------------------------------------*/

void send_set_host_flags(int i, int flags, int state)
{
    struct host *H = (struct host *) vecget(host, i);

    /* Host flags are only used at host creation time, so there's no point   */
    /* in sending them off to already-initialized hosts.                     */

    if (state)
        H->flags = H->flags | ( flags);
    else
        H->flags = H->flags & (~flags);
}

void send_set_tile_flags(int i, int flags, int state)
{
    struct tile *T = (struct tile *) vecget(tile, i);

    send_event(EVENT_SET_TILE_FLAGS);
    send_index(i);
    send_index(flags);
    send_index(state);

    if (state)
        T->flags = T->flags | ( flags);
    else
        T->flags = T->flags & (~flags);
}

void send_set_tile_position(int i, const float o[3],
                                   const float r[3],
                                   const float u[3])
{
    struct tile *T = (struct tile *) vecget(tile, i);

    send_event(EVENT_SET_TILE_POSITION);
    send_index(i);
    send_float((T->o[0] = o[0]));
    send_float((T->o[1] = o[1]));
    send_float((T->o[2] = o[2]));
    send_float((T->r[0] = r[0]));
    send_float((T->r[1] = r[1]));
    send_float((T->r[2] = r[2]));
    send_float((T->u[0] = u[0]));
    send_float((T->u[1] = u[1]));
    send_float((T->u[2] = u[2]));
}

void send_set_tile_viewport(int i, int x, int y, int w, int h)
{
    struct tile *T = (struct tile *) vecget(tile, i);

    send_event(EVENT_SET_TILE_VIEWPORT);
    send_index(i);
    send_index((T->pix_x = x));
    send_index((T->pix_y = y));
    send_index((T->pix_w = w));
    send_index((T->pix_h = h));

    bound_display();
}

void send_set_tile_line_screen(int i, float p, float a,
                                      float t, float s, float c)
{
    struct tile *T = (struct tile *) vecget(tile, i);

    send_event(EVENT_SET_TILE_LINE_SCREEN);
    send_index(i);
    send_float((T->varrier_pitch = p));
    send_float((T->varrier_angle = a));
    send_float((T->varrier_thick = t));
    send_float((T->varrier_shift = s));
    send_float((T->varrier_cycle = c));
}

void send_set_tile_view_mirror(int i, const float p[4])
{
    struct tile *T = (struct tile *) vecget(tile, i);

    send_event(EVENT_SET_TILE_VIEW_MIRROR);
    send_index(i);
    send_float((T->p[0] = p[0]));
    send_float((T->p[1] = p[1]));
    send_float((T->p[2] = p[2]));
    send_float((T->p[3] = p[3]));
}

void send_set_tile_view_offset(int i, const float d[3])
{
    struct tile *T = (struct tile *) vecget(tile, i);

    send_event(EVENT_SET_TILE_VIEW_OFFSET);
    send_index(i);
    send_float((T->d[0] = d[0]));
    send_float((T->d[1] = d[1]));
    send_float((T->d[2] = d[2]));
}

/*---------------------------------------------------------------------------*/

void recv_set_tile_flags(void)
{
    struct tile *T = (struct tile *) vecget(tile, recv_index());

    int flags = recv_index();
    int state = recv_index();

    if (state)
        T->flags = T->flags | ( flags);
    else
        T->flags = T->flags & (~flags);
}

void recv_set_tile_position(void)
{
    struct tile *T = (struct tile *) vecget(tile, recv_index());

    T->o[0] = recv_float();
    T->o[1] = recv_float();
    T->o[2] = recv_float();
    T->r[0] = recv_float();
    T->r[1] = recv_float();
    T->r[2] = recv_float();
    T->u[0] = recv_float();
    T->u[1] = recv_float();
    T->u[2] = recv_float();
}

void recv_set_tile_viewport(void)
{
    struct tile *T = (struct tile *) vecget(tile, recv_index());

    T->pix_x = recv_index();
    T->pix_y = recv_index();
    T->pix_w = recv_index();
    T->pix_h = recv_index();

    bound_display();
}

void recv_set_tile_line_screen(void)
{
    struct tile *T = (struct tile *) vecget(tile, recv_index());

    T->varrier_pitch = recv_float();
    T->varrier_angle = recv_float();
    T->varrier_thick = recv_float();
    T->varrier_shift = recv_float();
    T->varrier_cycle = recv_float();
}

void recv_set_tile_view_mirror(void)
{
    struct tile *T = (struct tile *) vecget(tile, recv_index());

    T->p[0] = recv_float();
    T->p[1] = recv_float();
    T->p[2] = recv_float();
    T->p[3] = recv_float();
}

void recv_set_tile_view_offset(void)
{
    struct tile *T = (struct tile *) vecget(tile, recv_index());

    T->d[0] = recv_float();
    T->d[1] = recv_float();
    T->d[2] = recv_float();
}

/*---------------------------------------------------------------------------*/

void send_set_background(const float c0[3], const float c1[3])
{
    send_event(EVENT_SET_BACKGROUND);

    send_float((color0[0] = c0[0]));
    send_float((color0[1] = c0[1]));
    send_float((color0[2] = c0[2]));

    send_float((color1[0] = c1[0]));
    send_float((color1[1] = c1[1]));
    send_float((color1[2] = c1[2]));
}

void recv_set_background(void)
{
    color0[0] = recv_float();
    color0[1] = recv_float();
    color0[2] = recv_float();

    color1[0] = recv_float();
    color1[1] = recv_float();
    color1[2] = recv_float();
}

/*---------------------------------------------------------------------------*/

void set_window_full(int f)
{
    if (local)
    {
        if (f)
            local->flags |= ( HOST_FULL);
        else
            local->flags &= (~HOST_FULL);
    }
}

void set_window_w(int w)
{
    int i;

    if (local)
    {
        local->win_w = w;

        for (i = 0; i < local->n; ++i)
        {
            ((struct tile *) vecget(tile, local->tile[i]))->win_w = w;
            ((struct tile *) vecget(tile, local->tile[i]))->pix_w = w;
        }
    }
}

void set_window_h(int h)
{
    int i;

    if (local)
    {
        local->win_h = h;

        for (i = 0; i < local->n; ++i)
        {
             ((struct tile *) vecget(tile, local->tile[i]))->win_h = h;
             ((struct tile *) vecget(tile, local->tile[i]))->pix_h = h;
        }
   }
}

int get_window_w(void) { return local ? local->win_w : DEFAULT_W; }
int get_window_h(void) { return local ? local->win_h : DEFAULT_H; }

int get_window_full(void)
{
    return local ? (local->flags & HOST_FULL) : 0;
}

int get_window_stereo(void)
{
    return local ? (local->flags & HOST_STEREO) : 0;
}

int get_window_framed(void)
{
    return local ? (local->flags & HOST_FRAMED) : 1;
}

/*---------------------------------------------------------------------------*/

void get_display_point(float v[3], const float p[3], int x, int y)
{
    static float last_v[3] = { 0.0f, 0.0f, 0.0f };

    int i, n = vecnum(tile);

    /* Search all tiles for one encompassing the given point (x, y). */

    for (i = 0; i < n; ++i)
    {
        struct tile *T = (struct tile *) vecget(tile, i);

        if (T->pix_x <= x && x < T->pix_x + T->pix_w &&
            T->pix_y <= y && y < T->pix_y + T->pix_h)
        {
            /* Compute the world-space vector of this point. */

            const float kx =        (float) (x - T->pix_x) / T->pix_w;
            const float ky = 1.0f - (float) (y - T->pix_y) / T->pix_h;

            v[0] = (T->o[0] + T->r[0] * kx + T->u[0] * ky) - p[0];
            v[1] = (T->o[1] + T->r[1] * kx + T->u[1] * ky) - p[1];
            v[2] = (T->o[2] + T->r[2] * kx + T->u[2] * ky) - p[2];

            normalize(v);

            /* Cache this value. */

            last_v[0] = v[0];
            last_v[1] = v[1];
            last_v[2] = v[2];
                
            return;
        }
    }

    /* The point does not fall within a tile.  Return the previous value. */

    v[0] = last_v[0];
    v[1] = last_v[1];
    v[2] = last_v[2];
}

void get_display_union(float b[4])
{
    if (local)
    {
        b[0] = (float) local->tot_x;
        b[1] = (float) local->tot_y;
        b[2] = (float) local->tot_w;
        b[3] = (float) local->tot_h;
    }
    else
    {
        b[0] = (float) display_x;
        b[1] = (float) display_y;
        b[2] = (float) display_w;
        b[3] = (float) display_h;
    }
}

void get_display_bound(float b[6])
{
    int i, n = vecnum(tile);

    if (n > 0)
    {
        struct tile *T = (struct tile *) vecget(tile, 0);

        /* Compute the rectangular union of all tiles. */

        b[0] = b[3] = T->o[0];
        b[1] = b[4] = T->o[1];
        b[2] = b[5] = T->o[2];

        for (i = 0; i < n; ++i)
        {
            T = (struct tile *) vecget(tile, i);

            /* Lower left corner. */

            b[0] = MIN(b[0], T->o[0]);
            b[1] = MIN(b[1], T->o[1]);
            b[2] = MIN(b[2], T->o[2]);
            b[3] = MAX(b[3], T->o[0]);
            b[4] = MAX(b[4], T->o[1]);
            b[5] = MAX(b[5], T->o[2]);

            /* Lower right corner. */

            b[0] = MIN(b[0], T->o[0] + T->r[0]);
            b[1] = MIN(b[1], T->o[1] + T->r[1]);
            b[2] = MIN(b[2], T->o[2] + T->r[2]);
            b[3] = MAX(b[3], T->o[0] + T->r[0]);
            b[4] = MAX(b[4], T->o[1] + T->r[1]);
            b[5] = MAX(b[5], T->o[2] + T->r[2]);

            /* Upper left corner. */

            b[0] = MIN(b[0], T->o[0]           + T->u[0]);
            b[1] = MIN(b[1], T->o[1]           + T->u[1]);
            b[2] = MIN(b[2], T->o[2]           + T->u[2]);
            b[3] = MAX(b[3], T->o[0]           + T->u[0]);
            b[4] = MAX(b[4], T->o[1]           + T->u[1]);
            b[5] = MAX(b[5], T->o[2]           + T->u[2]);

            /* Upper right corner. */

            b[0] = MIN(b[0], T->o[0] + T->r[0] + T->u[0]);
            b[1] = MIN(b[1], T->o[1] + T->r[1] + T->u[1]);
            b[2] = MIN(b[2], T->o[2] + T->r[2] + T->u[2]);
            b[3] = MAX(b[3], T->o[0] + T->r[0] + T->u[0]);
            b[4] = MAX(b[4], T->o[1] + T->r[1] + T->u[1]);
            b[5] = MAX(b[5], T->o[2] + T->r[2] + T->u[2]);
        }
    }
    else
    {
        /* No tiles are defined.  Return the defaults. */

        b[0] = DEFAULT_OX;
        b[1] = DEFAULT_OY;
        b[2] = DEFAULT_OZ;
        b[3] = DEFAULT_OX + DEFAULT_RX + DEFAULT_UX;
        b[4] = DEFAULT_OY + DEFAULT_RY + DEFAULT_UY;
        b[5] = DEFAULT_OZ + DEFAULT_RZ + DEFAULT_UZ;
   }
}

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

int get_tile_count(void)
{
	return local ? local->n : 0;
}

int get_tile_flags(int i)
{
    return local ? ((struct tile *) vecget(tile, local->tile[i]))->flags : 0;
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
    struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

    GLdouble fL = T->pix_x;
    GLdouble fR = T->pix_x + T->pix_w;
    GLdouble fB = T->pix_y;
    GLdouble fT = T->pix_y + T->pix_h;

    /* Flip the projection if requested. */

    if (T->flags & TILE_FLIP_X) swap(fL, fR);
    if (T->flags & TILE_FLIP_Y) swap(fB, fT);

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

    if (((T->flags & TILE_FLIP_X) ? 1 : 0) ^
        ((T->flags & TILE_FLIP_Y) ? 1 : 0))
        glFrontFace(GL_CW);
    else
        glFrontFace(GL_CCW);

    return 1;
}

int draw_persp(int i, float N, float F, int e, const float p[3])
{
    struct tile *T = (struct tile *) vecget(tile, local->tile[i]);

    const int L = (T->flags & TILE_LEFT_EYE)  ? 1 : 0;
    const int R = (T->flags & TILE_RIGHT_EYE) ? 1 : 0;

    if ((L == 0 && R == 0) || (L == 1 && e == 0) || (R == 1 && e == 1))
    {
        float P[3];
        float r[3];
        float u[3];
        float n[3];
        float k;

        float M[16];
        float I[16];

        float p0[3];
        float p1[3];
        float p3[3];

        /* Compute the view position. */

        P[0] = T->d[0] + p[0];
        P[1] = T->d[1] + p[1];
        P[2] = T->d[2] + p[2];

        /* Optionally reflect the view position across the mirror. */

        if (T->flags & TILE_MIRROR)
        {
            k = (P[0] * T->p[0] +
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

            if (T->flags & TILE_FLIP_X) swap(fL, fR);
            if (T->flags & TILE_FLIP_Y) swap(fB, fT);

            /* Apply the projection. */

            glLoadIdentity();
            glFrustum(fL, fR, fB, fT, N, F);

            /* Account for the orientation of the display. */

            M[0] = r[0]; M[4] = u[0]; M[8]  = n[0]; M[12] = 0.0f;
            M[1] = r[1]; M[5] = u[1]; M[9]  = n[1]; M[13] = 0.0f;
            M[2] = r[2]; M[6] = u[2]; M[10] = n[2]; M[14] = 0.0f;
            M[3] = 0.0f; M[7] = 0.0f; M[11] = 0.0f; M[15] = 1.0f;

            load_inv(I, M);
            glMultMatrixf(I);
        }
        glMatrixMode(GL_MODELVIEW);

        /* Apply the tile offset. */

        glLoadIdentity();
        glTranslatef(-T->d[0], -T->d[1], -T->d[2]);

        /* Rewind polygons if necessary. */

        if (((T->flags & TILE_FLIP_X) ? 1 : 0) ^
            ((T->flags & TILE_FLIP_Y) ? 1 : 0))
            glFrontFace(GL_CW);
        else
            glFrontFace(GL_CCW);

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void draw_tile_background(int i, int flags)
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
        float l = (T->flags & TILE_FLIP_X) ? 1.0f : 0.0f;
        float r = (T->flags & TILE_FLIP_X) ? 0.0f : 1.0f;
        float b = (T->flags & TILE_FLIP_Y) ? 1.0f : 0.0f;
        float t = (T->flags & TILE_FLIP_Y) ? 0.0f : 1.0f;

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);

        if (flags & DRAW_VARRIER_TEXGEN)
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
        draw_tile_background(i, 0);
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

        if (GL_TEXTURE3_ARB < GL_max_multitexture)
        {
            glActiveTextureARB(GL_TEXTURE3_ARB);
            set_active_texture_coordinates(S, T, R, Q);
        }
        if (GL_TEXTURE2_ARB < GL_max_multitexture)
        {
            glActiveTextureARB(GL_TEXTURE2_ARB);
            set_active_texture_coordinates(S, T, R, Q);
        }
        if (GL_TEXTURE1_ARB < GL_max_multitexture)
        {
            glActiveTextureARB(GL_TEXTURE1_ARB);
            set_active_texture_coordinates(S, T, R, Q);
        }
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
