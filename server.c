/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    TOTAL PERSPECTIVE VORTEX is free software;  you can redistribute it    */
/*    and/or modify it under the terms of the  GNU General Public License    */
/*    as published by the  Free Software Foundation;  either version 2 of    */
/*    the License, or (at your option) any later version.                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#include <mpi.h>
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "opengl.h"
#include "shared.h"
#include "status.h"
#include "galaxy.h"
#include "script.h"
#include "server.h"
#include "star.h"

static void server_draw(void);

/*---------------------------------------------------------------------------*/

static void server_send_event(struct event *e)
{
    size_t sz = sizeof (struct event);
    int  err;

    if ((err = MPI_Bcast(e, sz, MPI_BYTE, 0, MPI_COMM_WORLD)) != MPI_SUCCESS)
        mpi_error(err);
}

void server_send_draw(void)
{
    struct event e = { EVENT_DRAW, 0.0f, 0.0f, 0.0f };

    server_send_event(&e);
    server_draw();
}

void server_send_move(void)
{
    struct event e = { EVENT_MOVE, 0.0f, 0.0f, 0.0f };

    status_get_camera_pos(&e.x, &e.y, &e.z);
    server_send_event(&e);
}

void server_send_turn(void)
{
    struct event e = { EVENT_TURN, 0.0f, 0.0f, 0.0f };

    status_get_camera_rot(&e.x, &e.y, &e.z);
    server_send_event(&e);
}

void server_send_dist(void)
{
    struct event e = { EVENT_DIST, 0.0f, 0.0f, 0.0f };

    e.x = status_get_camera_dist();
    server_send_event(&e);
}

void server_send_magn(void)
{
    struct event e = { EVENT_MAGN, 0.0f, 0.0f, 0.0f };

    e.x = status_get_camera_magn();
    server_send_event(&e);
}

void server_send_zoom(void)
{
    struct event e = { EVENT_ZOOM, 0.0f, 0.0f, 0.0f };

    e.x = status_get_camera_zoom();
    server_send_event(&e);
}

void server_send_exit(void)
{
    struct event e = { EVENT_EXIT, 0.0f, 0.0f, 0.0f };

    server_send_event(&e);
}

/*---------------------------------------------------------------------------*/

static void server_init(void)
{
    glViewport(0, 0, status_get_viewport_w(), status_get_viewport_h());

    galaxy_init();
    star_init();

    server_send_draw();
}

static void server_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    status_draw_camera();
    galaxy_draw();

    MPI_Barrier(MPI_COMM_WORLD);
    SDL_GL_SwapBuffers();
}

static int server_loop(void)
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_MOUSEMOTION:     script_point(e.motion.x, e.motion.y); break;
        case SDL_MOUSEBUTTONDOWN: script_click(e.button.button,  1);    break;
        case SDL_MOUSEBUTTONUP:   script_click(e.button.button,  0);    break;
        case SDL_KEYDOWN:         script_keybd(e.key.keysym.sym, 1);    break;
        case SDL_KEYUP:           script_keybd(e.key.keysym.sym, 0);    break;

        case SDL_QUIT:
            server_send_exit();
            return 0;
        }

    return 1;
}

/*---------------------------------------------------------------------------*/

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s [OPTION]...\n", name);
    fprintf(stderr, "\t-s <filename>   Source Lua script\n");
    fprintf(stderr, "\t-f <filename>   Read binary star catalog\n");
    fprintf(stderr, "\t-t <filename>   Read ascii star catalog\n");
    fprintf(stderr, "\t-o <filename>   Write binary star catalog\n");
    fprintf(stderr, "\t-h              Help\n");
}

void server(int np, int argc, char *argv[])
{
    if (script_init())
    {
        int i;

        viewport_init(np);

        /* Parse command line options.  Load scripts and data files. */

        for (i = 1; i < argc; i++)
            if      (!strcmp(argv[i], "-s")) script_file(argv[++i]);
            else if (!strcmp(argv[i], "-f")) star_read_catalog_bin(argv[++i]);
            else if (!strcmp(argv[i], "-t")) star_read_catalog_txt(argv[++i]);
            else if (!strcmp(argv[i], "-o")) star_write_catalog(argv[++i]);
            else usage(argv[0]);

        status_init();
        viewport_sync(0, np);

        /* Initialize the main server window. */

        if (SDL_Init(SDL_INIT_VIDEO) == 0)
        {
            int w = status_get_viewport_w();
            int h = status_get_viewport_h();
            int m = SDL_OPENGL;

            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            SDL_WM_SetCaption(TITLE, TITLE);

            if (SDL_SetVideoMode(w, h, 0, m) && opengl_init())
            {
                server_init();

                /* Block on SDL events.  Service them as they arrive. */

                while (SDL_WaitEvent(NULL))
                    if (server_loop() == 0)
                        break;

                /* Ensure everyone finishes all events before exiting. */

                MPI_Barrier(MPI_COMM_WORLD);
            }
            else fprintf(stderr, "%s\n", SDL_GetError());

            SDL_Quit();
        }
        else fprintf(stderr, "%s\n", SDL_GetError());

        script_free();
    }
}

/*---------------------------------------------------------------------------*/
