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
#include "image.h"
#include "entity.h"
#include "display.h"

/*---------------------------------------------------------------------------*/

int video_stereo     = 0;
int video_fullscreen = 0;

/*---------------------------------------------------------------------------*/

static void init_options(int w, int h)
{
    glViewport(0, 0, w, h);

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

int init_video(int w, int h, int r)
{
    int m = r ? SDL_OPENGL | SDL_NOFRAME : SDL_OPENGL | SDL_RESIZABLE;

    fini_images();
    fini_entities();

    if (video_fullscreen)
        m |= SDL_FULLSCREEN;

    if (video_stereo)
        SDL_GL_SetAttribute(SDL_GL_STEREO, 1);
    else
        SDL_GL_SetAttribute(SDL_GL_STEREO, 0);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (SDL_SetVideoMode(w, h, 0, m))
    {
        init_opengl();
        init_options(w, h);
        init_images();
        init_entities();

        return 1;
    }
    else if (video_stereo)
        return set_video_stereo(0, r);

    return 0;
}

/*---------------------------------------------------------------------------*/

int set_video_stereo(int s, int r)
{
    video_stereo = s;

    return init_video(get_window_w(), get_window_h(), r);
}

int set_video_fullscreen(int f, int r)
{
    video_fullscreen = f;

    return init_video(get_window_w(), get_window_h(), r);
}

/*---------------------------------------------------------------------------*/
