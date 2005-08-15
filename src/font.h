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

#ifndef FONT_H
#define FONT_H

/*---------------------------------------------------------------------------*/

int startup_font(void);

/*---------------------------------------------------------------------------*/

void send_set_font(const char *, float, float);
void recv_set_font(void);

/*---------------------------------------------------------------------------*/

int  get_font(void);

void fini_fonts(void);

void draw_font(int, const char *, int);
void aabb_font(int, const char *, float[6]);

/*---------------------------------------------------------------------------*/

#endif
