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

#ifndef IMAGE_H
#define IMAGE_H

#include "opengl.h"

/*---------------------------------------------------------------------------*/

struct image
{
    int    state;
    GLuint texture;
    char  *filename;
    void  *p;
    int    w;
    int    h;
    int    b;
};

/*---------------------------------------------------------------------------*/

GLuint make_texture(const void *, int,   int,   int);
void  *load_image  (const char *, int *, int *, int *);

/*---------------------------------------------------------------------------*/

int  init_image(void);
void draw_image(int);

void init_image_gl(int);
void free_image_gl(int);

int  send_create_image(const char *);
void recv_create_image(void);

void delete_image(int);

/*---------------------------------------------------------------------------*/

void get_image_p(int, int, int, unsigned char[4]);
int  get_image_w(int);
int  get_image_h(int);

/*---------------------------------------------------------------------------*/

#endif
