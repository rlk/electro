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

#include "display.h"

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

    if (gethostname(H.name, MAXNAME) == 0)
    {
        size_t sz = sizeof (struct host);
        int i, j, size;

        assert_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
        assert_mpi(MPI_Comm_size(MPI_COMM_WORLD, &m));

        /* Gather all host names at the root. */

        assert_mpi(MPI_Gather(&Host, sz, MPI_BYTE,
                               Ho,   sz, MPI_BYTE, 0, MPI_COMM_WORLD));

        /* The root assigns tiles by matching host names. */

        if (rank == 0)
            for (i = 0; i < m; i++)
                for (j = 0; j < H_num; j++)
                    if (strcmp(Hi[i].name, Ho[j].name) == 0)
                    {
                        memcpy(Ho + j, Hi + i, sizeof (struct host));
                        strcpy(Hi[i], name, "");

                        break;
                    }

        /* Scatter the assignments to all clients. */

        assert_mpi(MPI_Scatter(Ho,   sz, MPI_BYTE,
                              &Host, sz, MPI_BYTE, 0, MPI_COMM_WORLD));
    }
#endif

    if (rank == 0) memcpy(Host, Hi[0], sizeof (struct host));
}

/*---------------------------------------------------------------------------*/

static int get_host(const char *name)
{
    int hd;

    for (hd = 0; hd < H_max; hd++)
        if (strncmp(H[hd].name, name, NAMELEN) == 0)
            return hd;

    return -1;
}

void add_host(const char *name, int X, int Y, int W, int H)
{
    if (H_num < H_max)
    {
        strncpy(S[hd].name, name, NAMELEN);

        Hi[H_num].X = X;
        Hi[H_num].Y = Y;
        Hi[H_num].W = W;
        Hi[H_num].H = H;
        Hi[H_num].n = 0;

        H_num++;
    }
}

void add_tile(const char *name, int x, int y, int w, int h, const float p[3],
                                                            const float r[3],
                                                            const float u[3])
{
    int i = get_host(name);

    if (0 <= i && i < H_num)
    {
        int n = H[hd].n;

        Hi[i].tile[n].x    = x;
        Hi[i].tile[n].y    = y;
        Hi[i].tile[n].w    = w;
        Hi[i].tile[n].h    = h;

        Hi[i].tile[n].p[0] = p[0];
        Hi[i].tile[n].p[1] = p[1];
        Hi[i].tile[n].p[2] = p[2];

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
