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

#ifndef SPRITE_H
#define SPRITE_H

/*---------------------------------------------------------------------------*/

#define MAXSPRITE 128

struct sprite
{
    GLuint texture;
    int    w;
    int    h;

    float  pos[2];
    float  rot;
    float  size;
    float  alpha;
};

/*---------------------------------------------------------------------------*/

int   sprite_init(void);
void  sprite_draw(void);

int   sprite_load(const char *);
void  sprite_free(int);

void  sprite_move(int, float, float);
void  sprite_turn(int, float);
void  sprite_size(int, float);
void  sprite_fade(int, float);

/*---------------------------------------------------------------------------*/

#endif
