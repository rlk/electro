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
    int   flag;
    int   image;
    float s0;
    float s1;
    float t0;
    float t1;
};

/*---------------------------------------------------------------------------*/

int  init_sprite(void);
void draw_sprite(int, int, const float[16]);

int  send_create_sprite(const char *);
void recv_create_sprite(void);

void send_set_sprite_bounds(int, float, float, float, float);
void recv_set_sprite_bounds(void);

void delete_sprite(int);

/*---------------------------------------------------------------------------*/

void get_sprite_p(int, int, int, unsigned char[4]);
int  get_sprite_w(int);
int  get_sprite_h(int);

/*---------------------------------------------------------------------------*/

#endif
