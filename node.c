/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    TOTAL PERSPECTIVE VORTEX is free software;  you can redistribute it    */
/*    and/or modify it under the terms of the  GNU General Public License    */
/*    as published by the  Free Software Foundation;  either version 2 of    */
/*    the License, or (at your option) any later version.                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#include <stdlib.h>

#include "opengl.h"
#include "galaxy.h"
#include "node.h"

/*---------------------------------------------------------------------------*/

static struct node *N;

/*---------------------------------------------------------------------------*/

static int node_init_count(int d)
{
    if (d == 0)
        return 8;
    else
        return 8 * node_init_count(d - 1);
}

static void node_init_pos(int i, int d, float x, float y, float z, float r)
{
    float s = r / 2;

    N[i].position[0] = x;
    N[i].position[1] = y;
    N[i].position[2] = z;
    N[i].radius      = r;

    if (d > 0)
    {
        node_init_pos(8 * i + 1, d - 1, x + s, y + s, z + s, s);
        node_init_pos(8 * i + 2, d - 1, x - s, y + s, z + s, s);
        node_init_pos(8 * i + 3, d - 1, x + s, y - s, z + s, s);
        node_init_pos(8 * i + 4, d - 1, x - s, y - s, z + s, s);
        node_init_pos(8 * i + 5, d - 1, x + s, y + s, z - s, s);
        node_init_pos(8 * i + 6, d - 1, x - s, y + s, z - s, s);
        node_init_pos(8 * i + 7, d - 1, x + s, y - s, z - s, s);
        node_init_pos(8 * i + 8, d - 1, x - s, y - s, z - s, s);
    }
}

int node_init(int d)
{
    int n = node_init_count(d);

    if ((N = (struct node *) calloc(n, sizeof (struct node))))
    {
        node_init_pos(0, d, 0, 0, 0, RADIUS);
        return 1;
    }
    return 0;
}

void node_draw(int i, int d)
{
    if (d == 0)
    {
        glVertex3fv(N[i].position);
    }
    else
    {
        node_draw(8 * i + 1, d - 1);
        node_draw(8 * i + 2, d - 1);
        node_draw(8 * i + 3, d - 1);
        node_draw(8 * i + 4, d - 1);
        node_draw(8 * i + 5, d - 1);
        node_draw(8 * i + 6, d - 1);
        node_draw(8 * i + 7, d - 1);
        node_draw(8 * i + 8, d - 1);
    }
}

/*---------------------------------------------------------------------------*/
