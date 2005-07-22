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

#ifndef VECTOR_H
#define VECTOR_H

/*---------------------------------------------------------------------------*/

struct vector
{
    int   num;
    int   max;
    int   siz;
    void *buf;
};

typedef struct vector *vector_t;

/*---------------------------------------------------------------------------*/

vector_t vecnew(int, int);
void     vecdel(vector_t);
void     vecclr(vector_t);
void     vecpop(vector_t);
int      vecadd(vector_t);
void    *vecget(vector_t, int);

#define  vecnum(V) ((V)->num)
#define  vecsiz(V) ((V)->siz)
#define  vecbuf(V) ((V)->buf)

#define  veccmp(V, i, j) memcmp(vecget((V), (i)), vecget((V), (j)), (V)->siz)

/*---------------------------------------------------------------------------*/

#endif
