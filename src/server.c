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

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "opengl.h"
#include "viewport.h"
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
    static int         timer_on = 1;

    /* Enable or disable an SDL timer callback. */

    if (b && !timer_on)
    {
        timer_id = SDL_AddTimer(1000 / 30, timer_callback, NULL);
        timer_on = 1;
        server_time = SDL_GetTicks();
    }

    if (!b && timer_on)
    {
        timer_on = 0;
        SDL_RemoveTimer(timer_id);
    }
}

/*---------------------------------------------------------------------------*/

void server_send(int type)
{
#ifndef NDEBUG
    printf("%d of %d: server_send(%s)\n", mpi_rank(),
                                          mpi_size(), event_string(type));
#endif

    mpi_share_integer(1, &type);
}

/*---------------------------------------------------------------------------*/

static void server_init(void)
{
    glViewport(0, 0, window_get_w(), window_get_h());

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    entity_init();
    opengl_check("server_init");
}

static void server_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    /* Draw the defined viewports to the stencil buffer. */

    glStencilFunc(GL_ALWAYS,   1, 0xFFFFFFFF);
    viewport_draw();

    /* Draw the mullions into the non-viewport parts of the frame buffer. */

    glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
    viewport_fill(0.1f, 0.1f, 0.1f);

    /* Draw the scene into the viewport parts of the frame buffer. */

    glStencilFunc(GL_EQUAL,    1, 0xFFFFFFFF);
    entity_render();

    /* Sync and swap. */

    mpi_barrier();
    SDL_GL_SwapBuffers();
}

static void server_perf(void)
{
    static int fps_old = 0;
    int        fps_new = opengl_perf();

    if (fps_new != fps_old)
    {
        char buf[32];

        sprintf(buf, "%s - %d FPS\n", TITLE, fps_new);
        SDL_WM_SetCaption(buf, buf);

        fps_old = fps_new;
    }
}

/*---------------------------------------------------------------------------*/

static int server_loop(void)
{
    static int dirty = 1;

    SDL_Event e;

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
                dirty |= script_point(e.motion.xrel, e.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
                dirty |= script_click(e.button.button, 1);
                break;
            case SDL_MOUSEBUTTONUP:
                dirty |= script_click(e.button.button, 0);
                break;
            case SDL_USEREVENT:
                dirty |= script_timer(e.user.code);
                break;
            case SDL_KEYDOWN:
                dirty |= script_keyboard(e.key.keysym.sym, 1);
                break;
            case SDL_KEYUP:
                dirty |= script_keyboard(e.key.keysym.sym, 0);
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

    if (dirty)
    {
        server_send(EVENT_DRAW);
        server_draw();
        server_perf();

        dirty = 0;
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

void server(int argc, char *argv[])
{
    int argi;

    if (script_init())
    {
        viewport_init();

        /* Read and execute all scripts given on the command line. */

        for (argi = 1; argi < argc; argi++)
            script_file(argv[argi]);

        viewport_sync();

        /* Initialize the main server window. */

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0)
        {
            int w = window_get_w();
            int h = window_get_h();
            int m = SDL_OPENGL;

            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
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

                mpi_barrier();
            }
            else fprintf(stderr, "%s\n", SDL_GetError());

            SDL_Quit();
        }
        else fprintf(stderr, "%s\n", SDL_GetError());

        script_free();
    }
}

/*---------------------------------------------------------------------------*/
