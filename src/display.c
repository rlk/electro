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
#include "utility.h"
#include "tracker.h"
#include "display.h"
#include "matrix.h"
#include "buffer.h"
#include "event.h"
#include "video.h"
#include "vec.h"

/*===========================================================================*/

static struct tile *tile  = NULL;
static struct host *host  = NULL;

static struct tile  default_tile;
static struct host  current_host;

static int display_x = DEFAULT_X;
static int display_y = DEFAULT_Y;
static int display_w = DEFAULT_W;
static int display_h = DEFAULT_H;

static float color0[3] = { 0.1f, 0.2f, 0.4f };
static float color1[3] = { 0.0f, 0.0f, 0.0f };

/*---------------------------------------------------------------------------*/

static unsigned int new_tile(void)
{
    unsigned int i;
    void        *v;

    if ((i = vec_add(tile, sizeof (struct tile))))
    {
        memset(tile +i, 0, sizeof (struct tile));
        return i;
    }

    if ((v = vec_gro(tile, sizeof (struct tile))))
    {
        tile = (struct tile *) v;
        return new_tile();
    }
    return 0;
}

static unsigned int new_host(void)
{
    unsigned int i;
    void        *v;

    if ((i = vec_add(host, sizeof (struct host))))
    {
        memset(host +i, 0, sizeof (struct host));
        return i;
    }

    if ((v = vec_gro(host, sizeof (struct host))))
    {
        host = (struct host *) v;
        return new_host();
    }
    return 0;
}

#define chk_tile(i) vec_chk(tile, sizeof (struct tile), i)
#define chk_host(i) vec_chk(host, sizeof (struct host), i)

#define ALL_TILES(i, ii) \
    i = ii = 0; vec_all(tile, sizeof (struct tile), &i, &ii);
#define ALL_HOSTS(i, ii) \
    i = ii = 0; vec_all(host, sizeof (struct host), &i, &ii);
 
/*---------------------------------------------------------------------------*/

static struct tile *get_tile(unsigned int i)
{
    if (i < current_host.n)
        return tile + current_host.tile[i];
    else
        return &default_tile;
}

/*===========================================================================*/

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

    unsigned int i, ii;

    /* Compute the total pixel size of all tiles. */

    for (ALL_TILES(i, ii))
    {
        l = MIN(l, tile[i].pix_x);
        r = MAX(r, tile[i].pix_x + tile[i].pix_w);
        b = MIN(b, tile[i].pix_y);
        t = MAX(t, tile[i].pix_y + tile[i].pix_h);
    }

    display_x =     l;
    display_y =     b;
    display_w = r - l;
    display_h = t - b;

    /* Copy the total pixel size to all host configurations. */

    for (ALL_HOSTS(i, ii))
    {
        host[i].tot_x = display_x;
        host[i].tot_y = display_y;
        host[i].tot_w = display_w;
        host[i].tot_h = display_h;
    }
}

/*---------------------------------------------------------------------------*/

unsigned int add_host(const char *name, int x, int y, int w, int h)
{
    unsigned int i;

    if ((i = new_host()))
    {
        host[i].flags = 0;
        host[i].n     = 0;

        /* Store the name for future host name searching. */

        strncpy(host[i].name, name, MAXNAME);

        /* The rectangle defines window size and default total display size. */

        host[i].tot_x = host[i].win_x = x;
        host[i].tot_y = host[i].win_y = y;
        host[i].tot_w = host[i].win_w = w;
        host[i].tot_h = host[i].win_h = h;
    }
    return i;
}

static unsigned int add_tile(unsigned int i, int x, int y, int w, int h)
{
    int j = 0;
    
    if ((chk_host(i)) && (j = new_tile()))
    {
        /* Set a default display configuration. */

        tile[j] = default_tile;

        /* The rectangle defines viewport size and default ortho projection. */

        tile[j].pix_x = tile[j].win_x = x;
        tile[j].pix_y = tile[j].win_y = y;
        tile[j].pix_w = tile[j].win_w = w;
        tile[j].pix_h = tile[j].win_h = h;

        /* Include this tile in the host and in the total display. */

        host[i].tile[host[i].n++] = j;

        bound_display();
    }
    return j;
}

/*---------------------------------------------------------------------------*/

