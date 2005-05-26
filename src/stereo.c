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

int stereo_varrier(int eye, int tile, int pass)
{
    return 0;
}

int stereo_quad(int eye, int tile, int pass)
{
    if (pass == 0)
    {
        if (eye)
            glDrawBuffer(GL_BACK_RIGHT);
        else
            glDrawBuffer(GL_BACK_LEFT);
        return 1;
    }
    else
        return 0;
}

int stereo_red_blue(int eye, int tile, int pass)
{
    if (pass == 0)
    {
        if (eye)
        {
            glColorMask(0, 0, 1, 0);
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        else
        {
            glColorMask(1, 0, 0, 0);
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        return 1;
    }
    else
    {
        glColorMask(1, 1, 1, 1);
        return 0;
    }
}

/*---------------------------------------------------------------------------*/

