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

#define TILE_FLIP_X  1
#define TILE_FLIP_Y  2
#define TILE_OFFSET  4
#define TILE_MIRROR  8
#define TILE_TEST   16

struct tile
{
    int   flag;

    float o[3];                 /* World-space tile origin                   */
    float r[3];                 /* World-space tile right vector             */
    float u[3];                 /* World-space tile up vector                */
    float d[3];                 /* World-space view offset vector            */
    float p[4];                 /* World-space view mirror plane             */

    int   win_x;                /* Tile rectangle within host window         */
    int   win_y;
    int   win_w;
    int   win_h;

    int   pix_x;                /* Tile rectangle within display             */
    int   pix_y;
    int   pix_w;
    int   pix_h;

    float varrier_pitch;        /* Virtual barrier parameters.               */
    float varrier_angle;
    float varrier_thick;
    float varrier_shift;
    float varrier_cycle;
};

struct host
{
    char name[MAXNAME];
    int  tile[MAXTILE];

    int n;

    int win_x;                  /* Host window rectangle within desktop      */
    int win_y;
    int win_w;
    int win_h;

    int tot_x;                  /* Total display rectanglar bound            */
    int tot_y;
    int tot_w;
    int tot_h;
};

/*---------------------------------------------------------------------------*/

int startup_display(void);

void sync_display(void);

int  view_ortho(int, struct frustum *);
int  view_persp(int, struct frustum *, const float [3]);
int  draw_ortho(int, float, float);
int  draw_persp(int, float, float, const float[3]);

int  add_host(const char *, int, int, int, int);

/*---------------------------------------------------------------------------*/

int  send_add_tile(int, int, int, int, int);
void recv_add_tile(void);

/*---------------------------------------------------------------------------*/

void send_set_tile_flag       (int, int, int);
void send_set_tile_viewport   (int, int, int, int, int);
void send_set_tile_line_screen(int, float, float, float, float, float);
void send_set_tile_view_mirror(int, const float[4]);
void send_set_tile_view_offset(int, const float[3]);
void send_set_tile_position   (int, const float[3],
                                    const float[3],
                                    const float[3]);

void recv_set_tile_flag       (void);
void recv_set_tile_viewport   (void);
void recv_set_tile_line_screen(void);
void recv_set_tile_view_mirror(void);
void recv_set_tile_view_offset(void);
void recv_set_tile_position   (void);

/*---------------------------------------------------------------------------*/

void set_window_pos(int, int);
void set_window_siz(int);

void set_window_w(int);
void set_window_h(int);
int  get_window_w(void);
int  get_window_h(void);

/*---------------------------------------------------------------------------*/

int   get_viewport_x(void);
int   get_viewport_y(void);
int   get_viewport_w(void);
int   get_viewport_h(void);

void  get_tile_o(int, float[3]);
void  get_tile_r(int, float[3]);
void  get_tile_u(int, float[3]);

int   get_tile_flag(int);

float get_varrier_pitch(int);
float get_varrier_angle(int);
float get_varrier_thick(int);
float get_varrier_shift(int);
float get_varrier_cycle(int);

/*---------------------------------------------------------------------------*/

void send_set_background(const float[3], const float[3]);
void recv_set_background(void);

void draw_background(void);

/*---------------------------------------------------------------------------*/

#endif