int find_display(const char *name)
{
    unsigned int i, ii;

    /* Find a host definition for the given name.  Mark it as used. */

    for (ALL_HOSTS(i, ii))
        if (strncmp(host[i].name, name, MAXNAME) == 0)
        {
            host[i].name[0] = '\0';
            return i;
        }

    /* Find a default host definition specification.  Mark it as used. */

    for (ALL_HOSTS(i, ii))
        if (strncmp(host[i].name, DEFAULT_NAME, MAXNAME) == 0)
        {
            host[i].name[0] = '\0';
            return i;
        }

    /* Return 0 upon failure. */

    return 0;
}

void sync_display(void)
{
#ifdef CONF_MPI
    MPI_Status stat;
    int        size = 0;
#endif
    int        rank = 0;

    char name[MAXNAME];
    int  i = 0;

    gethostname(name, MAXNAME);

#ifdef CONF_MPI

    assert_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    assert_mpi(MPI_Comm_size(MPI_COMM_WORLD, &size));

    if (rank)
    {
        /* Send the name to the root, recieve the host index. */

        MPI_Send(name, MAXNAME, MPI_BYTE, 0,0, MPI_COMM_WORLD);
        MPI_Recv(&i,   4,       MPI_BYTE, 0,0, MPI_COMM_WORLD, &stat);
    }
    else
    {
        int j, k;

        /* Find a host definition for the root. */

        i = find_display(name);

        /* Recieve a name from each client, send a host definition index. */

        for (j = 1; j < size; ++j)
        {
            MPI_Recv(name, MAXNAME, MPI_BYTE, j,0, MPI_COMM_WORLD, &stat);
            k = find_display(name);
            MPI_Send(&k,   4,       MPI_BYTE, j,0, MPI_COMM_WORLD);
        }
    }
#else

    i = find_display(name);

#endif

    /* If no host definition was found, create a default. */

    if (i == 0)
    {
        i = add_host(DEFAULT_NAME, DEFAULT_X, DEFAULT_Y, DEFAULT_W, DEFAULT_H);
            add_tile(i,            DEFAULT_X, DEFAULT_Y, DEFAULT_W, DEFAULT_H);

        host[i].flags = HOST_FRAMED;
    }

    /* Note the indexed host definition as current. */

    current_host = host[i];

    /* Position the server window, if necessary. */

    if (rank || (current_host.flags & HOST_FRAMED) == 0)
        set_window_pos(current_host.win_x, current_host.win_y);
}

/*===========================================================================*/

unsigned int send_add_host(const char *name, int x, int y, int w, int h)
{
    int n = strlen(name) + 1;

    send_event(EVENT_ADD_HOST);
    send_value(n);
    send_value(x);
    send_value(y);
    send_value(w);
    send_value(h);
    send_array(name, n, 1);

    return add_host(name, x, y, w, h);
}

void recv_add_host(void)
{
    char name[MAXNAME];

    int n = recv_value();
    int x = recv_value();
    int y = recv_value();
    int w = recv_value();
    int h = recv_value();

    recv_array(name, n, 1);

    add_host(name, x, y, w, h);
}

/*---------------------------------------------------------------------------*/

unsigned int send_add_tile(unsigned int i, int x, int y, int w, int h)
{
    send_event(EVENT_ADD_TILE);
    send_index(i);
    send_value(x);
    send_value(y);
    send_value(w);
    send_value(h);

    return add_tile(i, x, y, w, h);
}

void recv_add_tile(void)
{
    unsigned int i = recv_index();

    int x = recv_value();
    int y = recv_value();
    int w = recv_value();
    int h = recv_value();

    add_tile(i, x, y, w, h);
}

/*---------------------------------------------------------------------------*/

void send_set_host_flags(unsigned int i, unsigned int flags,
                                         unsigned int state)
{
    send_event(EVENT_SET_HOST_FLAGS);
    send_index(i);
    send_index(flags);
    send_index(state);

    if (state)
        host[i].flags = host[i].flags | ( flags);
    else
        host[i].flags = host[i].flags & (~flags);
}

void send_set_tile_flags(unsigned int i, unsigned int flags,
                                         unsigned int state)
{
    send_event(EVENT_SET_TILE_FLAGS);
    send_index(i);
    send_index(flags);
    send_index(state);

    if (state)
        tile[i].flags = tile[i].flags | ( flags);
    else
        tile[i].flags = tile[i].flags & (~flags);
}

