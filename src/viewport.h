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

#define DEFAULT_X -400.0f
#define DEFAULT_Y -300.0f
#define DEFAULT_W  800.0f
#define DEFAULT_H  600.0f

/*---------------------------------------------------------------------------*/

#define NAMELEN 32

struct viewport
{
    char name[NAMELEN];      /* Node host name                               */
    float X;                 /* Window X position, relative to desktop       */
    float Y;                 /* Window Y position, relative to desktop       */
    float x;                 /* Tile X position, relative to center of array */
    float y;                 /* Tile Y position, relative to center of array */
    float w;                 /* Tile width                                   */
    float h;                 /* Tile height                                  */
};

void viewport_init(void);
void viewport_draw(void);
void viewport_fill(float, float, float);
void viewport_tile(const char *, float, float, float, float, float, float);
void viewport_sync(void);

/*---------------------------------------------------------------------------*/

void viewport_set(float, float, float, float, float, float);

float viewport_total_x(void);
float viewport_total_y(void);
float viewport_total_w(void);
float viewport_total_h(void);

float viewport_local_x(void);
float viewport_local_y(void);
float viewport_local_w(void);
float viewport_local_h(void);

float viewport_scale(void);

/*---------------------------------------------------------------------------*/

int window_w(void);
int window_h(void);

/*---------------------------------------------------------------------------*/
