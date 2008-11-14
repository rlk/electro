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

#include <stdlib.h>
#include "vec.h"

/* This module implements a simple vector data type suitible for use as a    */
/* descriptor-indexed record container.  The vector is implemented using a   */
/* heap-allocated array and may be indexed and deleted normally.  Free space */
/* within the vector is dynamically managed.  Vector overhead is 1 record,   */
/* the first.  As the first record is structural, applications may use 0 to  */
/* indicate an invalid index.                                                */

/* Vector operations take a pointer v to the beginning of the heap-allocated */
/* buffer and the size s of each record.  Size must be at least 32 bits.     */

/*---------------------------------------------------------------------------*/

struct vec
{
    unsigned int next : 31;
    unsigned int last :  1;
};

#define NEXT(v, s, i) ((struct vec *) ((char *) (v) + (s) * (i)))->next
#define LAST(v, s, i) ((struct vec *) ((char *) (v) + (s) * (i)))->last

/*---------------------------------------------------------------------------*/
/* vecnew(n, s) allocates and initializes a new vector containing n records  */
/* of size s.  It returns the pointer the heap-allocated buffer, or NULL on  */
/* failure.                                                                  */

void *vec_new(size_t n, size_t s)
{
    void        *v;
    unsigned int i;

    /* Allocate an empty vector and initialize the free list. */

    if ((v = calloc(n * s, 1)))
    {
        for (i = 0; i < n - 1; ++i)
        {
            NEXT(v, s, i) = i + 1;
            LAST(v, s, i) = 0;
        }

        NEXT(v, s, i) = i + 1;
        LAST(v, s, i) = 1;

        return v;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/
/* vecgro(v, s) doubles the size of vector (v, s).  It returns a pointer to  */
/* the heap-allocated buffer, which may have changed in order to accomodate  */
/* the new size.  It returns NULL on failure, leaving the original intact.   */

void *vec_gro(void *v, size_t s)
{
    unsigned int i;
    unsigned int j;
    unsigned int n;
    void        *w;

    /* Determine the total length and last free record of vector (v, s). */

    for (j = 0; LAST(v, s, j) == 0; j = NEXT(v, s, j))
        ;

    n = NEXT(v, s, j);
        
    /* Reallocate the vector at twice the size and extend the free list. */

    if ((w = realloc(v, 2 * n * s)))
    {
        for (i = n; i < 2 * n - 1; ++i)
        {
            NEXT(w, s, i) = i + 1;
            LAST(w, s, i) = 0;
        }

        NEXT(w, s, i) = i + 1;
        LAST(w, s, i) = 1;
        LAST(w, s, j) = 0;

        return w;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/
/* vecadd(v, s) finds the first available record in vector (v, s) and marks  */
/* it as allocated.  It returns a valid index, or 0 if no records remain.    */
/* The new record is NOT initialized.                                        */

unsigned int vec_add(void *v, size_t s)
{
    /* Return the first record on the free list. */

    if (LAST(v, s, 0) == 0)
    {
        unsigned int i = NEXT(v, s, 0);

        NEXT(v, s, 0) = NEXT(v, s, i);
        LAST(v, s, 0) = LAST(v, s, i);

        return i;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* vecdel(v, s, i) marks record i as available in vector (v, s).  If record  */
/* i is already available then vecdel does nothing, but is still O(n).       */

void vec_del(void *v, size_t s, unsigned int k)
{
    unsigned int i;

    /* Insert record K into the free list. */

    for (i = 0; i < k; i = NEXT(v, s, i))
        if (k < NEXT(v, s, i))
        {
            NEXT(v, s, k) = NEXT(v, s, i);
            LAST(v, s, k) = LAST(v, s, i);

            NEXT(v, s, i) = k;
            LAST(v, s, i) = 0;
        }
}

/*---------------------------------------------------------------------------*/
/* vecchk(v, s, i) returns false if record i is on the free list, thus it    */
/* returns true if record i is allocated.  This operation is O(n).           */

unsigned int vec_chk(void *v, size_t s, unsigned int i)
{
    unsigned int j;

    for (j = 0; !LAST(v, s, j); j = NEXT(v, s, j))
        if (i == j)
            return 0;

    return 1;
}

/*---------------------------------------------------------------------------*/
/* vecall(v, s, i, j) iterates over all allocated elements of vector (v, s). */
/* The first call should be made with i=0 and j=0.  Each call will increment */
/* i, skipping over unused records.  j will indicate the last unused record. */
/* vecall returns 1 while i is a valid index, and 0 when the end is reached. */

unsigned int vec_all(void *v, size_t s, unsigned int *i, unsigned int *j)
{
    *i = *i + 1;

    /* Continue in an unfinished block.  Stop on the last block. */

    if (NEXT(v, s, *j) > *i) return 1;
    if (LAST(v, s, *j) == 1) return 0;

    /* Skip over free blocks to the next allocated block. */

    do
    {
        *i = NEXT(v, s, *j) + 1;
        *j = NEXT(v, s, *j);
    }
    while (NEXT(v, s, *j) == *i &&
           LAST(v, s, *j) ==  0);

    /* Indicate whether anything remains to be iterated. */

    return (NEXT(v, s, *j) > *i);
}

/*---------------------------------------------------------------------------*/
