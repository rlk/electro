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

/*---------------------------------------------------------------------------*/

#define STAR_HIP_RECLEN 451
#define STAR_TYC_RECLEN 207
#define STAR_BIN_RECLEN sizeof (struct star)

/*---------------------------------------------------------------------------*/

struct star
{
    GLubyte col[3];
    GLfloat pos[3];
    GLfloat mag;
};

/*---------------------------------------------------------------------------*/

int star_write(const char *);

int star_read_hip(const char *);
int star_read_typ(const char *);
int star_read_bin(const char *);

void star_send_create(void);
void star_recv_create(void);

void star_draw(void);

void star_delete(void);

/*---------------------------------------------------------------------------*/

#endif
