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
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>

#include "opengl.h"
#include "utility.h"
#include "viewport.h"
#include "buffer.h"
#include "shared.h"
#include "entity.h"
#include "galaxy.h"
#include "star.h"
#include "node.h"

/*---------------------------------------------------------------------------*/

#define N_MAX   16834
#define S_MAX 2621440

#define GMAXINIT 2

static struct galaxy *G;
static int            G_max;

static GLuint star_texture;
static GLuint star_frag;
static GLuint star_vert;

static int galaxy_exists(int gd)
{
    return (G && 0 <= gd && gd < G_max && G[gd].S_num);
}

/*---------------------------------------------------------------------------*/

int galaxy_init(void)
{
    if ((G = (struct galaxy *) calloc(GMAXINIT, sizeof (struct galaxy))))
    {
        G_max = GMAXINIT;

        star_texture = star_make_texture();
        star_frag    = star_frag_program();
        star_vert    = star_vert_program();

        return 1;
    }
    return 0;
}

void galaxy_draw(int id, int gd, const float V[16])
{
    GLsizei stride = sizeof (struct star);

    GLenum ub = GL_UNSIGNED_BYTE;
    GLenum fl = GL_FLOAT;

    float W[16];
    int c = 0;

    if (galaxy_exists(gd))
    {
        glPushMatrix();
        {
            /* Apply the local coordinate system transformation. */

            entity_transform(id, W, V);

            glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            {
                /* Set up the GL state for star rendering. */

                glEnable(GL_VERTEX_PROGRAM_ARB);
                glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
                glEnable(GL_FRAGMENT_PROGRAM_ARB);
                glEnable(GL_POINT_SPRITE_ARB);
                glEnable(GL_COLOR_MATERIAL);

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_LIGHTING);
                glDisable(GL_DEPTH_TEST);

                glBindTexture   (GL_TEXTURE_2D,           star_texture);
                glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, star_frag);
                glBindProgramARB(GL_VERTEX_PROGRAM_ARB,   star_vert);

                glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
                glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
                                           1, G[gd].magn, 0, 0, 0);
                glBlendFunc(GL_ONE, GL_ONE);

                /* Enable the star arrays. */

                glEnableClientState(GL_COLOR_ARRAY);
                glEnableClientState(GL_VERTEX_ARRAY);
                glEnableVertexAttribArrayARB(6);

                glColorPointer             (3, ub,    stride,  G[gd].S->col);
                glVertexPointer            (3, fl,    stride,  G[gd].S->pos);
                glVertexAttribPointerARB(6, 1, fl, 0, stride, &G[gd].S->mag);

                /* Render all stars. */

                c = node_draw(G[gd].N, 0, 0, V, G[gd].bound);
            }
            glPopClientAttrib();
            glPopAttrib();

            /* Render all child entities in this coordinate system. */

            entity_traversal(id, W);
        }
        glPopMatrix();
    }

#ifndef NDEBUG
    printf("%d stars\n", c);
#endif
}

/*---------------------------------------------------------------------------*/

struct head
{
    int   N_num;
    int   S_num;
    float bound[6];
};

int galaxy_parse(const char *filename, struct galaxy *g)
{
    FILE *fp;

    if ((fp = fopen(filename, FMODE_RB)))
    {
        struct head H;

        if (fread(&H, sizeof (struct head), 1, fp) == 1)
        {
            int i;

            g->magn     = 1.0;
            g->N_num    = ntohl(H.N_num);
            g->S_num    = ntohl(H.S_num);
            g->bound[0] = ntohf(H.bound[0]);
            g->bound[1] = ntohf(H.bound[1]);
            g->bound[2] = ntohf(H.bound[2]);
            g->bound[3] = ntohf(H.bound[3]);
            g->bound[4] = ntohf(H.bound[4]);
            g->bound[5] = ntohf(H.bound[5]);

            if ((g->N = (struct node *) calloc(g->N_num, sizeof(struct node))))
                for (i = 0; i < g->N_num; ++i)
                    node_parse_bin(g->N + i, fp);

            if ((g->S = (struct star *) calloc(g->S_num, sizeof(struct star))))
                for (i = 0; i < g->S_num; ++i)
                    star_parse_bin(g->S + i, fp);
        }
        fclose(fp);

        return 1;
    }
    return 0;
}

