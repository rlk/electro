/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    TOTAL PERSPECTIVE VORTEX is free software;  you can redistribute it    */
/*    and/or modify it under the terms of the  GNU General Public License    */
/*    as published by the  Free Software Foundation;  either version 2 of    */
/*    the License, or (at your option) any later version.                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#ifndef STAR_H
#define STAR_H

/*---------------------------------------------------------------------------*/

#define STAR_TXT_RECLEN 451
#define STAR_BIN_RECLEN sizeof (struct star)

/*---------------------------------------------------------------------------*/

struct star
{
    GLubyte col[3];
    GLfloat pos[3];
    GLfloat mag;
};

/*---------------------------------------------------------------------------*/

int star_write_catalog(const char *);

int star_read_catalog_txt(const char *);
int star_read_catalog_bin(const char *);

void star_init(int);
void star_draw(void);

/*---------------------------------------------------------------------------*/

#endif
