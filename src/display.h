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
#define MAXTILE  8

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

#define HOST_FULL        1
#define HOST_STEREO      2
#define HOST_FRAMED      4

#define TILE_FLIP_X      1
#define TILE_FLIP_Y      2
#define TILE_TEST_COLOR  4
#define TILE_TEST_GHOST  8
#define TILE_LOCAL_ROT  16
#define TILE_LEFT_EYE   32
#define TILE_RIGHT_EYE  64

struct tile
{
    unsigned int flags;

    float o[3];                 /* World-space tile origin                   */
    float r[3];                 /* World-space tile right vector             */
    float u[3];                 /* World-space tile up vector                */

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

    float quality[2];
};

struct host
{
    unsigned int flags;

    char         name[MAXNAME];
    unsigned int tile[MAXTILE];
    unsigned int n;

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

/*---------------------------------------------------------------------------*/

unsigned int send_add_host(const char *, int, int, int, int);
void         recv_add_host(void);

unsigned int send_add_tile(unsigned int, int, int, int, int);
void         recv_add_tile(void);

/*---------------------------------------------------------------------------*/

void send_set_host_flags   (unsigned int, unsigned int, unsigned int);
void send_set_tile_flags   (unsigned int, unsigned int, unsigned int);

void send_set_tile_quality (unsigned int, const float[2]);
void send_set_tile_viewport(unsigned int, int, int, int, int);
void send_set_tile_position(unsigned int, const float[3],
                                          const float[3],
                                          const float[3]);
void send_set_tile_linescrn(unsigned int, float, float,
                                   float, float, float);

void recv_set_host_flags   (void);
void recv_set_tile_flags   (void);
void recv_set_tile_quality (void);
void recv_set_tile_viewport(void);
void recv_set_tile_position(void);
void recv_set_tile_linescrn(void);

/*---------------------------------------------------------------------------*/

void send_set_background(const float[3], const float[3]);
void recv_set_background(void);

/*---------------------------------------------------------------------------*/

void  set_window_pos(int, int);
void  set_window_siz(int);

void  set_window_full(int);
void  set_window_w(int);
void  set_window_h(int);
int   get_window_w(void);
int   get_window_h(void);
int   get_window_full(void);
int   get_window_framed(void);
int   get_window_stereo(void);

/*---------------------------------------------------------------------------*/

void  get_display_point(float[3], const float[3], int, int);
void  get_display_union(float[4]);
void  get_display_bound(float[6]);

void  get_tile_o(unsigned int, float[3]);
void  get_tile_r(unsigned int, float[3]);
void  get_tile_u(unsigned int, float[3]);
void  get_tile_n(unsigned int, float[3]);

void  get_tile_quality(unsigned int, float[2]);

float get_varrier_pitch(unsigned int);
float get_varrier_angle(unsigned int);
float get_varrier_thick(unsigned int);
float get_varrier_shift(unsigned int);
float get_varrier_cycle(unsigned int);

unsigned int get_tile_count(void);
unsigned int get_tile_flags(unsigned int);

/*---------------------------------------------------------------------------*/

int  draw_ortho(unsigned int, float, float);
int  draw_persp(unsigned int, float, float, int, const float[3]);

/*---------------------------------------------------------------------------*/

void draw_tile_background(unsigned int);
void draw_host_background(void);

void set_texture_coordinates(void);

/*---------------------------------------------------------------------------*/

#endif
