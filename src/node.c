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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "utility.h"
#include "opengl.h"
#include "star.h"
#include "node.h"

/*---------------------------------------------------------------------------*/

#define N_SIZ 1024   /* Maximum number of stars at one leaf of the BSP tree. */

/*---------------------------------------------------------------------------*/

int node_write_bin(struct node *n, FILE *fp)
{
    struct node t = *n;

    /* Ensure all values are represented in network byte order. */

    t.k     = htonf(t.k);
    t.star0 = htonl(t.star0);
    t.starc = htonl(t.starc);
    t.nodeL = htonl(t.nodeL);
    t.nodeR = htonl(t.nodeR);

    /* Write the record to the given file. */

    if (fwrite(&t, sizeof (struct node), 1, fp) > 0)
        return 1;
    else
        return 0;
}

int node_parse_bin(struct node *n, FILE *fp)
{
    /* Read a single node record from the given file. */

    if (fread(n, sizeof (struct node), 1, fp) > 0)
    {
        /* Ensure all values are represented in host byte order. */

        n->k     = ntohf(n->k);
        n->star0 = ntohl(n->star0);
        n->starc = ntohl(n->starc);
        n->nodeL = ntohl(n->nodeL);
        n->nodeR = ntohl(n->nodeR);

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

int node_sort(struct node *N, int n0, int n1,
              struct star *S, int s0, int s1, int i)
{
    /* This node contains stars s0 through s1. */

    N[n0].starc = s1 - s0;
    N[n0].star0 = s0;

    /* If this node has too many stars, subdivide it. */

    if (s1 - s0 > N_SIZ)
    {
        int sm = (s1 + s0) / 2;

        /* Sort these stars along the i-axis. */

        qsort(S + s0, s1 - s0, sizeof (struct star), star_cmp[i]);

        /* Create a BSP split at the i-position of the middle star. */

        N[n0].k     = S[sm].pos[i];
        N[n0].nodeL = n1++;
        N[n0].nodeR = n1++;

        /* Create new nodes, each containing half of the stars. */

        n1 = node_sort(N, N[n0].nodeL, n1, S, s0, sm, (i + 1) % 3);
        n1 = node_sort(N, N[n0].nodeR, n1, S, sm, s1, (i + 1) % 3);
    }
    return n1;
}

/*---------------------------------------------------------------------------*/

static int node_test(const float V[4], const float b[6])
{
    /* Test all 8 points of the bounding box against the view frustum. */

    if (b[0] * V[0] + b[1] * V[1] + b[2] * V[2] > V[3]) return 0;
    if (b[0] * V[0] + b[1] * V[1] + b[5] * V[2] > V[3]) return 0;
    if (b[0] * V[0] + b[4] * V[1] + b[2] * V[2] > V[3]) return 0;
    if (b[0] * V[0] + b[4] * V[1] + b[5] * V[2] > V[3]) return 0;
    if (b[3] * V[0] + b[1] * V[1] + b[2] * V[2] > V[3]) return 0;
    if (b[3] * V[0] + b[1] * V[1] + b[5] * V[2] > V[3]) return 0;
    if (b[3] * V[0] + b[4] * V[1] + b[2] * V[2] > V[3]) return 0;
    if (b[3] * V[0] + b[4] * V[1] + b[5] * V[2] > V[3]) return 0;

    /* All points are outsite the frustum.  Cull the box. */

    return 1;
}

int node_draw(const struct node *N, int n, int i,
              const float V[16], const float b[6])
{
    int c = 0;

    /* Test this node against the view frustem. */

    if (node_test(V +  0, b)) return 0;
    if (node_test(V +  4, b)) return 0;
    if (node_test(V +  8, b)) return 0;
    if (node_test(V + 12, b)) return 0;

    if (N[n].nodeL && N[n].nodeR)
    {
        float bR[6];
        float bL[6];

        /* We're at a branch.  Cut the bounding box in two. */

        bL[0] = bR[0] = b[0];
        bL[1] = bR[1] = b[1];
        bL[2] = bR[2] = b[2];
        bL[3] = bR[3] = b[3];
        bL[4] = bR[4] = b[4];
        bL[5] = bR[5] = b[5];

        bL[i] = bR[i + 3] = N[n].k;

        /* Render each subtree in its own bounding box. */

        c += node_draw(N, N[n].nodeL, (i + 1) % 3, V, bL);
        c += node_draw(N, N[n].nodeR, (i + 1) % 3, V, bR);
    }
    else
    {
        /* We've hit a leaf.  Apparently, it is visible.  Draw it. */

        glDrawArrays(GL_POINTS, N[n].star0, N[n].starc);
        c = N[n].starc;
    }

    return c;
}

/*---------------------------------------------------------------------------*/

