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
#include "font.h"
#include "image.h"
#include "brush.h"
#include "entity.h"
#include "console.h"
#include "display.h"
#include "utility.h"

/*---------------------------------------------------------------------------*/

static int stereo_status = 0;

int quad_stereo_status(void)
{
    return stereo_status;
}

/*---------------------------------------------------------------------------*/

static void init_options(void)
{
    GLfloat a[4] = { 0.2f, 0.2f, 0.2f, 0.0f };

    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, a);
    glLightModeli (GL_LIGHT_MODEL_COLOR_CONTROL,
                   GL_SEPARATE_SPECULAR_COLOR);

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPixelStorei(GL_PACK_ALIGNMENT,   1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glClearColor(CLEAR_R, CLEAR_G, CLEAR_B, CLEAR_A);
}

int init_video(int width, int height, int full, int framed, int stereo)
{
    int mode = SDL_OPENGL;
    
    mode |= (framed ? 0 : SDL_NOFRAME);
    mode |= (full   ? SDL_FULLSCREEN : 0);

    /* Release all OpenGL resources in preparation for losing the context. */

    fini_fonts();
    fini_images();
	fini_brushes();
    fini_console();
    fini_entities();

    /* Configure the new visual. */

    SDL_GL_SetAttribute(SDL_GL_STEREO,  stereo);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (SDL_SetVideoMode(width, height, 0, mode))
    {
        /* Stereo is problematic.  Note whether we've got it. */

        SDL_GL_GetAttribute(SDL_GL_STEREO, &stereo_status);

        /* Initialize all OpenGL state in the new context. */

        init_opengl();
        init_options();
        init_images();
		init_brushes();
        init_entities();

        return 1;
    }
    else fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
        
    /* In the event of initialization failure, try again without stereo. */

    if (stereo)
        return init_video(width, height, full, framed, 0);

    /* If THAT didn't work, punt. */

    return 0;
}

/*---------------------------------------------------------------------------*/
