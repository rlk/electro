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

#include "opengl.h"
#include "star.h"
#include "node.h"

/*---------------------------------------------------------------------------*/

#define N_MAX    4096
/*
#define N_SIZ    1024
*/
#define N_SIZ     256
#define S_MAX 2621440

static struct node *N;
static int          N_num;
static int          N_max;

static struct star *S;
static int          S_num;
static int          S_max;

/*---------------------------------------------------------------------------*/

static int star_cmp0(const void *p1,
                     const void *p2)
{
    return (((const struct star *) p1)->pos[0] <
            ((const struct star *) p2)->pos[0]);
}

static int star_cmp1(const void *p1,
                     const void *p2)
{
    return (((const struct star *) p1)->pos[1] <
            ((const struct star *) p2)->pos[1]);
}

static int star_cmp2(const void *p1,
                     const void *p2)
{
    return (((const struct star *) p1)->pos[2] <
            ((const struct star *) p2)->pos[2]);
}

static int (*star_cmp[3])(const void *, const void *) = {
    star_cmp0,
    star_cmp1,
    star_cmp2
};

static void prep_node(int n, int i, int a, int z)
{
    N[n].star0 = a;
    N[n].starc = z - a;

    if (z - a > N_SIZ && N_num < N_max)
    {
        int m = (a + z) / 2;

        qsort(S + a, z - a, sizeof (struct star), star_cmp[i]);

        N[n].k     = S[m].pos[i];
        N[n].nodeL = N_num++;
        N[n].nodeR = N_num++;

        prep_node(N[n].nodeL, (i + 1) % 3, a, m);
        prep_node(N[n].nodeR, (i + 1) % 3, m, z);
    }
}

/*---------------------------------------------------------------------------*/

void prep_init(void)
{
    if ((N = (struct node *) calloc(N_MAX, sizeof (struct node))))
    {
        N_max = N_MAX;
        N_num = 0;
    }
    if ((S = (struct star *) calloc(S_MAX, sizeof (struct star))))
    {
        S_max = S_MAX;
        S_num = 0;
    }
}

void prep_sort(float bound[6])
{
    int i;

    N_num = 1;

    prep_node(0, 0, 0, S_num);

    bound[0] = bound[1] = bound[2] =  1000000.0f;
    bound[3] = bound[4] = bound[5] = -1000000.0f;

    for (i = 0; i < S_num; ++i)
    {
        if (bound[0] > S[i].pos[0]) bound[0] = S[i].pos[0];
        if (bound[1] > S[i].pos[1]) bound[1] = S[i].pos[1];
        if (bound[2] > S[i].pos[2]) bound[2] = S[i].pos[2];
        if (bound[3] < S[i].pos[0]) bound[3] = S[i].pos[0];
        if (bound[4] < S[i].pos[1]) bound[4] = S[i].pos[1];
        if (bound[5] < S[i].pos[2]) bound[5] = S[i].pos[2];
    }
}

/*---------------------------------------------------------------------------*/

void prep_file_hip(const char *filename)
{
    struct stat buf;
    FILE *fp;

    if (stat(filename, &buf) == 0)
    {
        int n = (int) buf.st_size / STAR_HIP_RECLEN;
        int i;

        if ((fp = fopen(filename, "r")))
        {
            for (i = 0; i < n && S_num < S_max; i++)
                S_num += star_parse_hip(fp, S + S_num);

            fclose(fp);
        }
    }
}

/*---------------------------------------------------------------------------*/

static GLuint star_texture;
static GLuint star_frag;
static GLuint star_vert;

void node_init(void)
{
    star_texture = star_make_texture();
    star_frag    = star_frag_program();
    star_vert    = star_vert_program();
}

/*---------------------------------------------------------------------------*/

static int star_count;
static int node_count;

static int node_test(const float V[4], const float b[6])
{
    if (b[0] * V[0] + b[1] * V[1] + b[2] * V[2] < V[3]) return 0;
    if (b[0] * V[0] + b[1] * V[1] + b[5] * V[2] < V[3]) return 0;
    if (b[0] * V[0] + b[4] * V[1] + b[2] * V[2] < V[3]) return 0;
    if (b[0] * V[0] + b[4] * V[1] + b[5] * V[2] < V[3]) return 0;
    if (b[3] * V[0] + b[1] * V[1] + b[2] * V[2] < V[3]) return 0;
    if (b[3] * V[0] + b[1] * V[1] + b[5] * V[2] < V[3]) return 0;
    if (b[3] * V[0] + b[4] * V[1] + b[2] * V[2] < V[3]) return 0;
    if (b[3] * V[0] + b[4] * V[1] + b[5] * V[2] < V[3]) return 0;

    return 1; /* cull */
}

static void node_draw_tree(int n, int i, const float V[16], const float b[6])
{
    node_count += 1;

    if (node_test(V +  0, b)) return;
    if (node_test(V +  4, b)) return;
    if (node_test(V +  8, b)) return;
    if (node_test(V + 12, b)) return;

    if (N[n].nodeL && N[n].nodeR)
    {
        float bR[6];
        float bL[6];

        bL[0] = bR[0] = b[0];
        bL[1] = bR[1] = b[1];
        bL[2] = bR[2] = b[2];
        bL[3] = bR[3] = b[3];
        bL[4] = bR[4] = b[4];
        bL[5] = bR[5] = b[5];

        bL[i] = bR[i + 3] = N[n].k;

        node_draw_tree(N[n].nodeL, (i + 1) % 3, V, bL);
        node_draw_tree(N[n].nodeR, (i + 1) % 3, V, bR);
    }
    else
    {
        star_count += N[n].starc;
        glDrawArrays(GL_POINTS, N[n].star0, N[n].starc);
    }
}

void node_draw(const float V[16], const float b[6])
{
    GLenum  ub = GL_UNSIGNED_BYTE;
    GLenum  fl = GL_FLOAT;
    GLsizei sz = sizeof (struct star);

    star_count = 0;
    node_count = 0;

    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    {
        glEnable(GL_VERTEX_PROGRAM_ARB);
        glEnable(GL_FRAGMENT_PROGRAM_ARB);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableVertexAttribArrayARB(6);

        glBindTexture(GL_TEXTURE_2D, star_texture);

        glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, star_frag);
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB,   star_vert);

        glVertexPointer            (3, fl,    sz, &S[0].pos);
        glColorPointer             (3, ub,    sz, &S[0].col);
        glVertexAttribPointerARB(6, 1, fl, 0, sz, &S[0].mag);

        node_draw_tree(0, 0, V, b);
    }
    glPopClientAttrib();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

