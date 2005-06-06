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
#include "utility.h"

/*---------------------------------------------------------------------------*/

static void init_options(void)
{
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

int init_video(int width, int height, int framed, int stereo)
{
    int mode = SDL_OPENGL | (framed ? 0 : SDL_NOFRAME);

    fini_images();
    fini_entities();

	SDL_GL_SetAttribute(SDL_GL_STEREO,  stereo);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if (SDL_SetVideoMode(width, height, 0, mode))
	{
		init_opengl();
		init_options();
		init_images();
		init_entities();

		return 1;
	}
    else fprintf(stderr, "%s\n", SDL_GetError());

	if (stereo)
		return init_video(width, height, 0, framed);

    return 0;
}

/*---------------------------------------------------------------------------*/
