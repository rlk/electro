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

#ifndef LIGHT_H
#define LIGHT_H

/*---------------------------------------------------------------------------*/

#define LIGHT_POSITIONAL  1
#define LIGHT_DIRECTIONAL 2

struct light
{
    int   count;
    int   type;
    float d[4];
};

/*---------------------------------------------------------------------------*/

int  init_light(void);
void draw_light(int, int, const float[16], float);

int  send_create_light(int);
void recv_create_light(void);

void send_set_light_color(int, float, float, float);
void recv_set_light_color(void);

void clone_light(int);
void delete_light(int);

/*---------------------------------------------------------------------------*/

#endif
