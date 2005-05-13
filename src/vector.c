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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

/*---------------------------------------------------------------------------*/

struct vector
{
    int   num;
    int   max;
    int   siz;
    void *buf;
};

/*---------------------------------------------------------------------------*/

vector_t vecnew(int max, int siz)
{
    vector_t V;

    assert(max > 0);
    assert(siz > 0);

    if ((V = (vector_t) malloc(sizeof (struct vector))))
    {
        if ((V->buf = malloc(max * siz)))
        {
            V->num =   0;
            V->max = max;
            V->siz = siz;

            return V;
        }
        free(V);
    }
    return NULL;
}

void vecdel(vector_t V)
{
    assert(V);

    free(V->buf);
    free(V);
}

/*---------------------------------------------------------------------------*/

int vecnum(vector_t V)
{
    assert(V);

    return V->num;
}

int vecadd(vector_t V)
{
    void *buf;

    assert(V);

    if (V->num == V->max)
    {
        if ((buf = realloc(V->buf, 2 * V->max * V->siz)))
        {
            V->buf  = buf;
            V->max *=   2;
        }
        else return -1;
    }
    return V->num++;
}

void *vecget(vector_t V, int i)
{
    assert(V);
    assert(0 <= i && i < V->max);

    return ((char *) V->buf) + (V->siz * i);
}

/*---------------------------------------------------------------------------*/
