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
#include "server.h"
#include "script.h"
#include "camera.h"
#include "sprite.h"
#include "galaxy.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

static void server_draw(void);

static int server_grab = 0;
static int server_idle = 0;
static int server_time = 0;

/*---------------------------------------------------------------------------*/

void enable_grab(int b)
{
    if (b && !server_grab)
    {
        SDL_WM_GrabInput(SDL_GRAB_ON);
        SDL_ShowCursor(0);
    }
    if (!b && server_grab)
    {
        SDL_WM_GrabInput(SDL_GRAB_OFF);
        SDL_ShowCursor(1);
    }
    server_grab = b;
}

void enable_idle(int b)
{
    server_idle = b;
    server_time = SDL_GetTicks();
}

/*---------------------------------------------------------------------------*/

void server_send(int type)
{
    int err;

    if ((err = MPI_Bcast(&type, 1, MPI_INTEGER, 0, MPI_COMM_WORLD)))
        mpi_error(err);
}

/*---------------------------------------------------------------------------*/

static void server_init(void)
{
    glViewport(0, 0, camera_get_viewport_w(), camera_get_viewport_h());

    sprite_init();
    galaxy_init();
    star_init();
}

static void server_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    camera_draw();
    galaxy_draw();
    sprite_draw();

    MPI_Barrier(MPI_COMM_WORLD);
    SDL_GL_SwapBuffers();
}

static void server_perf(void)
{
    static int fps_old = 0;
    int        fps_new = opengl_perf();

    if (fps_new != fps_old)
    {
        char buf[32];

        sprintf(buf, "%d FPS\n", fps_new);
        SDL_WM_SetCaption(buf, buf);

        fps_old = fps_new;
    }
}

/*---------------------------------------------------------------------------*/

static int server_loop(void)
{
    SDL_Event e;

    int t = SDL_GetTicks();
    int c = 0;

    while (SDL_PollEvent(&e))
    {
        /* Handle point grab toggle. */

        if (e.type == SDL_KEYUP && e.key.keysym.sym == 27) enable_grab(0);
        if (e.type == SDL_MOUSEBUTTONDOWN)                 enable_grab(1);

        /* Dispatch the event to the scripting system. */

        if (server_grab)
            switch (e.type)
            {
            case SDL_MOUSEMOTION:
                c += script_point(e.motion.xrel, e.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
                c += script_click(e.button.button, 1);
                break;
            case SDL_MOUSEBUTTONUP:
                c += script_click(e.button.button, 0);
                break;
            case SDL_USEREVENT:
                c += script_timer(t - server_time);
                break;
            case SDL_KEYDOWN:
                c += script_keybd(e.key.keysym.sym, 1);
                break;
            case SDL_KEYUP:
                c += script_keybd(e.key.keysym.sym, 0);
                break;
            }

        /* Handle a clean exit. */

        if (e.type == SDL_QUIT)
        {
            server_send(EVENT_EXIT);
            return 0;
        }
    }

    server_time = t;

    /* If the idle loop is enabled post a timer event. */

    if (server_idle)
    {
        e.type = SDL_USEREVENT;
        SDL_PushEvent(&e);
    }

    /* Redraw a dirty buffer. */

    if (c)
    {
        server_send(EVENT_DRAW);
        server_draw();
        server_perf();
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

static void parse(int argc, char *argv[])
{
    int i;

    for (i = 1; i < argc; i++)
        if      (!strcmp(argv[i], "-s")) script_file(argv[++i]);
        else if (!strcmp(argv[i], "-f")) star_read_catalog_bin(argv[++i]);
        else if (!strcmp(argv[i], "-t")) star_read_catalog_txt(argv[++i]);
        else if (!strcmp(argv[i], "-o")) star_write_catalog(argv[++i]);
        else usage(argv[0]);
}

void server(int np, int argc, char *argv[])
{
    if (script_init())
    {
        viewport_init(np);
        parse(argc, argv);

        camera_init();
        viewport_sync(np);

        /* Initialize the main server window. */

        if (SDL_Init(SDL_INIT_VIDEO) == 0)
        {
            int w = camera_get_viewport_w();
            int h = camera_get_viewport_h();
            int m = SDL_OPENGL;

            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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
