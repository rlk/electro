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
#define N_SIZ    4096
#define S_MAX 2621440

static struct node *N;
static int          N_num;
static int          N_max;

static struct star *S;
static int          S_num;
static int          S_max;

static int N_curr = 0;

/*---------------------------------------------------------------------------*/
void prep_d(int n)
{
    if (0 <= n && n < N[N_curr].nodec)
        N_curr = N[N_curr].node0 + n;
}

void prep_u(void)
{
    if (N_curr) N_curr = N[N_curr].nodep;
}

/*---------------------------------------------------------------------------*/

static int prep_part(float k, int i, int s0, int sn)
{
    struct star T;

    sn--;

    while (s0 < sn)
    {
        while (S[s0].pos[i] < k) s0++;
        while (S[sn].pos[i] > k) sn--;

        if (s0 < sn)
        {
            T     = S[s0];
            S[s0] = S[sn];
            S[sn] = T;
        }
    }

    return s0;
}

static void prep_node(int n0, int s0, int s8, float x0, float x2,
                                              float y0, float y2,
                                              float z0, float z2)
{
    N[n0].star0 = s0;
    N[n0].starc = s8 - s0;

    N[n0].x0 = x0;
    N[n0].x1 = x2;
    N[n0].y0 = y0;
    N[n0].y1 = y2;
    N[n0].z0 = z0;
    N[n0].z1 = z2;

    if (s8 - s0 > N_SIZ && N_num < N_max)
    {
        float x1 = (x0 + x2) / 2;
        float y1 = (y0 + y2) / 2;
        float z1 = (z0 + z2) / 2;

        int s4 = prep_part(x1, 0, s0, s8);
        int s2 = prep_part(y1, 1, s0, s4);
        int s6 = prep_part(y1, 1, s4, s8);
        int s1 = prep_part(z1, 2, s0, s2);
        int s3 = prep_part(z1, 2, s2, s4);
        int s5 = prep_part(z1, 2, s4, s6);
        int s7 = prep_part(z1, 2, s6, s8);

        int N0 = N_num;

        int n1 = (s1 > s0) ? N_num++ : 0;
        int n2 = (s2 > s1) ? N_num++ : 0;
        int n3 = (s3 > s2) ? N_num++ : 0;
        int n4 = (s4 > s3) ? N_num++ : 0;
        int n5 = (s5 > s4) ? N_num++ : 0;
        int n6 = (s6 > s5) ? N_num++ : 0;
        int n7 = (s7 > s6) ? N_num++ : 0;
        int n8 = (s8 > s7) ? N_num++ : 0;

        int N8 = N_num;

        N[n0].node0 = N0;
        N[n0].nodec = N8 - N0;

        if (n1) prep_node(n1, s0, s1, x0, x1, y0, y1, z0, z1);
        if (n2) prep_node(n2, s1, s2, x0, x1, y0, y1, z1, z2);
        if (n3) prep_node(n3, s2, s3, x0, x1, y1, y2, z0, z1);
        if (n4) prep_node(n4, s3, s4, x0, x1, y1, y2, z1, z2);
        if (n5) prep_node(n5, s4, s5, x1, x2, y0, y1, z0, z1);
        if (n6) prep_node(n6, s5, s6, x1, x2, y0, y1, z1, z2);
        if (n7) prep_node(n7, s6, s7, x1, x2, y1, y2, z0, z1);
        if (n8) prep_node(n8, s7, s8, x1, x2, y1, y2, z1, z2);


        if (n1) N[n1].nodep = n0;
        if (n2) N[n2].nodep = n0;
        if (n3) N[n3].nodep = n0;
        if (n4) N[n4].nodep = n0;
        if (n5) N[n5].nodep = n0;
        if (n6) N[n6].nodep = n0;
        if (n7) N[n7].nodep = n0;
        if (n8) N[n8].nodep = n0;
    }
}

void prep_dump(int n, int d)
{
    int i;

    for (i = 0; i < d; i++)
        printf(" ");

    printf("%d: %d %d %d %d\n", n,
           N[n].star0, N[n].starc,
           N[n].node0, N[n].nodec);

    for (i = 0; i < N[n].nodec; i++)
        prep_dump(N[n].node0 + i, d + 1);
}

/*---------------------------------------------------------------------------*/

static float x_min, x_max;
static float y_min, y_max;
static float z_min, z_max;

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

    x_min = y_min = z_min =  HUGE_VAL;
    x_max = y_max = z_max = -HUGE_VAL;
}

void prep_sort(void)
{
    int i;

    for (i = 0; i < S_num; ++i)
    {
        if (x_min > S[i].pos[0]) x_min = S[i].pos[0];
        if (x_max < S[i].pos[0]) x_max = S[i].pos[0];
        if (y_min > S[i].pos[1]) y_min = S[i].pos[1];
        if (y_max < S[i].pos[1]) y_max = S[i].pos[1];
        if (z_min > S[i].pos[2]) z_min = S[i].pos[2];
        if (z_max < S[i].pos[2]) z_max = S[i].pos[2];
    }

    N_num = 1;
    prep_node(0, 0, S_num, x_min, x_max, y_min, y_max, z_min, z_max);
    prep_dump(0, 0);
    printf("%d %d\n", S_num, N_num);
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

static GLuint star_texture;
static GLuint star_frag;
static GLuint star_vert;

void node_init(void)
{
    star_texture = star_make_texture();
    star_frag    = star_frag_program();
    star_vert    = star_vert_program();
}

static void box(float x0, float x1, float y0, float y1, float z0, float z1)
{
    glBegin(GL_LINES);
    {
        glVertex3f(x0, y0, z0); glVertex3f(x0, y0, z1);
        glVertex3f(x0, y1, z0); glVertex3f(x0, y1, z1);
        glVertex3f(x1, y0, z0); glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z0); glVertex3f(x1, y1, z1);

        glVertex3f(x0, y0, z0); glVertex3f(x0, y1, z0);
        glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1);
        glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0);
        glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1);

        glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0);
        glVertex3f(x0, y0, z1); glVertex3f(x1, y0, z1);
        glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0);
        glVertex3f(x0, y1, z1); glVertex3f(x1, y1, z1);
    }
    glEnd();
}

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

        printf("draw %d: %d %d\n", n, N[n].star0, N[n].starc);
    }
    glPopClientAttrib();
    glPopAttrib();
    
    box(N[n].x0, N[n].x1, N[n].y0, N[n].y1, N[n].z0, N[n].z1);
}

/*---------------------------------------------------------------------------*/

