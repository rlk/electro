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

#ifndef NODE_H
#define NODE_H

#include <stdio.h>

#include "frustum.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

struct node
{
    float k;

    int star0;
    int starc;
    int nodeL;
    int nodeR;
};

/*---------------------------------------------------------------------------*/

int node_write_bin(struct node *, FILE *);
int node_parse_bin(struct node *, FILE *);

/*---------------------------------------------------------------------------*/

int  node_sort(struct node *, int, int,
               struct star *, int, int, int);
void node_draw(const struct node *, int, int,
               const struct frustum *, const float[6]);
int  node_pick(const struct node *, int,
               const struct star *, int,
               const float[3], const float[3], float *);

/*---------------------------------------------------------------------------*/

#endif
