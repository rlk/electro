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
#include "video.h"
#include "tracker.h"
#include "joystick.h"
#include "console.h"
#include "display.h"
#include "physics.h"
#include "buffer.h"
#include "script.h"
#include "entity.h"
#include "galaxy.h"
#include "utility.h"
#include "sound.h"
#include "image.h"
#include "brush.h"
#include "font.h"
#include "net.h"
#include "event.h"
#include "server.h"

/*---------------------------------------------------------------------------*/

/* A consistent time step is necessary to ensure reliable physics.  It       */
/* not only helps achieve stable integration, it ensures constent CFM        */
/* and ERP across hardware of varying power (this is an ODE issue).          */
/*                                                                           */
/* The proper time step is difficult to choose.  It must be greater          */
/* than 30Hz to be believable.  It should be less than 100Hz, as more is     */
/* unnecessary.  Ideally, it would match the refresh rate of the display.    */
/* However, not all displays run at the same rate.                           */
/*                                                                           */
/* 60Hz is chosen as it best fits these criteria.  Some stuttering may be    */
/* evident when the real frame rate is near to but not equal to 60.  For     */
/* example, on a 72Hz display, 1 out of 5 updates will include 2 time        */
/* steps.  This is unfortunate, but difficult to work around.                */

#define TIME_STEP (1.0f / 60.0f)

/*---------------------------------------------------------------------------*/

static void server_draw(void);

static int server_grab    = 0;

static int    timer_on    = 0;
static double timer_last  = 0;
static double timer_value = 0;

static float average_fps  = 0;

static int   console_port = 0;
static char *console_log  = NULL;
static int   tracker_key  = TRACKER_KEY;
static int   control_key  = CONTROL_KEY;
static int   head_sensor  = 0;

/*---------------------------------------------------------------------------*/

void grab(int b)
{
    if (server_grab)
    {
        if (b)
        {
            SDL_WM_GrabInput(SDL_GRAB_ON);
            SDL_ShowCursor(0);
        }
        else
        {
            SDL_WM_GrabInput(SDL_GRAB_OFF);
            SDL_ShowCursor(1);
        }
    }
}

void enable_timer(int b)
{
    timer_on    = b;
    timer_last  = (double) SDL_GetTicks() / 1000.0;
    timer_value = 0;
}

/*---------------------------------------------------------------------------*/

static int server_tick(void)
{
    int dirty = 0;

    if (timer_on)
    {
        float now = (float) SDL_GetTicks() / 1000.0f;

        if (now - timer_last > TIME_STEP)
        {
            timer_value += now - timer_last;
            timer_last   = now;

            while (timer_value > TIME_STEP)
            {
                dirty |= step_entities(TIME_STEP, head_sensor);
                dirty |= do_timer_script(TIME_STEP);

                timer_value -= TIME_STEP;
            }
        }
    }
    return dirty;
}

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_host_background();
    draw_entities();
    draw_console();
    server_swap();

    opengl_check("server_draw");
}

static void server_perf(void)
{
    static int fps_old = 0;
    int        fps_new = (int) opengl_perf(&average_fps);

    if (fps_new != fps_old)
    {
        char buf[32];

        sprintf(buf, "%s (%d FPS)", TITLE, fps_new);
        SDL_WM_SetCaption(buf, buf);

        printf("%3d fps\n", fps_new);

        fps_old = fps_new;
    }
}

/*---------------------------------------------------------------------------*/

static int server_keydn(SDL_KeyboardEvent *k)
{
    if (console_is_enabled())
        return input_console(k->keysym.sym, k->keysym.unicode);

    if (do_keyboard_script(k->keysym.sym, 1))
        return 1;

    return 0;
}

static int server_keyup(SDL_KeyboardEvent *k)
{
    if (console_is_enabled())
        return input_console(0, 0);

    if (do_keyboard_script(k->keysym.sym, 0))
        return 1;

    return 0;
}

/*---------------------------------------------------------------------------*/

void send_user_event(const char *str)
{
    SDL_Event e;

    e.type = SDL_USEREVENT;

    if (str)
        e.user.data1 = memdup(str, 1, 1 + strlen(str));
    else
        e.user.data1 = NULL;

    SDL_PushEvent(&e);
}

