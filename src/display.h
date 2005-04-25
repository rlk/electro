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

#ifndef DISPLAY
#define DISPLAY

#include "frustum.h"

#define MAXNAME 32
#define MAXTILE  4

/*---------------------------------------------------------------------------*/

#define DEFAULT_X    0
#define DEFAULT_Y    0
#define DEFAULT_W  800
#define DEFAULT_H  600

/*---------------------------------------------------------------------------*/

struct tile
{
    float o[3];
    float r[3];
    float u[3];

    int   win_x;
    int   win_y;
    int   win_w;
    int   win_h;

    int   pix_x;
    int   pix_y;
    int   pix_w;
    int   pix_h;
};

struct host
{
    char        name[MAXNAME];
    struct tile tile[MAXTILE];

    int n;

    int win_x;
    int win_y;
    int win_w;
    int win_h;

    int pix_x;
    int pix_y;
    int pix_w;
    int pix_h;
};

/*---------------------------------------------------------------------------*/

void init_display(void);
void sync_display(void);

int  draw_ortho(struct frustum *,                 float, float, int);
int  draw_persp(struct frustum *, const float[3], float, float, int);

void add_host(const char *, int, int, int, int);
void add_tile(const char *, int, int, int, int,
                            int, int, int, int, float[3][3]);

/*---------------------------------------------------------------------------*/

void set_window_w(int);
void set_window_h(int);
int  get_window_w(void);
int  get_window_h(void);

int  get_viewport_x(void);
int  get_viewport_y(void);
int  get_viewport_w(void);
int  get_viewport_h(void);

/*---------------------------------------------------------------------------*/

void send_set_background(const float[3], const float[3]);
void recv_set_background(void);

void draw_background(void);

/*---------------------------------------------------------------------------*/

#endif
