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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "vector.h"
#include "buffer.h"
#include "utility.h"

/*---------------------------------------------------------------------------*/
/* FIXME: Byte-order dependant.                                              */
/* FIXME: Not protected against buffer overflow.                             */

#define BUFINIT (64 * 1024 * 1024)

static unsigned char *buf;
static int            pos;
static int            len;
static int            max;

union typecast
{
    int   i;
    char  c[4];
    float f;
};

/*---------------------------------------------------------------------------*/

int startup_buffer(void)
{
    if ((buf = (unsigned char *) calloc(BUFINIT, 1)))
    {
        max = BUFINIT;
        pos = 0;
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void sync_buffer(void)
{
    len = pos;

#ifdef MPI
    assert_mpi(MPI_Bcast(&len,  1, MPI_INTEGER, 0, MPI_COMM_WORLD));
    assert_mpi(MPI_Bcast(buf, len, MPI_BYTE,    0, MPI_COMM_WORLD));
#endif

    pos = 0;
}

/*---------------------------------------------------------------------------*/

void pack_vector(vector_t V)
{
    int num = vecnum(V);
    int siz = vecsiz(V);

    pack_index(num);
    pack_index(siz);

    memcpy(buf + pos, vecget(V, 0), num * siz);
    pos += (num * siz);

    assert(pos < max);
}

vector_t unpack_vector(void)
{
    vector_t V;

    int num = unpack_index();
    int siz = unpack_index();

    if ((V = vecnew(num, siz)))
    {
         memcpy(vecget(V, 0), buf + pos, num * siz);
         pos += (num * siz);
    }

    return V;
}

/*---------------------------------------------------------------------------*/

void pack_index(int i)
{
    union typecast *T = (union typecast *) (buf + pos);

    T->i = i;
    pos += sizeof (int);

    assert(pos < max);
}

void pack_event(char c)
{
    union typecast *T = (union typecast *) (buf + pos);

    T->c[0] = c;
    pos += sizeof (unsigned char);

    assert(pos < max);
}

void pack_float(float f)
{
    union typecast *T = (union typecast *) (buf + pos);

    T->f = f;
    pos += sizeof (float);

    assert(pos < max);
}

void pack_alloc(int siz, const void *ptr)
{
    memcpy(buf + pos, ptr, siz);
    pos += siz;

    assert(pos < max);
}

/*---------------------------------------------------------------------------*/

int unpack_count(void)
{
    return (len - pos);
}

int unpack_index(void)
{
    union typecast *T = (union typecast *) (buf + pos);

    pos += sizeof (int);
    return T->i;
}

char unpack_event(void)
{
    union typecast *T = (union typecast *) (buf + pos);

    pos += sizeof (char);
    return T->c[0];
}

float unpack_float(void)
{
    union typecast *T = (union typecast *) (buf + pos);

    pos += sizeof (float);
    return T->f;
}

void *unpack_alloc(int siz)
{
    void *ptr = NULL;

    if ((ptr = malloc(siz)))
    {
        memcpy(ptr, buf + pos, siz);
        pos += siz;
    }
    return ptr;
}

/*---------------------------------------------------------------------------*/

