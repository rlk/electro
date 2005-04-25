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

#include "opengl.h"
#include "utility.h"
#include "display.h"
#include "buffer.h"
#include "camera.h"
#include "sprite.h"
#include "object.h"
#include "galaxy.h"
#include "light.h"
#include "pivot.h"
#include "image.h"
#include "event.h"
#include "entity.h"

/*---------------------------------------------------------------------------*/

static void recv_draw_client(void)
{
    SDL_Event e;

    e.type      = SDL_USEREVENT;
    e.user.code = EVENT_DRAW;

    SDL_PushEvent(&e);
}

static void recv_exit_client(void)
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

    sync_buffer();

    while ((event = unpack_event()))
        switch (event)
        {
        case EVENT_DRAW:                 recv_draw_client();          break;
        case EVENT_EXIT:                 recv_exit_client();          break;

        case EVENT_CREATE_CAMERA:        recv_create_camera();        break;
        case EVENT_CREATE_SPRITE:        recv_create_sprite();        break;
        case EVENT_CREATE_OBJECT:        recv_create_object();        break;
        case EVENT_CREATE_GALAXY:        recv_create_galaxy();        break;
        case EVENT_CREATE_LIGHT:         recv_create_light();         break;
        case EVENT_CREATE_PIVOT:         recv_create_pivot();         break;
        case EVENT_CREATE_IMAGE:         recv_create_image();         break;
        case EVENT_CREATE_CLONE:         recv_create_clone();         break;

        case EVENT_PARENT_ENTITY:        recv_parent_entity();        break;
        case EVENT_DELETE_ENTITY:        recv_delete_entity();        break;

        case EVENT_SET_ENTITY_POSITION:  recv_set_entity_position();  break;
        case EVENT_SET_ENTITY_BASIS:     recv_set_entity_basis();     break;
        case EVENT_SET_ENTITY_SCALE:     recv_set_entity_scale();     break;
        case EVENT_SET_ENTITY_ALPHA:     recv_set_entity_alpha();     break;
        case EVENT_SET_ENTITY_FLAG:      recv_set_entity_flag();      break;
        case EVENT_SET_ENTITY_FRAG_PROG: recv_set_entity_frag_prog(); break;
        case EVENT_SET_ENTITY_VERT_PROG: recv_set_entity_vert_prog(); break;

        case EVENT_SET_GALAXY_MAGNITUDE: recv_set_galaxy_magnitude(); break;
        case EVENT_SET_CAMERA_OFFSET:    recv_set_camera_offset();    break;
        case EVENT_SET_CAMERA_STEREO:    recv_set_camera_stereo();    break;
        case EVENT_SET_SPRITE_BOUNDS:    recv_set_sprite_bounds();    break;
        case EVENT_SET_LIGHT_COLOR:      recv_set_light_color();      break;
        case EVENT_SET_BACKGROUND:       recv_set_background();       break;
        }
}

/*---------------------------------------------------------------------------*/

static void init_client(void)
{
    glViewport(0, 0, get_window_w(), get_window_h());

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(4.0);
}

static int init_video(int w, int h, int m)
{
    free_entity_gl();

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (SDL_SetVideoMode(w, h, 0, m))
    {
        init_opengl();
        init_client();
        init_entity_gl();

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static void client_swap(void)
{
#ifdef MPI
    glFinish();
    assert_mpi(MPI_Barrier(MPI_COMM_WORLD));
#endif
    SDL_GL_SwapBuffers();
}

static void client_draw(void)
{
    draw_background();
    draw_entity();

    client_swap();
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

/*---------------------------------------------------------------------------*/

void client(void)
{
    init_buffer();
    init_image();
    init_entity();
    sync_display();

    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        int w = get_window_w();
        int h = get_window_h();
        int m = SDL_OPENGL | SDL_NOFRAME;

        SDL_ShowCursor(0);

        if (init_video(w, h, m))
        {
            /* Handle any SDL events. Block on server messages. */

            while (client_loop())
                client_recv();

            /* Ensure everyone finishes all events before exiting. */

#ifdef MPI
            assert_mpi(MPI_Barrier(MPI_COMM_WORLD));
#endif
        }
        else fprintf(stderr, "%s\n", SDL_GetError());

        SDL_Quit();
    }
    else fprintf(stderr, "%s\n", SDL_GetError());
}

/*---------------------------------------------------------------------------*/
