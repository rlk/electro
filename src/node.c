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
#include <float.h>

#include "utility.h"
#include "opengl.h"
#include "node.h"

/*---------------------------------------------------------------------------*/

#define N_SIZ 256    /* Maximum number of stars at one leaf of the BSP tree. */

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

void node_draw(const struct node *N, int n, int i,
               const struct frustum *F, const float b[6])
{
    int r = test_frustum(F, b);

    /* If this node is entirely invisible, prune the tree. */

    if (r < 0) return;

    /* If this is a leaf, or is entirely visible, draw it. */

    if (N[n].nodeL == 0 || N[n].nodeR == 0 || r > 0)
        glDrawArrays(GL_POINTS, N[n].star0, N[n].starc);

    else
    {
        /* We're at necessary a branch.  Cut the bounding box in two. */

        float bR[6];
        float bL[6];

        memcpy(bR, b, 6 * sizeof (float));
        memcpy(bL, b, 6 * sizeof (float));

        bL[i + 3] = bR[i] = N[n].k;

        /* Render each subtree in its own bounding box. */

        node_draw(N, N[n].nodeL, (i + 1) % 3, V, bL);
        node_draw(N, N[n].nodeR, (i + 1) % 3, V, bR);
    }
}

int node_pick(const struct node *N, int n,
              const struct star *S, int i,
              const float p[3], const float v[3], float *d)
{
    int s = -1;

    if (N[n].nodeL && N[n].nodeR)
    {
        float dL = -1.0f;
        float dR = -1.0f;
        int   sL = -1;
        int   sR = -1;

        /* Test the left and right child nodes, as necessary. */
        
        if (p[i] < N[n].k || v[i] < 0)
            sL = node_pick(N, N[n].nodeL, S, (i + 1) % 3, p, v, &dL);
        if (p[i] > N[n].k || v[i] > 0)
            sR = node_pick(N, N[n].nodeR, S, (i + 1) % 3, p, v, &dR);

        /* Note the best find thus far. */

       *d = (dL > dR) ? dL : dR;
        s = (dL > dR) ? sL : sR;
    }
    else
    {
        int   si;
        float di;

        *d = -1.0f;

        /* Find the nearest star at this node. */

        for (si = 0; si < N[n].starc; ++si)
            if ((di = star_pick(S + si + N[n].star0, p, v)) > *d)
            {
               *d = di;
                s = si + N[n].star0;
            }
    }

    return s;
}

/*---------------------------------------------------------------------------*/

