/*    |||   ELECTRO                                                          */
/*    O o   Copyright (C) 2005 Robert Kooima                                 */
/*     -                                                                     */
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

#ifndef DISPLAY
#define DISPLAY

#define MAXNAME 32
#define MAXTILE  4

/*---------------------------------------------------------------------------*/

#define DEFAULT_X  100
#define DEFAULT_Y  100
#define DEFAULT_W  800
#define DEFAULT_H  600

/*---------------------------------------------------------------------------*/

struct tile
{
    int x;
    int y;
    int w;
    int h;

    float p[3];
    float r[3];
    float u[3];
};

struct host
{
    char        name[MAXNAME];
    struct tile tile[MAXTILE];

    int X;
    int Y;
    int W;
    int H;
    int n;
};

/*---------------------------------------------------------------------------*/

int  init_display(void);
void sync_display(void);

void add_host(const char *, int, int, int, int);
void add_tile(const char *, int, int, int, int, const float[3],
                                                const float[3],
                                                const float[3]);

/*---------------------------------------------------------------------------*/

int get_window_w(void);
int get_window_h(void);

/*---------------------------------------------------------------------------*/

void send_set_background(const float[3], const float[3]);
void recv_set_background(void);

void draw_background(void);

/*---------------------------------------------------------------------------*/

#endif
