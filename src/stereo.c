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

#include "opengl.h"
#include "stereo.h"

/*---------------------------------------------------------------------------*/

void init_stereo(void);

/*---------------------------------------------------------------------------*/

static void enable_stereo_quad(int eye)
{
    if (eye < 0)
        glDrawBuffer(GL_BACK_LEFT);
    else
        glDrawBuffer(GL_BACK_RIGHT);
}

static void disable_stereo_quad(void)
{
    glDrawBuffer(GL_BACK);
}

/*---------------------------------------------------------------------------*/

static void enable_stereo_red_blue(int eye)
{
    if (eye < 0)
    {
        glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    else
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
}

static void disable_stereo_red_blue(void)
{
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

/*---------------------------------------------------------------------------*/

void enable_stereo(int mode, int eye)
{
    switch (mode)
    {
    case STEREO_QUAD:     enable_stereo_quad(eye);     break;
    case STEREO_RED_BLUE: enable_stereo_red_blue(eye); break;
    }
}

void disable_stereo(int mode)
{
    switch (mode)
    {
    case STEREO_QUAD:     disable_stereo_quad();     break;
    case STEREO_RED_BLUE: disable_stereo_red_blue(); break;
    }
}

/*---------------------------------------------------------------------------*/

