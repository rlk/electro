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
#include "entity.h"
#include "galaxy.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

static void server_draw(void);

static int server_grab = 0;
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

/*---------------------------------------------------------------------------*/

static Uint32 timer_callback(Uint32 interval, void *parameter)
{
    Uint32 t = SDL_GetTicks();
    SDL_Event e;

    /* On callback, push a user event giving time passed since last timer. */

    e.type      = SDL_USEREVENT;
    e.user.code = t - server_time;
    server_time = t;

    SDL_PushEvent(&e);

    /* Return the given interval to schedule a repeat timer event. */

    return interval;
}

void enable_idle(int b)
{
    static SDL_TimerID timer_id;

    /* Enable or disable an SDL timer callback. */

    if (b)
    {
        timer_id = SDL_AddTimer(1000 / 30, timer_callback, NULL);
        server_time = SDL_GetTicks();
    }
    else
        SDL_RemoveTimer(timer_id);
}

/*---------------------------------------------------------------------------*/

void server_send(int type)
{
    mpi_share_integer(1, &type);
}

/*---------------------------------------------------------------------------*/

static void server_init(void)
{
    glViewport(0, 0, viewport_get_w(), viewport_get_h());
    /*
    galaxy_init();
    star_init();
    */
    server_send(EVENT_DRAW);
    server_draw();
}

static void server_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    entity_render();
    /*
    galaxy_draw();
    */

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
                c += script_timer(e.user.code);
                break;
            case SDL_KEYDOWN:
                c += script_keyboard(e.key.keysym.sym, 1);
                break;
            case SDL_KEYUP:
                c += script_keyboard(e.key.keysym.sym, 0);
                break;
            }

        /* Handle a clean exit. */

        if (e.type == SDL_QUIT)
        {
            server_send(EVENT_EXIT);
            return 0;
        }
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
        viewport_sync(np);

        /* Initialize the main server window. */

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0)
        {
            int w = viewport_get_w();
            int h = viewport_get_h();
            int m = SDL_OPENGL;

            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            if (SDL_SetVideoMode(w, h, 0, m) && opengl_init())
            {
                server_init();
                script_start();

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
