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
#define N_SIZ    1024
#define S_MAX 2621440

static struct node *N;
static int          N_num;
static int          N_max;

static struct star *S;
static int          S_num;
static int          S_max;

static int N_curr = 0;

/*---------------------------------------------------------------------------*/

void prep_L(void)
{
    if (N[N_curr].nodeL)
        N_curr = N[N_curr].nodeL;
}

void prep_R(void)
{
    if (N[N_curr].nodeR)
        N_curr = N[N_curr].nodeR;
}

void prep_p(void)
{
    if (N_curr)
        N_curr = N[N_curr].nodep;
}

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

static void prep_node(int n, int a, int z, int i)
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

        prep_node(N[n].nodeL, a, m, (i + 1) % 3);
        prep_node(N[n].nodeR, m, z, (i + 1) % 3);

        N[N[n].nodeL].nodep = n;
        N[N[n].nodeR].nodep = n;
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

void prep_sort(void)
{
    N_num = 1;
    prep_node(0, 0, S_num, 0);
}

/*---------------------------------------------------------------------------*/

void prep_file_hip(const char *filename)
{
    struct stat buf;
    FILE *fp;

    if (stat(filename, &buf) == 0)
    {
        int i, n = (int) buf.st_size / STAR_HIP_RECLEN;

        if ((fp = fopen(filename, "r")))
        {
            for (i = 0; i < n && S_num < S_max; i++)
                S_num += star_parse_hip(fp, S + S_num);

            fclose(fp);
        }
    }
}

/*---------------------------------------------------------------------------*/

int node_test(float V[4][4], float p[3])
{
    if (p[0] * V[0][0] + p[1] * V[0][1] + p[2] * V[0][2] > V[0][3])
        return 0;
    if (p[0] * V[1][0] + p[1] * V[1][1] + p[2] * V[1][2] > V[1][3])
        return 0;
    if (p[0] * V[2][0] + p[1] * V[2][1] + p[2] * V[2][2] > V[2][3])
        return 0;
    if (p[0] * V[3][0] + p[1] * V[3][1] + p[2] * V[3][2] > V[3][3])
        return 0;

    return 1;
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

void node_draw(float P[3], float V[4][4])
{
    GLenum  ub = GL_UNSIGNED_BYTE;
    GLenum  fl = GL_FLOAT;
    GLsizei sz = sizeof (struct star);

    int i, n = N_curr;

    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    {
        int c = 0;

        glEnable(GL_VERTEX_PROGRAM_ARB);
        glEnable(GL_FRAGMENT_PROGRAM_ARB);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableVertexAttribArrayARB(6);

        glBindTexture(GL_TEXTURE_2D, star_texture);

        glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, star_frag);
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB,   star_vert);

        glVertexPointer            (3, fl,    sz, &S[N[n].star0].pos);
        glColorPointer             (3, ub,    sz, &S[N[n].star0].col);
        glVertexAttribPointerARB(6, 1, fl, 0, sz, &S[N[n].star0].mag);

        for (i = 0; i < N[n].starc; ++i)
            if (node_test(V, S[i].pos))
            {
                glDrawArrays(GL_POINTS, i, 1);
                c++;
            }

        printf("%d\n", c);
    }
    glPopClientAttrib();
    glPopAttrib();
}

/*
void node_draw(void)
{
    GLenum  ub = GL_UNSIGNED_BYTE;
    GLenum  fl = GL_FLOAT;
    GLsizei sz = sizeof (struct star);

    int n = N_curr;

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

        glVertexPointer            (3, fl,    sz, &S[N[n].star0].pos);
        glColorPointer             (3, ub,    sz, &S[N[n].star0].col);
        glVertexAttribPointerARB(6, 1, fl, 0, sz, &S[N[n].star0].mag);

        glDrawArrays(GL_POINTS, 0, N[n].starc);
    }
    glPopClientAttrib();
    glPopAttrib();
}
*/
/*---------------------------------------------------------------------------*/

