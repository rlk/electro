/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    vec.[ch] is free software; you can redistribute it and/or modify it    */
/*    under the terms of the  GNU General Public License  as published by    */
/*    the  Free Software Foundation;  either version 2 of the License, or    */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#ifndef VEC_H
#define VEC_H

#include <stdlib.h>

/*---------------------------------------------------------------------------*/

void        *vec_new(size_t, size_t);
void        *vec_gro(void *, size_t);
unsigned int vec_add(void *, size_t);
void         vec_del(void *, size_t, unsigned int);
unsigned int vec_chk(void *, size_t, unsigned int);
unsigned int vec_all(void *, size_t, unsigned int *, unsigned int *);

/*---------------------------------------------------------------------------*/

#endif
