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
#include <string.h>

#include "opengl.h"
#include "version.h"
#include "tracker.h"
#include "joystick.h"
#include "console.h"
#include "display.h"
#include "buffer.h"
#include "script.h"
#include "entity.h"
#include "galaxy.h"
#include "utility.h"
#include "sound.h"
#include "image.h"
#include "event.h"
#include "server.h"
/*
#define GRAB 1
*/
/*---------------------------------------------------------------------------*/

static void server_draw(void);

static int server_full   = 0;
static int server_grab   = 0;
static int server_time   = 0;
static int server_mirror = 1;

static int timer_on = 0;

static float average_fps = 0.0f;

/*---------------------------------------------------------------------------*/

void grab(int b)
{
#if GRAB
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
#endif

    server_grab = b;
}

/*---------------------------------------------------------------------------*/

static void timer_callback(void)
{
    Uint32 t = SDL_GetTicks();
    SDL_Event e;

    /* On callback, push a user event giving time passed since last timer. */

    e.type      = SDL_USEREVENT;
    e.user.code = t - server_time;
    server_time = t;

    SDL_PushEvent(&e);
}

void enable_timer(int b)
{
    timer_on    = b;
    server_time = SDL_GetTicks();
}

/*---------------------------------------------------------------------------*/

static void init_server(void)
{
    glViewport(0, 0, get_window_w(), get_window_h());

    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,
                  GL_SEPARATE_SPECULAR_COLOR);

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPixelStorei(GL_PACK_ALIGNMENT,   1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

static int init_video(int w, int h, int m)
{
    free_all_entity_gl();

    set_window_w(w);
    set_window_h(h);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (SDL_SetVideoMode(w, h, 0, m | SDL_OPENGL | SDL_RESIZABLE))
    {
        init_opengl();
        init_server();
        init_all_entity_gl();

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static void server_swap(void)
{
#ifdef MPI
    glFinish();
    assert_mpi(MPI_Barrier(MPI_COMM_WORLD));
#endif
    SDL_GL_SwapBuffers();
}

static void server_draw(void)
{
    draw_background();

    if (server_mirror)
        draw_entity();

    draw_console();

    server_swap();
}

static void server_step(void)
{
    step_entity();
}

static void server_perf(void)
{
    static int fps_old = 0;
    int        fps_new = (int) opengl_perf(&average_fps);

    if (fps_new != fps_old)
    {
        char buf[32];

        sprintf(buf, "%s %s (%d FPS)", TITLE, version(), fps_new);
        SDL_WM_SetCaption(buf, buf);

        fps_old = fps_new;
    }
}

/*---------------------------------------------------------------------------*/

static int server_keydn(SDL_KeyboardEvent *k)
{
    if (console_is_enabled())
        return input_console(k->keysym.sym, k->keysym.unicode);
    else
        return do_keyboard_script(k->keysym.sym, 1);
}

static int server_keyup(SDL_KeyboardEvent *k)
{
    if (console_is_enabled())
        return 0;
    else
        return do_keyboard_script(k->keysym.sym, 0);
}

/*---------------------------------------------------------------------------*/

int which = 0;

static int server_loop(void)
{
    static int dirty = 1;
    static int count = 0;

    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        /* Handle point grab toggle. */

        if (e.type == SDL_KEYUP && e.key.keysym.sym == 27) grab(0);
        if (e.type == SDL_MOUSEBUTTONDOWN)                 grab(1);

        /* Handle global server control keys. */

        if (e.type == SDL_KEYDOWN)
            switch (e.key.keysym.sym)
            {
            case SDLK_F1:
                dirty |= set_console_enable(!console_is_enabled());
                break;
            case SDLK_F2:
                server_mirror = 1 - server_mirror;
                break;
            case SDLK_F3:
                set_window_siz(-1);
                dirty |= init_video(get_window_w(),
                                    get_window_h(), server_full);
                break;
            case SDLK_F4:
                set_window_siz(+1);
                dirty |= init_video(get_window_w(),
                                    get_window_h(), server_full);
                break;
            case SDLK_F5:
                server_full = SDL_FULLSCREEN - server_full;
                dirty |= init_video(get_window_w(),
                                    get_window_h(), server_full);
                break;
            default:
                break;
            }

        /* Dispatch the event to the scripting system. */

        if (server_grab)
            switch (e.type)
            {
            case SDL_VIDEORESIZE:
                dirty |= init_video(e.resize.w, e.resize.h, server_full);
                break;
            case SDL_MOUSEMOTION:
                dirty |= do_point_script(e.motion.xrel, e.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
                dirty |= do_click_script(e.button.button, 1);
                break;
            case SDL_MOUSEBUTTONUP:
                dirty |= do_click_script(e.button.button, 0);
                break;
            case SDL_JOYBUTTONDOWN:
                dirty |= do_joystick_script(e.jbutton.which,
                                            e.jbutton.button, 1);
                break;
            case SDL_JOYBUTTONUP:
                dirty |= do_joystick_script(e.jbutton.which,
                                            e.jbutton.button, 0);
                break;
            case SDL_KEYDOWN:
                dirty |= server_keydn(&e.key);
                break;
            case SDL_KEYUP:
                dirty |= server_keyup(&e.key);
                break;
            case SDL_USEREVENT:
                dirty |= do_timer_script(e.user.code);
                break;
            default:
                break;
            }

        /* Handle a clean exit.  TODO: remove redundancy. */

        if (e.type == SDL_QUIT)
        {
            pack_event(EVENT_EXIT);
            pack_event(EVENT_NULL);
            sync_buffer();

            return 0;
        }
    }

    /* Redraw a dirty buffer. */

    if (server_grab || dirty)
    {
        do_frame_script();

        if (dirty)
            pack_event(EVENT_DRAW);

        pack_event(EVENT_NULL);
        sync_buffer();

        if (dirty)
        {
            server_draw();
            server_perf();
            server_step();

            dirty = 0;
            count = count + 1;
        }
    }

    if (timer_on && server_grab) timer_callback();

    return 1;
}

/*---------------------------------------------------------------------------*/

void parse_args(int argc, char *argv[])
{
    int i, c = 1;

    /* Scan the list for Lua script arguments. */

    for (i = 1; i < argc; ++i)
        if      (strcmp(argv[i], "-f") == 0) i += 1;
        else if (strcmp(argv[i], "-H") == 0) i += 2;
        else if (strcmp(argv[i], "-T") == 0) i += 2;
        else
            add_argument(c++, argv[i]);

    /* Scan the list for Electro arguments. */

    for (i = 1; i < argc; ++i)
        if      (strcmp(argv[i], "-f") == 0)
        {
            load_script(argv[++i]);
        }

        else if (strcmp(argv[i], "-T") == 0)
        {
            const char *dat = argv[++i];
            const char *gal = argv[++i];

            prep_tyc_galaxy(dat, gal);
        }

        else if (strcmp(argv[i], "-H") == 0)
        {
            const char *dat = argv[++i];
            const char *gal = argv[++i];

            prep_hip_galaxy(dat, gal);
        }
}

void server(int argc, char *argv[])
{
    if (init_script())
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
                     SDL_INIT_AUDIO | SDL_INIT_TIMER) == 0)
        {
            /* Initialize all subsystems. */
        
            init_console(CONSOLE_COLS, CONSOLE_ROWS);
            init_display();
            init_tracker();
            init_buffer();
            init_sound();
            init_image();
            init_entity();
            init_joystick();

            parse_args(argc, argv);

            sync_display();

            if (init_video(get_window_w(),
                           get_window_h(), server_full))
            {
                SDL_EnableUNICODE(1);
                SDL_PauseAudio(0);

                grab(1);

                /* Block on SDL events.  Service them as they arrive. */

                while (SDL_WaitEvent(NULL))
                    if (server_loop() == 0)
                        break;
            }
            else fprintf(stderr, "%s\n", SDL_GetError());

            /* Ensure everyone finishes all events before exiting. */

#ifdef MPI
            assert_mpi(MPI_Barrier(MPI_COMM_WORLD));
#endif
            SDL_Quit();
        }
        else fprintf(stderr, "%s\n", SDL_GetError());
    }
}

/*---------------------------------------------------------------------------*/
