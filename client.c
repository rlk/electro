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

#include "opengl.h"
#include "star.h"
#include "galaxy.h"
#include "shared.h"
#include "status.h"
#include "client.h"

/*---------------------------------------------------------------------------*/

static void client_recv_draw(void)
{
    SDL_Event e;

    e.type      = SDL_USEREVENT;
    e.user.code = EVENT_DRAW;

    SDL_PushEvent(&e);
}

static void client_recv_exit(void)
{
    SDL_Event e;

    e.type      = SDL_USEREVENT;
    e.user.code = EVENT_EXIT;

    SDL_PushEvent(&e);
}

static void client_recv_event(void)
{
    size_t sz = sizeof (struct event);
    struct event e;
    int err;

    if ((err = MPI_Bcast(&e, sz, MPI_BYTE, 0, MPI_COMM_WORLD)) == MPI_SUCCESS)
    {
        switch (e.type)
        {
        case EVENT_DRAW: client_recv_draw();                   break;
        case EVENT_MOVE: status_set_camera_org(e.x, e.y, e.z); break;
        case EVENT_TURN: status_set_camera_rot(e.x, e.y, e.z); break;
        case EVENT_DIST: status_set_camera_dist(e.x);          break;
        case EVENT_MAGN: status_set_camera_magn(e.x);          break;
        case EVENT_ZOOM: status_set_camera_zoom(e.x);          break;
        case EVENT_EXIT: client_recv_exit();                   break;
        }
    }
    else mpi_error(err);
}

/*---------------------------------------------------------------------------*/

static void client_init(int id)
{
    glViewport(0, 0, status_get_viewport_w(), status_get_viewport_h());

    galaxy_init(id);
    star_init(id);
}

static void client_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    status_draw_camera();
    galaxy_draw();

    MPI_Barrier(MPI_COMM_WORLD);
    SDL_GL_SwapBuffers();
}

static int client_loop(void)
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_USEREVENT:
            switch (e.user.code)
            {
            case EVENT_DRAW: client_draw(); return 1;
            case EVENT_EXIT:                return 0;
            }
            break;

        case SDL_QUIT:
            return 0;
        }

    return 1;
}

void client(int np, int id)
{
    status_init();
    viewport_sync(id, np);

    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        int w = status_get_viewport_w();
        int h = status_get_viewport_h();
        int m = SDL_OPENGL | SDL_NOFRAME;

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        if (SDL_SetVideoMode(w, h, 0, m) && opengl_init())
        {
            client_init(id);

            /* Handle any SDL events. Block on server messages. */

            while (client_loop())
                client_recv_event();

            /* Ensure everyone finishes all events before exiting. */

            MPI_Barrier(MPI_COMM_WORLD);
        }
        else fprintf(stderr, "%s\n", SDL_GetError());

        SDL_Quit();
    }
    else fprintf(stderr, "%s\n", SDL_GetError());
}

/*---------------------------------------------------------------------------*/
