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

#ifndef SPRITE_H
#define SPRITE_H

#include "opengl.h"

/*---------------------------------------------------------------------------*/

struct sprite
{
    GLuint texture;
    int    w;
    int    h;
    float  a;
};

/*---------------------------------------------------------------------------*/

int  sprite_create(const char *);
void sprite_render(int);
void sprite_delete(int);

void sprite_alpha(int, float);

/*---------------------------------------------------------------------------*/

#endif
