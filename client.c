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
#include "shared.h"
#include "client.h"
#include "camera.h"
#include "shared.h"
#include "sprite.h"
#include "entity.h"
#include "galaxy.h"
#include "star.h"

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

static void client_recv(void)
{
    int type;

    if (mpi_share_integer(1, &type))
    {
        switch (type)
        {
        case EVENT_DRAW:          client_recv_draw();      break;
        case EVENT_EXIT:          client_recv_exit();      break;

        case EVENT_ENTITY_PARENT: entity_parent(0, 0);     break;
        case EVENT_ENTITY_DELETE: entity_delete(0);        break;

        case EVENT_ENTITY_MOVE:   entity_position(0, 0, 0, 0); break;
        case EVENT_ENTITY_TURN:   entity_rotation(0, 0, 0, 0); break;
        case EVENT_ENTITY_SIZE:   entity_scale(0, 0, 0, 0);    break;

        case EVENT_SPRITE_CREATE: sprite_create(NULL);     break;

        case EVENT_CAMERA_MOVE: camera_set_org(0, 0, 0);   break;
        case EVENT_CAMERA_TURN: camera_set_rot(0, 0, 0);   break;
        case EVENT_CAMERA_DIST: camera_set_dist(0);        break;
        case EVENT_CAMERA_MAGN: camera_set_magn(0);        break;
        case EVENT_CAMERA_ZOOM: camera_set_zoom(0);        break;
        }
    }
}

/*---------------------------------------------------------------------------*/

static void client_init(void)
{
    glViewport(0, 0, camera_get_viewport_w(), camera_get_viewport_h());

    galaxy_init();
    star_init();
}

static void client_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    camera_draw();
    galaxy_draw();
    entity_render();

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
    camera_init();
    viewport_sync(np);

    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        int w = camera_get_viewport_w();
        int h = camera_get_viewport_h();
        int m = SDL_OPENGL | SDL_NOFRAME;

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        SDL_ShowCursor(0);

        if (SDL_SetVideoMode(w, h, 0, m) && opengl_init())
        {
            client_init();

            /* Handle any SDL events. Block on server messages. */

            while (client_loop())
                client_recv();

            /* Ensure everyone finishes all events before exiting. */

            MPI_Barrier(MPI_COMM_WORLD);
        }
        else fprintf(stderr, "%s\n", SDL_GetError());

        SDL_Quit();
    }
    else fprintf(stderr, "%s\n", SDL_GetError());
}

/*---------------------------------------------------------------------------*/
