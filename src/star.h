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

/*---------------------------------------------------------------------------*/

struct star
{
    unsigned char col[3];
    float         pos[3];
    float         mag;
};

/*---------------------------------------------------------------------------*/

#define STAR_BIN_RECLEN sizeof (struct star)
#define STAR_HIP_RECLEN (451 + 1)
#define STAR_TYC_RECLEN (207 + 1)

/*---------------------------------------------------------------------------*/
/*
int  star_write(const char *);
*/

void   star_color(char, unsigned char[3]);

int    star_write_bin(FILE *, struct star *);
int    star_parse_bin(FILE *, struct star *);
int    star_parse_hip(FILE *, struct star *);
int    star_parse_tyc(FILE *, struct star *);

GLuint star_make_texture(void);
GLuint star_frag_program(void);
GLuint star_vert_program(void);

/*
int  star_read_sol(void);
int  star_read_near_hip(const char *);
int  star_read_near_bin(const char *);
int  star_read_far_tyc(const char *);
int  star_read_far_bin(const char *);

void star_send_create(void);
void star_recv_create(void);

void star_draw(void);

void star_delete(void);
*/
/*---------------------------------------------------------------------------*/

#endif
