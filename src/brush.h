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

#ifndef BRUSH_H
#define BRUSH_H

/*---------------------------------------------------------------------------*/

/* Default material properties */

#define BRUSH_DIFFUSE_R  0.8f
#define BRUSH_DIFFUSE_G  0.8f
#define BRUSH_DIFFUSE_B  0.8f
#define BRUSH_DIFFUSE_A  1.0f

#define BRUSH_SPECULAR_R 0.0f
#define BRUSH_SPECULAR_G 0.0f
#define BRUSH_SPECULAR_B 0.0f
#define BRUSH_SPECULAR_A 1.0f

#define BRUSH_AMBIENT_R  0.2f
#define BRUSH_AMBIENT_G  0.2f
#define BRUSH_AMBIENT_B  0.2f
#define BRUSH_AMBIENT_A  1.0f

#define BRUSH_SHININESS  0.0f

/*---------------------------------------------------------------------------*/

/* Brush flags */

#define BRUSH_DIFFUSE     0x0001
#define BRUSH_SPECULAR    0x0002
#define BRUSH_AMBIENT     0x0004
#define BRUSH_SHINY       0x0008
#define BRUSH_TRANSPARENT 0x0010
#define BRUSH_UNLIT       0x0020

/*---------------------------------------------------------------------------*/

int startup_brush(void);

/*---------------------------------------------------------------------------*/

int  send_create_brush(const char *, const char *);
void recv_create_brush(void);

void send_set_brush_flags(int, int, int);
void recv_set_brush_flags(void);

void send_set_brush_image(int, int);
void recv_set_brush_image(void);

void send_set_brush_frag_prog(int, const char *);
void recv_set_brush_frag_prog(void);

void send_set_brush_vert_prog(int, const char *);
void recv_set_brush_vert_prog(void);

void send_set_brush_color(int, const float[4],
                               const float[4],
                               const float[4],
                               const float[1], int);
void recv_set_brush_color(void);

/*---------------------------------------------------------------------------*/

void init_brush(int);
void fini_brush(int);
int  draw_brush(int, float);
void dupe_brush(int);
void free_brush(int);

void init_brushes(void);
void fini_brushes(void);

/*---------------------------------------------------------------------------*/

int  get_brush_w(int);
int  get_brush_h(int);

/*---------------------------------------------------------------------------*/

#endif
