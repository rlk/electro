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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "opengl.h"
#include "galaxy.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

void galaxy_init(void)
{
    const char *vert_program;
    const char *frag_program;

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);

    glEnable(GL_POINT_SPRITE_ARB);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_ONE, GL_ONE);

    glPointSize(4);

    glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
    /*
    vert_program = opengl_read("star.vp");
    frag_program = opengl_read("star.fp");

    glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB,
                       strlen(vert_program), vert_program);

    if (glGetError() != GL_NO_ERROR)
        printf("vert_program: %s", glGetString(GL_PROGRAM_ERROR_STRING_ARB));

    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB,
                       strlen(frag_program), frag_program);

    if (glGetError() != GL_NO_ERROR)
        printf("frag_program: %s", glGetString(GL_PROGRAM_ERROR_STRING_ARB));
    */
}

/*---------------------------------------------------------------------------*/

void circum_draw(void)
{
    int i;

    glDisable(GL_TEXTURE_2D);
    {
        glBegin(GL_LINE_LOOP);
        {
            for (i = 0; i < 360; i++)
                glVertex3d(RADIUS * sin(PI * i / 180.0), 0,
                           RADIUS * cos(PI * i / 180.0));
        }
        glEnd();
    }
    glEnable(GL_TEXTURE_2D);
}

void galaxy_draw(void)
{
    circum_draw();
    star_draw();
}

/*---------------------------------------------------------------------------*/