int recv_user_event(SDL_UserEvent *e)
{
    if (e->data1)
    {
        do_command(e->data1);
        free(e->data1);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

int which = 0;

static int server_loop(void)
{
    static int dirty = 1;
    static int count = 0;

    unsigned int i;
    unsigned int b;

    SDL_Event e;

    /* Service all queued events. */

    while (SDL_PollEvent(&e))
    {
        /* Handle global server control keys. */

        if (e.type == SDL_KEYDOWN)
        {
            switch (e.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                e.type = SDL_QUIT;
                break;
            case SDLK_F1:
                dirty |= set_console_enable(!console_is_enabled());
                break;
            case SDLK_F2:
                set_window_full(!get_window_full());
                dirty |= init_video(get_window_w(),
                                    get_window_h(),
                                    get_window_full(),
                                    get_window_framed(),
                                    get_window_stereo());
                break;
            case SDLK_F3:
                set_window_siz(-1);
                dirty |= init_video(get_window_w(),
                                    get_window_h(),
                                    get_window_full(),
                                    get_window_framed(),
                                    get_window_stereo());
                break;
            case SDLK_F4:
                set_window_siz(+1);
                dirty |= init_video(get_window_w(),
                                    get_window_h(),
                                    get_window_full(),
                                    get_window_framed(),
                                    get_window_stereo());
                break;
            default:
                break;
            }
        }

        /* Dispatch the event to the scripting system. */

        switch (e.type)
        {
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
            dirty |= recv_user_event(&e.user);
            break;
        case SDL_VIDEOEXPOSE:
            dirty |= 1;
        default:
            break;
        }

        /* Dispatch tracker button events as joystick button events. */

        while (get_tracker_buttons(&i, &b))
            dirty |= do_joystick_script(0, i, b);

        /* Handle a clean exit. */

        if (e.type == SDL_QUIT)
        {
            send_event(EVENT_EXIT);
            send_event(EVENT_NULL);
            sync_buffer();

            return 0;
        }
    }

    /* Redraw a dirty buffer. */

    if (dirty)
    {
        step_images();

        send_event(EVENT_DRAW);
        send_event(EVENT_NULL);
        sync_buffer();

        server_tick();

        do_frame_script();
        server_draw();
        server_perf();

        dirty = 0;
        count = count + 1;
    }

    if (timer_on)
        send_user_event(NULL);

    return 1;
}

/*---------------------------------------------------------------------------*/

void parse_options(int argc, char *argv[])
{
    int i, c = 1;

    /* Scan the list for Electro options. */

    for (i = 1; i < argc; ++i)
        if      (strcmp(argv[i], "-f") == 0 && i < argc - 1)
            i++;

        else if (strcmp(argv[i], "-T") == 0 && i < argc - 2)
        {
            const char *dat = argv[++i];
            const char *gal = argv[++i];

            prep_tyc_galaxy(dat, gal);
        }

        else if (strcmp(argv[i], "-H") == 0 && i < argc - 2)
        {
            const char *dat = argv[++i];
            const char *gal = argv[++i];

            prep_hip_galaxy(dat, gal);
        }

        else if (strcmp(argv[i], "-p") == 0 && i < argc - 1)
            console_port = atoi(argv[++i]);
        else if (strcmp(argv[i], "-l") == 0 && i < argc - 1)
            console_log  = argv[++i];

        else if (strcmp(argv[i], "-t") == 0 && i < argc - 1)
            tracker_key = atoi(argv[++i]);

        else if (strcmp(argv[i], "-c") == 0 && i < argc - 1)
            control_key = atoi(argv[++i]);

        else if (strcmp(argv[i], "-h") == 0 && i < argc - 1)
            head_sensor = atoi(argv[++i]);

        else if (strcmp(argv[i], "-m") == 0)
            server_grab = 1;

        else
            add_argument(c++, argv[i]);
}

void parse_scripts(int argc, char *argv[])
{
    int i;

    /* Scan the list for Lua scripts. */

    for (i = 1; i < argc; ++i)
        if      (strcmp(argv[i], "-f") == 0)
            load_script(argv[++i]);
        else if (strcmp(argv[i], "-H") == 0) i += 2;
        else if (strcmp(argv[i], "-T") == 0) i += 2;
        else if (strcmp(argv[i], "-p") == 0) i += 1;
        else if (strcmp(argv[i], "-l") == 0) i += 1;
        else if (strcmp(argv[i], "-t") == 0) i += 1;
        else if (strcmp(argv[i], "-c") == 0) i += 1;
        else if (strcmp(argv[i], "-h") == 0) i += 1;
        else if (strcmp(argv[i], "-m") == 0) i += 0;
}

/*---------------------------------------------------------------------------*/

void server(int argc, char *argv[])
{
    if (init_script())
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) == 0)
        {
            parse_options(argc, argv);

            /* Initialize all subsystems. */
        
            if (startup_console(console_log, CONSOLE_COLS, CONSOLE_ROWS) &&
                startup_joystick() &&
                startup_physics()  &&
                startup_buffer()   &&
                startup_display()  &&
                startup_entity()   &&
                startup_sound()    &&
                startup_image()    &&
                startup_brush()    &&
                startup_font()     &&
                startup_net(console_port))
            {
                acquire_tracker(tracker_key, control_key);

                parse_scripts(argc, argv);

                sync_display();

                if (init_video(get_window_w(),
                               get_window_h(),
                               get_window_full(),
                               get_window_framed(),
                               get_window_stereo()))
                {
                    SDL_Event e = { SDL_USEREVENT };

                    /* Kickstart SDL event processing. */

                    SDL_PushEvent(&e);
                    SDL_EnableUNICODE(1);
                    SDL_PauseAudio(0);

                    /* Loop, handling SDL events. */

                    grab(1);

                    while (SDL_WaitEvent(NULL))
                        if (server_loop() == 0)
                            break;

                    grab(0);
                }

                release_tracker();
            }

            /* Ensure everyone finishes all events before exiting. */
#ifdef MPI
            assert_mpi(MPI_Barrier(MPI_COMM_WORLD));
#endif
            SDL_Quit();
        }
        else fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    }
}

/*---------------------------------------------------------------------------*/
