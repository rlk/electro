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
    GLuint texture;
    char *filename;
    void *p;
    int   w;
    int   h;
    int   b;
};

/*---------------------------------------------------------------------------*/

GLuint image_make_tex(const void *, int,   int,   int);
void  *image_load_png(const char *, int *, int *, int *);

/*---------------------------------------------------------------------------*/

int  image_init(void);
void image_draw(int);

int  image_send_create(const char *);
void image_recv_create(void);

int  image_get_w(int);
int  image_get_h(int);

void image_delete(int);

/*---------------------------------------------------------------------------*/

#endif
