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

#ifndef STAR_H
#define STAR_H

#include <stdio.h>

#include "opengl.h"

/*---------------------------------------------------------------------------*/

struct star
{
    unsigned char col[4];
    float         pos[3];
    float         mag;
};

/*---------------------------------------------------------------------------*/

#define STAR_BIN_RECLEN sizeof (struct star)
#define STAR_HIP_RECLEN 451
#define STAR_TYC_RECLEN 207

/*---------------------------------------------------------------------------*/

extern int (*star_cmp[3])(const void *, const void *);

int    star_write_bin(struct star *, FILE *);
int    star_parse_bin(struct star *, FILE *);
int    star_parse_hip(struct star *, FILE *);
int    star_parse_tyc(struct star *, FILE *);
int    star_gimme_sol(struct star *);

GLuint star_make_texture(void);
GLuint star_frag_program(void);
GLuint star_vert_program(void);

float  star_pick(const struct star *, const float[3], const float[3]);

/*---------------------------------------------------------------------------*/

#endif
