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

#ifndef STRING_H
#define STRING_H

#include "entity.h"

/*---------------------------------------------------------------------------*/

struct entity_func *startup_string(void);

/*---------------------------------------------------------------------------*/

int  send_create_string(const char *);
void recv_create_string(void);

void send_set_string_color(int, float, float, float);
void recv_set_string_color(void);

void send_set_string_value(int, const char *);
void recv_set_string_value(void);

/*---------------------------------------------------------------------------*/

#endif