void send_set_tile_quality(unsigned int i, const float q[2])
{
    send_event(EVENT_SET_TILE_QUALITY);
    send_index(i);
    send_float((tile[i].quality[0] = q[0]));
    send_float((tile[i].quality[1] = q[1]));
}

void send_set_tile_viewport(unsigned int i, int x, int y, int w, int h)
{
    send_event(EVENT_SET_TILE_VIEWPORT);
    send_index(i);
    send_index((tile[i].pix_x = x));
    send_index((tile[i].pix_y = y));
    send_index((tile[i].pix_w = w));
    send_index((tile[i].pix_h = h));

    bound_display();
}

void send_set_tile_position(unsigned int i, const float o[3],
                                            const float r[3],
                                            const float u[3])
{
    send_event(EVENT_SET_TILE_POSITION);
    send_index(i);
    send_float((tile[i].o[0] = o[0]));
    send_float((tile[i].o[1] = o[1]));
    send_float((tile[i].o[2] = o[2]));
    send_float((tile[i].r[0] = r[0]));
    send_float((tile[i].r[1] = r[1]));
    send_float((tile[i].r[2] = r[2]));
    send_float((tile[i].u[0] = u[0]));
    send_float((tile[i].u[1] = u[1]));
    send_float((tile[i].u[2] = u[2]));
}

void send_set_tile_linescrn(unsigned int i, float p, float a,
                                            float t, float s, float c)
{
    send_event(EVENT_SET_TILE_LINESCRN);
    send_index(i);
    send_float((tile[i].varrier_pitch = p));
    send_float((tile[i].varrier_angle = a));
    send_float((tile[i].varrier_thick = t));
    send_float((tile[i].varrier_shift = s));
    send_float((tile[i].varrier_cycle = c));
}

/*---------------------------------------------------------------------------*/

void recv_set_host_flags(void)
{
    unsigned int i     = recv_index();
    unsigned int flags = recv_index();
    unsigned int state = recv_index();

    if (state)
        host[i].flags = host[i].flags | ( flags);
    else
        host[i].flags = host[i].flags & (~flags);
}

void recv_set_tile_flags(void)
{
    unsigned int i     = recv_index();
    unsigned int flags = recv_index();
    unsigned int state = recv_index();

    if (state)
        tile[i].flags = tile[i].flags | ( flags);
    else
        tile[i].flags = tile[i].flags & (~flags);
}

void recv_set_tile_quality(void)
{
    unsigned int i = recv_index();

    tile[i].quality[0] = recv_float();
    tile[i].quality[1] = recv_float();
}

void recv_set_tile_viewport(void)
{
    unsigned int i = recv_index();

    tile[i].pix_x = recv_index();
    tile[i].pix_y = recv_index();
    tile[i].pix_w = recv_index();
    tile[i].pix_h = recv_index();

    bound_display();
}

void recv_set_tile_position(void)
{
    unsigned int i = recv_index();

    tile[i].o[0] = recv_float();
    tile[i].o[1] = recv_float();
    tile[i].o[2] = recv_float();
    tile[i].r[0] = recv_float();
    tile[i].r[1] = recv_float();
    tile[i].r[2] = recv_float();
    tile[i].u[0] = recv_float();
    tile[i].u[1] = recv_float();
    tile[i].u[2] = recv_float();
}

void recv_set_tile_linescrn(void)
{
    unsigned int i = recv_index();

    tile[i].varrier_pitch = recv_float();
    tile[i].varrier_angle = recv_float();
    tile[i].varrier_thick = recv_float();
    tile[i].varrier_shift = recv_float();
    tile[i].varrier_cycle = recv_float();
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
    if (f)
        current_host.flags |= ( HOST_FULL);
    else
        current_host.flags &= (~HOST_FULL);
}

void set_window_w(int w)
{
    unsigned int i;

    current_host.win_w = w;

    for (i = 0; i < current_host.n; ++i)
    {
        tile[current_host.tile[i]].win_w = w;
        tile[current_host.tile[i]].pix_w = w;
    }
}

void set_window_h(int h)
{
    unsigned int i;

    current_host.win_h = h;

    for (i = 0; i < current_host.n; ++i)
    {
        tile[current_host.tile[i]].win_h = h;
        tile[current_host.tile[i]].pix_h = h;
    }
}

