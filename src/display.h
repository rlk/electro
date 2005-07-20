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

#define MAXNAME 32
#define MAXTILE  4

/*---------------------------------------------------------------------------*/

#define DEFAULT_NAME "default"

#define DEFAULT_X    0
#define DEFAULT_Y    0
#define DEFAULT_W  800
#define DEFAULT_H  600

#define DEFAULT_OX  -0.5f
#define DEFAULT_OY (-0.5f * DEFAULT_H / DEFAULT_W)
#define DEFAULT_OZ  -1.0f

#define DEFAULT_RX  1.0f
#define DEFAULT_RY  0.0f
#define DEFAULT_RZ  0.0f

#define DEFAULT_UX  0.0f
#define DEFAULT_UY (1.0f * DEFAULT_H / DEFAULT_W)
#define DEFAULT_UZ  0.0f

#define DEFAULT_VARRIER_PITCH 1.00f
#define DEFAULT_VARRIER_ANGLE 0.00f
#define DEFAULT_VARRIER_THICK 0.00f
#define DEFAULT_VARRIER_SHIFT 0.00f
#define DEFAULT_VARRIER_CYCLE 0.75f

/*---------------------------------------------------------------------------*/

/* Draw flags. */

#define DRAW_VARRIER_TEXGEN 0x0001

/*---------------------------------------------------------------------------*/

#define HOST_FULL    1
#define HOST_STEREO  2
#define HOST_FRAMED  4

#define TILE_FLIP_X  1
#define TILE_FLIP_Y  2
#define TILE_OFFSET  4
#define TILE_MIRROR  8
#define TILE_TEST   16

struct tile
{
    int   flags;

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
    int  flags;
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

int  draw_ortho(int, float, float);
int  draw_persp(int, float, float, const float[3]);

int  add_host(const char *, int, int, int, int);

/*---------------------------------------------------------------------------*/

int  send_add_tile(int, int, int, int, int);
void recv_add_tile(void);

/*---------------------------------------------------------------------------*/

void send_set_host_flags      (int, int, int);
void send_set_tile_flags      (int, int, int);
void send_set_tile_viewport   (int, int, int, int, int);
void send_set_tile_line_screen(int, float, float, float, float, float);
void send_set_tile_view_mirror(int, const float[4]);
void send_set_tile_view_offset(int, const float[3]);
void send_set_tile_position   (int, const float[3],
                                    const float[3],
                                    const float[3]);

void recv_set_tile_flags      (void);
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
int  get_window_full(void);
int  get_window_framed(void);
int  get_window_stereo(void);

/*---------------------------------------------------------------------------*/

void  get_display_union(float[4]);
void  get_display_bound(float[6]);

void  get_tile_o(int, float[3]);
void  get_tile_r(int, float[3]);
void  get_tile_u(int, float[3]);
void  get_tile_n(int, float[3]);

int   get_tile_flags(int);

float get_varrier_pitch(int);
float get_varrier_angle(int);
float get_varrier_thick(int);
float get_varrier_shift(int);
float get_varrier_cycle(int);

/*---------------------------------------------------------------------------*/

void send_set_background(const float[3], const float[3]);
void recv_set_background(void);

void draw_tile_background(int, int);
void draw_host_background(void);

void set_texture_coordinates(void);

/*---------------------------------------------------------------------------*/

#endif
