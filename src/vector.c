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