int get_window_w(void) { return current_host.win_w; }
int get_window_h(void) { return current_host.win_h; }

int get_window_full(void)
{
    return ((current_host.flags & HOST_FULL)   ? 1 : 0);
}

int get_window_stereo(void)
{
    return ((current_host.flags & HOST_STEREO) ? 1 : 0);
}

int get_window_framed(void)
{
    return ((current_host.flags & HOST_FRAMED) ? 1 : 0);
}

/*---------------------------------------------------------------------------*/

void get_display_point(float v[3], const float p[3], int x, int y)
{
    static float last_v[3] = { 0.0f, 0.0f, 0.0f };

    unsigned int i, ii;

    /* Search all tiles for one encompassing the given point (x, y). */

    for (ALL_TILES(i, ii))
        if (tile[i].pix_x <= x && x < tile[i].pix_x + tile[i].pix_w &&
            tile[i].pix_y <= y && y < tile[i].pix_y + tile[i].pix_h)
        {
            /* Compute the world-space vector of this point. */

            float kx =        (float) (x - tile[i].pix_x) / tile[i].pix_w;
            float ky = 1.0f - (float) (y - tile[i].pix_y) / tile[i].pix_h;

            v[0] = (tile[i].o[0] + tile[i].r[0]*kx + tile[i].u[0]*ky) - p[0];
            v[1] = (tile[i].o[1] + tile[i].r[1]*kx + tile[i].u[1]*ky) - p[1];
            v[2] = (tile[i].o[2] + tile[i].r[2]*kx + tile[i].u[2]*ky) - p[2];

            normalize(v);

            /* Cache this value. */

            last_v[0] = v[0];
            last_v[1] = v[1];
            last_v[2] = v[2];
                
            return;
        }

    /* The point does not fall within a tile.  Return the previous value. */

    v[0] = last_v[0];
    v[1] = last_v[1];
    v[2] = last_v[2];
}

void get_display_union(float b[4])
{
/*
    b[0] = (float) current_host.tot_x;
    b[1] = (float) current_host.tot_y;
    b[2] = (float) current_host.tot_w;
    b[3] = (float) current_host.tot_h;
*/
    b[0] = (float) display_x;
    b[1] = (float) display_y;
    b[2] = (float) display_w;
    b[3] = (float) display_h;

}

void get_display_bound(float b[6])
{
    unsigned int i, ii, c = 0;

    b[0] =  1e10;
    b[1] =  1e10;
    b[2] =  1e10;
    b[3] = -1e10;
    b[4] = -1e10;
    b[5] = -1e10;

    /* Compute the rectangular union of all tiles. */

    for (ALL_TILES(i, ii))
    {
        /* Lower left corner. */

        b[0] = MIN(b[0], tile[i].o[0]);
        b[1] = MIN(b[1], tile[i].o[1]);
        b[2] = MIN(b[2], tile[i].o[2]);
        b[3] = MAX(b[3], tile[i].o[0]);
        b[4] = MAX(b[4], tile[i].o[1]);
        b[5] = MAX(b[5], tile[i].o[2]);

        /* Lower right corner. */

        b[0] = MIN(b[0], tile[i].o[0] + tile[i].r[0]);
        b[1] = MIN(b[1], tile[i].o[1] + tile[i].r[1]);
        b[2] = MIN(b[2], tile[i].o[2] + tile[i].r[2]);
        b[3] = MAX(b[3], tile[i].o[0] + tile[i].r[0]);
        b[4] = MAX(b[4], tile[i].o[1] + tile[i].r[1]);
        b[5] = MAX(b[5], tile[i].o[2] + tile[i].r[2]);

        /* Upper left corner. */

        b[0] = MIN(b[0], tile[i].o[0] + tile[i].u[0]);
        b[1] = MIN(b[1], tile[i].o[1] + tile[i].u[1]);
        b[2] = MIN(b[2], tile[i].o[2] + tile[i].u[2]);
        b[3] = MAX(b[3], tile[i].o[0] + tile[i].u[0]);
        b[4] = MAX(b[4], tile[i].o[1] + tile[i].u[1]);
        b[5] = MAX(b[5], tile[i].o[2] + tile[i].u[2]);

        /* Upper right corner. */

        b[0] = MIN(b[0], tile[i].o[0] + tile[i].r[0] + tile[i].u[0]);
        b[1] = MIN(b[1], tile[i].o[1] + tile[i].r[1] + tile[i].u[1]);
        b[2] = MIN(b[2], tile[i].o[2] + tile[i].r[2] + tile[i].u[2]);
        b[3] = MAX(b[3], tile[i].o[0] + tile[i].r[0] + tile[i].u[0]);
        b[4] = MAX(b[4], tile[i].o[1] + tile[i].r[1] + tile[i].u[1]);
        b[5] = MAX(b[5], tile[i].o[2] + tile[i].r[2] + tile[i].u[2]);

        c++;
    }

    if (c == 0)
    {
        /* No tiles are defined.  Return the defaults. */

        b[0] = default_tile.o[0];
        b[1] = default_tile.o[1];
        b[2] = default_tile.o[2];
        b[3] = default_tile.o[0] + default_tile.r[0] + default_tile.u[0];
        b[4] = default_tile.o[1] + default_tile.r[1] + default_tile.u[1];
        b[5] = default_tile.o[2] + default_tile.r[2] + default_tile.u[2];
    }
}

