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

#include <stdlib.h>
#include <string.h>

#include "shared.h"

/*---------------------------------------------------------------------------*/
/* FIXME: Byte-order dependant.                                              */
/* FIXME: Not protected against buffer overflow.                             */

#define BUFINIT (1024 * 1024)

static unsigned char *buf;
static int            pos;
static int            max;

union typecast
{
    int   i;
    char  c;
    float f;
};

/*---------------------------------------------------------------------------*/

void buffer_init(void)
{
    if ((buf = (unsigned char *) calloc(BUFINIT, 1)))
    {
        max = BUFINIT;
        pos = 0;
    }
}

void buffer_free(void)
{
    free(buf);

    buf = NULL;
    pos = 0;
    max = 0;
}

/*---------------------------------------------------------------------------*/

void buffer_sync(void)
{
    int n;

#ifdef MPI
    mpi_assert(MPI_Bcast(&n,  1, MPI_INTEGER, 0, MPI_COMM_WORLD));
    mpi_assert(MPI_Bcast(buf, n, MPI_BYTE,    0, MPI_COMM_WORLD));
#endif

    pos = 0;
}

/*---------------------------------------------------------------------------*/

void pack_index(int i)
{
    union typecast *T = (union typecast *) (buf + pos);

    T->i = i;
    pos += sizeof (int);
}

void pack_event(char c)
{
    union typecast *T = (union typecast *) (buf + pos);

    T->c = c;
    pos += sizeof (unsigned char);
}

void pack_float(float f)
{
    union typecast *T = (union typecast *) (buf + pos);

    T->f = f;
    pos += sizeof (float);
}

void pack_alloc(const void *ptr, int siz)
{
    memcpy(buf + pos, ptr, siz);
    pos += siz;
}

/*---------------------------------------------------------------------------*/

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
    return T->c;
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

