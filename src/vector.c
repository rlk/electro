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

vector_t vecnew(int max, int siz)
{
    vector_t V;

    assert(siz > 0);

    if ((V = (vector_t) calloc(1, sizeof (struct vector))))
    {
        if ((max == 0) || (V->buf = malloc(max * siz)))
        {
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

void vecpop(vector_t V)
{
    assert(V);
    assert(V->num > 0);

    V->num--;
}

void vecclr(vector_t V)
{
    assert(V);

    V->num = 0;
}

/*---------------------------------------------------------------------------*/

int vecadd(vector_t V)
{
    void *buf;
    int i;

    assert(V);

    if (V->num == V->max)
    {
        int max = (V->max == 0) ? 1 : 2 * V->max;

        if ((buf = realloc(V->buf, V->siz * max)))
        {
            V->max = max;
            V->buf = buf;
        }
        else return -1;
    }

    i = V->num++;

    memset(vecget(V, i), 0, V->siz);

    return i;
}

void *vecget(vector_t V, int i)
{
    assert(V);
    assert(0 <= i && i < V->num);

    return ((char *) (V)->buf) + ((V)->siz * (i));
}

/*---------------------------------------------------------------------------*/