/*---------------------------------------------------------------------------*/

void get_tile_o(unsigned int i, float o[3])
{
    struct tile *T = get_tile(i);

    o[0] = T->o[0];
    o[1] = T->o[1];
    o[2] = T->o[2];
}

void get_tile_r(unsigned int i, float r[3])
{
    struct tile *T = get_tile(i);

    r[0] = T->r[0];
    r[1] = T->r[1];
    r[2] = T->r[2];
}

void get_tile_u(unsigned int i, float u[3])
{
    struct tile *T = get_tile(i);

    u[0] = T->u[0];
    u[1] = T->u[1];
    u[2] = T->u[2];
}

void get_tile_n(unsigned int i, float n[3])
{
    float r[3];
    float u[3];

    get_tile_r(i, r);
    get_tile_u(i, u);

    cross(n, r, u);
    normalize(n);
}

void get_tile_quality(unsigned int i, float q[2])
{
    struct tile *T = get_tile(i);

    q[0] = T->quality[0];
    q[1] = T->quality[1];
}

float get_varrier_pitch(unsigned int i)
{
    return get_tile(i)->varrier_pitch;
}

float get_varrier_angle(unsigned int i)
{
    return get_tile(i)->varrier_angle;
}

float get_varrier_thick(unsigned int i)
{
    return get_tile(i)->varrier_thick;
}

float get_varrier_shift(unsigned int i)
{
    return get_tile(i)->varrier_shift;
}

float get_varrier_cycle(unsigned int i)
{
    return get_tile(i)->varrier_cycle;
}

unsigned int get_tile_count(void)
{
    return current_host.n;
}

unsigned int get_tile_flags(unsigned int i)
{
    return get_tile(i)->flags;
}

/*---------------------------------------------------------------------------*/

