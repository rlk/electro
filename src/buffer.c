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

#define BUCKETSZ (1 * 1024 * 1024)

struct bucket
{
    struct bucket *next;
    size_t         size;
};

static struct bucket *first = NULL;
static struct bucket *curr  = NULL;

static size_t count = 0;

/*---------------------------------------------------------------------------*/

int startup_buffer(void)
{
    /* All hosts begin with one empty bucket. */

    if ((first = curr = (struct bucket *) malloc(BUCKETSZ)))
    {
        count = sizeof (struct bucket);

        curr->next = NULL;
        curr->size = count;

        return 1;
    }
    return 0;
}

void sync_buffer(void)
{
    struct bucket *temp;
    int rank;
    int len;

    assert_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

    /* Iterate over all data-containing buckets. */

    for (curr = first; curr->size == BUCKETSZ; curr = curr->next)
    {
        /* Broadcast this bucket, preserving client 'next' links. */

        len  = curr->size;
        temp = curr->next;

        assert_mpi(MPI_Bcast(&len, 1,   MPI_INTEGER, 0, MPI_COMM_WORLD));
        assert_mpi(MPI_Bcast(curr, len, MPI_BYTE,    0, MPI_COMM_WORLD));

        curr->next = temp;

        /* If a client is out of buckets but more are coming, allocate. */

        if (rank && len == BUCKETSZ && curr->next == NULL)
        {
            if ((curr->next = (struct bucket *) malloc(BUCKETSZ)))
            {
                curr = curr->next;
                curr->next = NULL;
            }
        }
    }

    /* Rewind all hosts to read or write at the first bucket. */

    curr = first;
}

/*---------------------------------------------------------------------------*/

static void send_data(const void *buf, size_t len)
{
    size_t fit = BUCKETSZ - curr->size;

    /* Check if all of the data fit in the current bucket. */

    if (len < fit)
    {
        /* Append the data to the current bucket. */

        memcpy((char *) curr + curr->size, buf, len);
        curr->size += len;
    }
    else
    {
        /* Append as many data as will fit in the current bucket. */

        memcpy((char *) curr + curr->size, buf, fit);
        curr->size += fit;

        /* Ensure there is another bucket and handle the remaining data. */
        
        if (curr->next || (curr->next = (struct bucket *) malloc(BUCKETSZ)))
        {
            curr = curr->next;
            curr->next = NULL;
            curr->size = sizeof (struct bucket);

            send_data((char *) buf + fit, len - fit);
        }
    }
}

static void recv_data(void *buf, size_t len)
{
    size_t fit = curr->size - count;

    /* Check if the current bucket holds all of the requested data. */

    if (len < fit)
    {
        /* Copy the requested data from the current bucket. */

        memcpy(buf, (char *) curr + curr->size, len);
        count += len;
    }
    else
    {
        /* Copy as many of the requested data as the current bucket holds. */

        memcpy(buf, (char *) curr + curr->size, fit);
        count += fit;

        /* Move to the next bucket and handle the remainder of the request. */

        if (curr->next)
        {
            curr  = curr->next;
            count = sizeof (struct bucket);

            recv_data((char *) buf + fit, len - fit);
        }
    }
}

#ifdef SNIP

/*---------------------------------------------------------------------------*/
/* FIXME: Byte-order dependant.                                              */
/* FIXME: Not protected against buffer overflow.                             */

/*
#define BUFINIT (64 * 1024 * 1024)
*/
#define BUFINIT (1 * 1024 * 1024)

static unsigned char *buf;
static int            pos;
static int            len;
static int            max;

union typecast
{
    int   i;
    char  c;
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
    int num = V->num;
    int siz = V->siz;

    assert(pos + num * siz < max);

    pack_index(num);
    pack_index(siz);

    memcpy(buf + pos, vecbuf(V), num * siz);
    pos += (num * siz);
}

vector_t unpack_vector(void)
{
    vector_t V;

    int num = unpack_index();
    int siz = unpack_index();

    if ((V = vecnew(num, siz)))
    {
         memcpy(vecbuf(V), buf + pos, num * siz);
         V->num = num;

         pos += (num * siz);
    }

    return V;
}

/*---------------------------------------------------------------------------*/

void pack_index(int i)
{
    union typecast *T = (union typecast *) (buf + pos);

    assert(pos + sizeof (int) < max);

    T->i = i;
    pos += sizeof (int);
}

void pack_event(char c)
{
    union typecast *T = (union typecast *) (buf + pos);

    assert(pos + sizeof (unsigned char) < max);

    T->c = c;
    pos += sizeof (unsigned char);
}

void pack_float(float f)
{
    union typecast *T = (union typecast *) (buf + pos);

    assert(pos + sizeof (float) < max);

    T->f = f;
    pos += sizeof (float);
}

void pack_alloc(int siz, const void *ptr)
{
    assert(pos + siz < max);

    memcpy(buf + pos, ptr, siz);
    pos += siz;
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

#endif
