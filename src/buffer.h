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

#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>

#include "vector.h"

/*---------------------------------------------------------------------------*/

int startup_buffer(void);

void sync_buffer(void);

/*---------------------------------------------------------------------------*/

void  send_array(const void *, size_t, size_t);
void  send_index(int);
void  send_event(char);
void  send_float(float);

/*---------------------------------------------------------------------------*/

void  recv_array(void *, size_t, size_t);
int   recv_index(void);
char  recv_event(void);
float recv_float(void);

/*---------------------------------------------------------------------------*/

void     send_vector(vector_t);
vector_t recv_vector(void);

/*---------------------------------------------------------------------------*/

#endif
