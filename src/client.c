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
#include "video.h"
#include "utility.h"
#include "display.h"
#include "buffer.h"
#include "camera.h"
#include "sprite.h"
#include "object.h"
#include "string.h"
#include "galaxy.h"
#include "light.h"
#include "pivot.h"
#include "image.h"
#include "brush.h"
#include "font.h"
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

    while ((event = recv_event()))
        switch (event)
        {
        case EVENT_DRAW:                 recv_draw_client();          break;
        case EVENT_EXIT:                 recv_exit_client();          break;

        case EVENT_CREATE_CAMERA:        recv_create_camera();        break;
        case EVENT_CREATE_SPRITE:        recv_create_sprite();        break;
        case EVENT_CREATE_OBJECT:        recv_create_object();        break;
        case EVENT_CREATE_STRING:        recv_create_string();        break;
        case EVENT_CREATE_GALAXY:        recv_create_galaxy();        break;
        case EVENT_CREATE_LIGHT:         recv_create_light();         break;
        case EVENT_CREATE_PIVOT:         recv_create_pivot();         break;
        case EVENT_CREATE_CLONE:         recv_create_clone();         break;
        case EVENT_CREATE_IMAGE:         recv_create_image();         break;
        case EVENT_CREATE_BRUSH:         recv_create_brush();         break;

        case EVENT_PARENT_ENTITY:        recv_parent_entity();        break;
        case EVENT_DELETE_ENTITY:        recv_delete_entity();        break;
        case EVENT_DELETE_IMAGE:         recv_delete_image();         break;
        case EVENT_DELETE_BRUSH:         recv_delete_brush();         break;

        case EVENT_SET_ENTITY_POSITION:  recv_set_entity_position();  break;
        case EVENT_SET_ENTITY_BASIS:     recv_set_entity_basis();     break;
        case EVENT_SET_ENTITY_SCALE:     recv_set_entity_scale();     break;
        case EVENT_SET_ENTITY_ALPHA:     recv_set_entity_alpha();     break;
        case EVENT_SET_ENTITY_FLAGS:     recv_set_entity_flags();     break;

        case EVENT_CREATE_MESH:          recv_create_mesh();          break;
        case EVENT_CREATE_VERT:          recv_create_vert();          break;
        case EVENT_CREATE_FACE:          recv_create_face();          break;
        case EVENT_CREATE_EDGE:          recv_create_edge();          break;

        case EVENT_DELETE_MESH:          recv_delete_mesh();          break;
        case EVENT_DELETE_VERT:          recv_delete_vert();          break;
        case EVENT_DELETE_FACE:          recv_delete_face();          break;
        case EVENT_DELETE_EDGE:          recv_delete_edge();          break;

        case EVENT_SET_MESH:             recv_set_mesh();             break;
        case EVENT_SET_VERT:             recv_set_vert();             break;
        case EVENT_SET_FACE:             recv_set_face();             break;
        case EVENT_SET_EDGE:             recv_set_edge();             break;

        case EVENT_SET_IMAGE_PIXELS:     recv_set_image_pixels();     break;
        case EVENT_SET_BRUSH_IMAGE:      recv_set_brush_image();      break;
        case EVENT_SET_BRUSH_COLOR:      recv_set_brush_color();      break;
        case EVENT_SET_BRUSH_FLAGS:      recv_set_brush_flags();      break;
        case EVENT_SET_BRUSH_FRAG_PROG:  recv_set_brush_frag_prog();  break;
        case EVENT_SET_BRUSH_VERT_PROG:  recv_set_brush_vert_prog();  break;
        case EVENT_SET_BRUSH_FRAG_PARAM: recv_set_brush_frag_param(); break;
        case EVENT_SET_BRUSH_VERT_PARAM: recv_set_brush_vert_param(); break;

        case EVENT_SET_GALAXY_MAGNITUDE: recv_set_galaxy_magnitude(); break;
        case EVENT_SET_CAMERA_OFFSET:    recv_set_camera_offset();    break;
        case EVENT_SET_CAMERA_STEREO:    recv_set_camera_stereo();    break;
        case EVENT_SET_SPRITE_RANGE:     recv_set_sprite_range();     break;
        case EVENT_SET_SPRITE_BRUSH:     recv_set_sprite_brush();     break;
        case EVENT_SET_STRING_FILL:      recv_set_string_fill();      break;
        case EVENT_SET_STRING_LINE:      recv_set_string_line();      break;
        case EVENT_SET_STRING_TEXT:      recv_set_string_text();      break;
        case EVENT_SET_LIGHT_COLOR:      recv_set_light_color();      break;
        case EVENT_SET_BACKGROUND:       recv_set_background();       break;
        case EVENT_SET_FONT:             recv_set_font();             break;

        case EVENT_ADD_TILE:             recv_add_tile();             break;
        case EVENT_SET_TILE_FLAGS:       recv_set_tile_flags();       break;
        case EVENT_SET_TILE_VIEWPORT:    recv_set_tile_viewport();    break;
        case EVENT_SET_TILE_POSITION:    recv_set_tile_position();    break;
        case EVENT_SET_TILE_LINE_SCREEN: recv_set_tile_line_screen(); break;
        case EVENT_SET_TILE_VIEW_MIRROR: recv_set_tile_view_mirror(); break;
        case EVENT_SET_TILE_VIEW_OFFSET: recv_set_tile_view_offset(); break;
        }
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    draw_host_background();
    draw_entities();

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
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_ShowCursor(0);

        if (startup_buffer()  &&
            startup_display() &&
            startup_image()   &&
            startup_brush()   &&
            startup_font()    &&
            startup_entity())
        {
            sync_display();

            if (init_video(get_window_w(),
                           get_window_h(),
                           get_window_full(),
                           get_window_framed(),
                           get_window_stereo()))
            {
                /* Handle any SDL events. Block on server messages. */

                while (client_loop())
                    client_recv();
            }
            else fprintf(stderr, "%s\n", SDL_GetError());
        }

        /* Ensure everyone finishes all events before exiting. */
#ifdef MPI
        assert_mpi(MPI_Barrier(MPI_COMM_WORLD));
#endif
        SDL_Quit();
    }
    else fprintf(stderr, "%s\n", SDL_GetError());
}

/*---------------------------------------------------------------------------*/
