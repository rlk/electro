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
#include "viewport.h"
#include "buffer.h"
#include "shared.h"
#include "client.h"
#include "camera.h"
#include "sprite.h"
#include "object.h"
#include "galaxy.h"
#include "light.h"
#include "pivot.h"
#include "image.h"
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

/*---------------------------------------------------------------------------*/

static void client_recv(void)
{
    char event = EVENT_NULL;

    buffer_sync();

    while ((event = unpack_event()))
    {
#ifndef NDEBUG
    printf("%d of %d: client_recv(%s)\n", mpi_rank(), mpi_size(),
                                          event_string(event));
#endif
        switch (event)
        {
        case EVENT_DRAW:          client_recv_draw();     break;
        case EVENT_EXIT:          client_recv_exit();     break;

        case EVENT_ENTITY_PARENT: entity_recv_parent();   break;
        case EVENT_ENTITY_DELETE: entity_recv_delete();   break;
        case EVENT_ENTITY_CLONE:  entity_recv_clone();    break;
        case EVENT_ENTITY_MOVE:   entity_recv_position(); break;
        case EVENT_ENTITY_TURN:   entity_recv_rotation(); break;
        case EVENT_ENTITY_SIZE:   entity_recv_scale();    break;
        case EVENT_ENTITY_FADE:   entity_recv_alpha();    break;
        case EVENT_ENTITY_FLAG:   entity_recv_flag();     break;

        case EVENT_CAMERA_CREATE: camera_recv_create();   break;
        case EVENT_SPRITE_CREATE: sprite_recv_create();   break;
        case EVENT_OBJECT_CREATE: object_recv_create();   break;
        case EVENT_GALAXY_CREATE: galaxy_recv_create();   break;
        case EVENT_LIGHT_CREATE:  light_recv_create();    break;
        case EVENT_PIVOT_CREATE:  pivot_recv_create();    break;
        case EVENT_IMAGE_CREATE:  image_recv_create();    break;

        case EVENT_GALAXY_MAGN:   galaxy_recv_magn();     break;
        case EVENT_CAMERA_DIST:   camera_recv_dist();     break;
        case EVENT_CAMERA_ZOOM:   camera_recv_zoom();     break;
        case EVENT_SPRITE_BOUNDS: sprite_recv_bounds();   break;
        case EVENT_LIGHT_COLOR:   light_recv_color();     break;
        }
    }
}

/*---------------------------------------------------------------------------*/

static void client_init(void)
{
    glViewport(0, 0, window_w(), window_h());

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    opengl_check("client_init");
}

static void client_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    entity_draw();

/*
    mpi_barrier_clients();
*/
    mpi_barrier_all();
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
        int w = window_w();
        int h = window_h();
        int m = SDL_OPENGL | SDL_NOFRAME;

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        SDL_ShowCursor(0);

        if (SDL_SetVideoMode(w, h, 0, m) && opengl_init())
        {
            buffer_init();
            image_init();
            client_init();
            entity_init();

            /* Handle any SDL events. Block on server messages. */

            while (client_loop())
                client_recv();

            /* Ensure everyone finishes all events before exiting. */

            mpi_barrier_all();
        }
        else fprintf(stderr, "%s\n", SDL_GetError());

        SDL_Quit();
    }
    else fprintf(stderr, "%s\n", SDL_GetError());
}

/*---------------------------------------------------------------------------*/
