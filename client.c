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
        case EVENT_CAMERA_CREATE: camera_create(0);        break;

        case EVENT_CAMERA_DIST:   camera_set_dist(0, 0);        break;
        case EVENT_CAMERA_ZOOM:   camera_set_zoom(0, 0);        break;

        default: fprintf(stderr, "Uncaught event\n");
        }
    }
}

/*---------------------------------------------------------------------------*/

static void client_init(void)
{
    glViewport(0, 0, viewport_get_w(), viewport_get_h());
    /*
    galaxy_init();
    star_init();
    */
}

static void client_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    entity_render();
    /*
    galaxy_draw();
    */

    mpi_barrier();
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

void client(void)
{
    viewport_sync();

    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        int w = viewport_get_w();
        int h = viewport_get_h();
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

            mpi_barrier();
        }
        else fprintf(stderr, "%s\n", SDL_GetError());

        SDL_Quit();
    }
    else fprintf(stderr, "%s\n", SDL_GetError());
}

/*---------------------------------------------------------------------------*/
