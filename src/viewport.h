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
int  viewport_get_x(void);
int  viewport_get_y(void);
int  viewport_get_w(void);
int  viewport_get_h(void);

/*---------------------------------------------------------------------------*/

int  window_get_w(void);
int  window_get_h(void);

/*---------------------------------------------------------------------------*/
