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
#include "frustum.h"
#include "display.h"
#include "opengl.h"
#include "node.h"

/*---------------------------------------------------------------------------*/

#define N_SIZ 256    /* Maximum number of stars at one leaf of the BSP tree. */

/*---------------------------------------------------------------------------*/

int node_write_bin(struct node *n, FILE *fp)
{
    struct node t = *n;

    /* Ensure all values are represented in network byte order. */

    t.split    = host_to_net_float(t.split);
    t.bound[0] = host_to_net_float(t.bound[0]);
    t.bound[1] = host_to_net_float(t.bound[1]);
    t.bound[2] = host_to_net_float(t.bound[2]);
    t.bound[3] = host_to_net_float(t.bound[3]);
    t.bound[4] = host_to_net_float(t.bound[4]);
    t.bound[5] = host_to_net_float(t.bound[5]);

    t.star0    = host_to_net_int(t.star0);
    t.starc    = host_to_net_int(t.starc);
    t.nodeL    = host_to_net_int(t.nodeL);
    t.nodeR    = host_to_net_int(t.nodeR);

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

        n->split    = net_to_host_float(n->split);
        n->bound[0] = net_to_host_float(n->bound[0]);
        n->bound[1] = net_to_host_float(n->bound[1]);
        n->bound[2] = net_to_host_float(n->bound[2]);
        n->bound[3] = net_to_host_float(n->bound[3]);
        n->bound[4] = net_to_host_float(n->bound[4]);
        n->bound[5] = net_to_host_float(n->bound[5]);

        n->star0    = net_to_host_int(n->star0);
        n->starc    = net_to_host_int(n->starc);
        n->nodeL    = net_to_host_int(n->nodeL);
        n->nodeR    = net_to_host_int(n->nodeR);

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

        N[n0].split = S[sm].pos[i];
        N[n0].nodeL = n1++;
        N[n0].nodeR = n1++;

        /* Create new nodes, each containing half of the stars. */

        n1 = node_sort(N, N[n0].nodeL, n1, S, s0, sm, (i + 1) % 3);
        n1 = node_sort(N, N[n0].nodeR, n1, S, sm, s1, (i + 1) % 3);

        /* Find the node bound. */

        N[n0].bound[0] = MIN(N[N[n0].nodeL].bound[0], N[N[n0].nodeR].bound[0]);
        N[n0].bound[1] = MIN(N[N[n0].nodeL].bound[1], N[N[n0].nodeR].bound[1]);
        N[n0].bound[2] = MIN(N[N[n0].nodeL].bound[2], N[N[n0].nodeR].bound[2]);
        N[n0].bound[3] = MAX(N[N[n0].nodeL].bound[3], N[N[n0].nodeR].bound[3]);
        N[n0].bound[4] = MAX(N[N[n0].nodeL].bound[4], N[N[n0].nodeR].bound[4]);
        N[n0].bound[5] = MAX(N[N[n0].nodeL].bound[5], N[N[n0].nodeR].bound[5]);
    }
    else
    {
        int s;

        /* Find the node bound. */

        N[n0].bound[0] = N[n0].bound[3] = S[s0].pos[0];
        N[n0].bound[1] = N[n0].bound[4] = S[s0].pos[1];
        N[n0].bound[2] = N[n0].bound[5] = S[s0].pos[2];

        for (s = s0; s < s1; s++)
        {
            N[n0].bound[0] = MIN(N[n0].bound[0], S[s].pos[0]);
            N[n0].bound[1] = MIN(N[n0].bound[1], S[s].pos[1]);
            N[n0].bound[2] = MIN(N[n0].bound[2], S[s].pos[2]);
            N[n0].bound[3] = MAX(N[n0].bound[3], S[s].pos[0]);
            N[n0].bound[4] = MAX(N[n0].bound[4], S[s].pos[1]);
            N[n0].bound[5] = MAX(N[n0].bound[5], S[s].pos[2]);
        }
    }
    return n1;
}

/*---------------------------------------------------------------------------*/

int node_draw(const struct node *N, int n, int i, const struct frustum *F)
{
    int r = tst_frustum(F, N[n].bound);

    /* If this node is entirely invisible, prune the tree. */

    if (r < 0) return 0;

    /* If this node is entirely visible, or is a leaf, draw it. */

    if (r > 0 || N[n].nodeL == 0 || N[n].nodeR == 0)
    {
        glDrawArrays(GL_POINTS, N[n].star0, N[n].starc);
        return N[n].starc;
    }
    else
        return (node_draw(N, N[n].nodeL, (i + 1) % 3, F) +
                node_draw(N, N[n].nodeR, (i + 1) % 3, F));
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
        
        if (p[i] < N[n].split || v[i] < 0)
            sL = node_pick(N, N[n].nodeL, S, (i + 1) % 3, p, v, &dL);
        if (p[i] > N[n].split || v[i] > 0)
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

