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

/*---------------------------------------------------------------------------*/

#define LIGHT_POSITIONAL  1
#define LIGHT_DIRECTIONAL 2

struct light
{
    int   type;
    float d[4];
};

/*---------------------------------------------------------------------------*/

int  light_init(void);
void light_draw(int, int, float[3], float[4][4]);

int  light_send_create(int);
void light_recv_create(void);

void light_send_color(int, float, float, float);
void light_recv_color(void);

void light_delete(int);

/*---------------------------------------------------------------------------*/
