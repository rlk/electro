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

#ifndef GALAXY_H
#define GALAXY_H

/*---------------------------------------------------------------------------*/

#define RADIUS 15000.0f

/*---------------------------------------------------------------------------*/

int  galaxy_send_create(const char *, const char *);
void galaxy_recv_create(void);

void galaxy_send_magn(int, float);
void galaxy_recv_magn(void);

void galaxy_draw(int, int, float[3], float[4][4]);

void galaxy_delete(int);

/*---------------------------------------------------------------------------*/

#endif