int galaxy_write(const char *filename, struct galaxy *g)
{
    FILE *fp;

    if ((fp = fopen(filename, FMODE_WB)))
    {
        struct head H;

        H.N_num    = htonl(g->N_num);
        H.S_num    = htonl(g->S_num);
        H.bound[0] = htonf(g->bound[0]);
        H.bound[1] = htonf(g->bound[1]);
        H.bound[2] = htonf(g->bound[2]);
        H.bound[3] = htonf(g->bound[3]);
        H.bound[4] = htonf(g->bound[4]);
        H.bound[5] = htonf(g->bound[5]);

        if (fwrite(&H, sizeof (struct head), 1, fp) == 1)
        {
            int i;

            for (i = 0; i < g->N_num; ++i)
                node_write_bin(g->N + i, fp);

            for (i = 0; i < g->S_num; ++i)
                star_write_bin(g->S + i, fp);
        }
        fclose(fp);

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static void galaxy_prep_init(struct galaxy *g)
{
    if ((g->N = (struct node *) calloc(N_MAX, sizeof (struct node))))
        g->N_num = 0;

    if ((g->S = (struct star *) calloc(S_MAX, sizeof (struct star))))
        g->S_num = 0;
}

static void galaxy_prep_free(struct galaxy *g)
{
    if (g->N) free(g->N);
    if (g->S) free(g->S);

    memset(g, 0, sizeof (struct galaxy));
}

static void galaxy_prep_fini(struct galaxy *g)
{
    int i;

    /* Sort the stars into a BSP tree. */

    g->N_num = node_sort(g->N, 0, 1, g->S, 0, g->S_num, 0);

    /* Find the Outer Limits (please stand by). */

    g->bound[0] = g->bound[1] = g->bound[2] =  HUGE_VAL;
    g->bound[3] = g->bound[4] = g->bound[5] = -HUGE_VAL;

    for (i = 0; i < g->S_num; ++i)
    {
        g->bound[0] = MIN(g->bound[0], g->S[i].pos[0]);
        g->bound[1] = MIN(g->bound[1], g->S[i].pos[1]);
        g->bound[2] = MIN(g->bound[2], g->S[i].pos[2]);
        g->bound[3] = MAX(g->bound[3], g->S[i].pos[0]);
        g->bound[4] = MAX(g->bound[4], g->S[i].pos[1]);
        g->bound[5] = MAX(g->bound[5], g->S[i].pos[2]);
    }
}

/*---------------------------------------------------------------------------*/

static int galaxy_prep_hip(struct star *S, int S_num, const char *filename)
{
    struct stat buf;
    FILE *fp;

    if (stat(filename, &buf) == 0)
    {
        if ((fp = fopen(filename, "r")))
        {
            int n = (int) buf.st_size / STAR_HIP_RECLEN;
            int i;

            for (i = 0; i < n && S_num < S_MAX; i++)
                S_num += star_parse_hip(S + S_num, fp);

            fclose(fp);
        }
    }
    return S_num;
}

static int galaxy_prep_tyc(struct star *S, int S_num, const char *filename)
{
    struct stat buf;
    FILE *fp;

    if (stat(filename, &buf) == 0)
    {
        if ((fp = fopen(filename, "r")))
        {
            int n = (int) buf.st_size / STAR_TYC_RECLEN;
            int i;

            for (i = 0; i < n && S_num < S_MAX; i++)
                S_num += star_parse_tyc(S + S_num, fp);

            fclose(fp);
        }
    }
    return S_num;
}

/*---------------------------------------------------------------------------*/

/* TODO: Generalize galaxy preprocessing. */

void galaxy_prep_large(void)
{
    struct galaxy g;

    galaxy_prep_init(&g);

    g.S_num = star_gimme_sol(g.S);
    g.S_num = galaxy_prep_hip(g.S, g.S_num, "../hip_main.dat");
    g.S_num = galaxy_prep_tyc(g.S, g.S_num, "../tyc2.dat");

    galaxy_prep_fini(&g);

    printf("large: %d stars, %d nodes.\n", g.S_num, g.N_num);

    galaxy_write("../galaxy_large.gal", &g);
    galaxy_prep_free(&g);
}

void galaxy_prep_small(void)
{
    struct galaxy g;

    galaxy_prep_init(&g);

    g.S_num = star_gimme_sol(g.S);
    g.S_num = galaxy_prep_hip(g.S, g.S_num, "../hip_main.dat");

    galaxy_prep_fini(&g);

    printf("small: %d stars, %d nodes.\n", g.S_num, g.N_num);

    galaxy_write("../galaxy_small.gal", &g);
    galaxy_prep_free(&g);
}

/*---------------------------------------------------------------------------*/

int galaxy_send_create(const char *filename)
{
    int gd;

    if ((gd = buffer_unused(G_max, galaxy_exists)) >= 0)
    {
        /* If the file exists and is successfully read... */

        if ((galaxy_parse(filename, G + gd)))
        {
            /* Pack the object header. */

            pack_event(EVENT_GALAXY_CREATE);
            pack_index(gd);

            pack_index(G[gd].S_num);
            pack_index(G[gd].N_num);

            pack_float(G[gd].bound[0]);
            pack_float(G[gd].bound[1]);
            pack_float(G[gd].bound[2]);
            pack_float(G[gd].bound[3]);
            pack_float(G[gd].bound[4]);
            pack_float(G[gd].bound[5]);

            /* Pack the stars and BSP nodes. */

            pack_alloc(G[gd].S_num * sizeof (struct star), G[gd].S);
            pack_alloc(G[gd].N_num * sizeof (struct node), G[gd].N);

            /* Encapsulate this object in an entity. */

            return entity_send_create(TYPE_GALAXY, 0);
        }
    }
    return -1;
}

void galaxy_recv_create(void)
{
    int gd = unpack_index();

    /* Unpack the object header. */

    G[gd].S_num = unpack_index();
    G[gd].N_num = unpack_index();

    G[gd].bound[0] = unpack_float();
    G[gd].bound[1] = unpack_float();
    G[gd].bound[2] = unpack_float();
    G[gd].bound[3] = unpack_float();
    G[gd].bound[4] = unpack_float();
    G[gd].bound[5] = unpack_float();

    /* Unpack the stars and BSP nodes. */

    G[gd].S = unpack_alloc(G[gd].S_num * sizeof (struct star));
    G[gd].N = unpack_alloc(G[gd].N_num * sizeof (struct node));
    
    /* Encapsulate this object in an entity. */

    entity_recv_create();
}

/*---------------------------------------------------------------------------*/

void galaxy_send_magn(int gd, float m)
{
    pack_event(EVENT_GALAXY_MAGN);
    pack_index(gd);

    pack_float(m);

    G[gd].magn = m * viewport_scale();
}

void galaxy_recv_magn(void)
{
    int gd = unpack_index();

    G[gd].magn = unpack_float();
}

/*---------------------------------------------------------------------------*/

void galaxy_delete(int gd)
{
    if (galaxy_exists(gd))
    {
        if (G[gd].S) free(G[gd].S);
        if (G[gd].N) free(G[gd].N);
    }

    memset(G + gd, 0, sizeof (struct galaxy));
}

/*---------------------------------------------------------------------------*/