int draw_ortho(unsigned int i, float N, float F)
{
    struct tile *T = get_tile(i);
    
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
        glOrtho(fL, fR, fB, fT, N, F);
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

int draw_persp(unsigned int i, float N, float F, int e, const float p[3])
{
    struct tile *T = get_tile(i);

    const int L = (T->flags & TILE_LEFT_EYE)  ? 1 : 0;
    const int R = (T->flags & TILE_RIGHT_EYE) ? 1 : 0;

    if ((L == 0 && R == 0) || (L == 1 && e == 0) || (R == 1 && e == 1))
    {
        float r[3];
        float u[3];
        float n[3];
        float k;

        float M[16];
        float I[16];

        float p0[3];
        float p1[3];
        float p3[3];

        /* Compute the screen corners. */

        p0[0] = p[0] - T->o[0];
        p0[1] = p[1] - T->o[1];
        p0[2] = p[2] - T->o[2];

        p1[0] = p[0] - T->r[0] - T->o[0];
        p1[1] = p[1] - T->r[1] - T->o[1];
        p1[2] = p[2] - T->r[2] - T->o[2];

        p3[0] = p[0] - T->u[0] - T->o[0];
        p3[1] = p[1] - T->u[1] - T->o[1];
        p3[2] = p[2] - T->u[2] - T->o[2];

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

        k = n[0] * (T->o[0] - p[0]) + 
            n[1] * (T->o[1] - p[1]) +
            n[2] * (T->o[2] - p[2]);

        glMatrixMode(GL_PROJECTION);
        {
            double fL = N * (r[0] * p0[0] + r[1] * p0[1] + r[2] * p0[2]) / k;
            double fR = N * (r[0] * p1[0] + r[1] * p1[1] + r[2] * p1[2]) / k;
            double fB = N * (u[0] * p0[0] + u[1] * p0[1] + u[2] * p0[2]) / k;
            double fT = N * (u[0] * p3[0] + u[1] * p3[1] + u[2] * p3[2]) / k;

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

            /* Move the apex of the frustum to the origin. */

            glTranslatef(-p[0], -p[1], -p[2]);
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
    return 0;
}

/*---------------------------------------------------------------------------*/

void draw_tile_background(unsigned int i)
{
    struct tile *T = get_tile(i);

    /* Compute the beginning and end of this tile's gradiant. */

    float k0 = (float) (T->pix_y            - current_host.tot_y) /
                                              current_host.tot_h;
    float k1 = (float) (T->pix_y + T->pix_h - current_host.tot_y) /
                                              current_host.tot_h;

    /* Confine rendering to this tile. */

    glPushAttrib(GL_ENABLE_BIT);
    {
        float l = (T->flags & TILE_FLIP_X) ? 1.0f : 0.0f;
        float r = (T->flags & TILE_FLIP_X) ? 0.0f : 1.0f;
        float b = (T->flags & TILE_FLIP_Y) ? 1.0f : 0.0f;
        float t = (T->flags & TILE_FLIP_Y) ? 0.0f : 1.0f;

        /* Map the tile onto the unit cube. */

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 1, 0, 1, 0, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        /* Fill the tile at the far plane using the computed gradient. */

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);

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

        /* Revert to the previous transformation. */

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    glPopAttrib();
}

void draw_host_background(void)
{
    unsigned int i;

    glViewport(current_host.win_x, current_host.win_y,
               current_host.win_w, current_host.win_h);
    glScissor (current_host.win_x, current_host.win_y,
               current_host.win_w, current_host.win_h);

    glClear(GL_DEPTH_BUFFER_BIT |
            GL_COLOR_BUFFER_BIT);

    for (i = 0; i < current_host.n; ++i)
        draw_tile_background(i);
}

/*---------------------------------------------------------------------------*/

int startup_display(void)
{
    /* Initialize the host and tile config structures. */

    tile = vec_new(4, sizeof (struct tile));
    host = vec_new(4, sizeof (struct host));

    /* Initialize the default tile config. */

    default_tile.flags = 0;

    default_tile.o[0]  = DEFAULT_OX;
    default_tile.o[1]  = DEFAULT_OY;
    default_tile.o[2]  = DEFAULT_OZ;

    default_tile.r[0]  = DEFAULT_RX;
    default_tile.r[1]  = DEFAULT_RY;
    default_tile.r[2]  = DEFAULT_RZ;

    default_tile.u[0]  = DEFAULT_UX;
    default_tile.u[1]  = DEFAULT_UY;
    default_tile.u[2]  = DEFAULT_UZ;

    default_tile.win_x = default_tile.pix_x = DEFAULT_X;
    default_tile.win_y = default_tile.pix_y = DEFAULT_Y;
    default_tile.win_w = default_tile.pix_w = DEFAULT_W;
    default_tile.win_h = default_tile.pix_h = DEFAULT_H;

    default_tile.varrier_pitch = DEFAULT_VARRIER_PITCH;
    default_tile.varrier_angle = DEFAULT_VARRIER_ANGLE;
    default_tile.varrier_thick = DEFAULT_VARRIER_THICK;
    default_tile.varrier_shift = DEFAULT_VARRIER_SHIFT;
    default_tile.varrier_cycle = DEFAULT_VARRIER_CYCLE;

    default_tile.quality[0] = 1.0f;
    default_tile.quality[1] = 1.0f;

    /* Initialize the default host config. */

    current_host.flags = HOST_FRAMED;
    current_host.n     = 0;

    current_host.win_x = current_host.tot_x = DEFAULT_X;
    current_host.win_y = current_host.tot_y = DEFAULT_Y;
    current_host.win_w = current_host.tot_w = DEFAULT_W;
    current_host.win_h = current_host.tot_h = DEFAULT_H;

    if (tile && host)
        return 1;
    else
        return 0;
}
