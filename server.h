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

#ifndef SERVER_H
#define SERVER_H

/*---------------------------------------------------------------------------*/

void server_send_draw(void);
void server_send_move(void);
void server_send_turn(void);
void server_send_zoom(void);
void server_send_dist(void);
void server_send_magn(void);

void server(int, int, char **);

/*---------------------------------------------------------------------------*/

#endif
