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

int  sprite_init(void);
void sprite_draw(int, int, const float[16]);

int  sprite_send_create(const char *);
void sprite_recv_create(void);

void sprite_send_bounds(int, float, float, float, float);
void sprite_recv_bounds(void);

void sprite_delete(int);

/*---------------------------------------------------------------------------*/

void sprite_get_p(int, int, int, unsigned char[4]);
int  sprite_get_w(int);
int  sprite_get_h(int);

/*---------------------------------------------------------------------------*/

#endif
