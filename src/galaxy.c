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
#include <math.h>
#include <sys/stat.h>

#include "opengl.h"
#include "utility.h"
#include "viewport.h"
#include "buffer.h"
#include "entity.h"
#include "event.h"
#include "utility.h"
#include "galaxy.h"

/*---------------------------------------------------------------------------*/

#define N_MAX   16834
#define S_MAX 2621440

#define GMAXINIT 4

static struct galaxy *G;
static int            G_max;

static GLuint star_texture;
static GLuint star_frag;
static GLuint star_vert;

static int galaxy_exists(int gd)
{
    return (G && 0 <= gd && gd < G_max && G[gd].count);
}

static int alloc_galaxy(void)
{
    return balloc((void **) &G, &G_max, sizeof (struct galaxy), galaxy_exists);
}

/*---------------------------------------------------------------------------*/

int init_galaxy(void)
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

void draw_galaxy(int id, int gd, const float V[16], float a)
{
    GLsizei sz = sizeof (struct star);

    GLenum ub = GL_UNSIGNED_BYTE;
    GLenum fl = GL_FLOAT;

    float W[16];
    int c = 0;

    if (galaxy_exists(gd))
    {
        glPushMatrix();
        {
            /* Apply the local coordinate system transformation. */

            transform_entity(id, W, V);

            glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            {
                /* Set up the GL state for star rendering. */

                glBindTexture(GL_TEXTURE_2D, star_texture);
                glBlendFunc(GL_ONE, GL_ONE);

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_LIGHTING);
                glDisable(GL_DEPTH_TEST);
                glEnable(GL_COLOR_MATERIAL);

                if (GL_has_program)
                {
                    glEnable(GL_VERTEX_PROGRAM_ARB);
                    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
                    glEnable(GL_FRAGMENT_PROGRAM_ARB);
                }
                if (GL_has_point_sprite)
                    glEnable(GL_POINT_SPRITE_ARB);

                if (GL_has_program)
                {
                    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, star_frag);
                    glBindProgramARB(GL_VERTEX_PROGRAM_ARB,   star_vert);
                    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
                                               1, G[gd].magnitude, 0, 0, 0);
                    glEnableVertexAttribArrayARB(6);
                }
                if (GL_has_point_sprite)
                    glTexEnvi(GL_POINT_SPRITE_ARB,
                              GL_COORD_REPLACE_ARB, GL_TRUE);

                /* Enable the star arrays. */

                glEnableClientState(GL_COLOR_ARRAY);
                glEnableClientState(GL_VERTEX_ARRAY);

                glColorPointer (3, ub, sz, G[gd].S->col);
                glVertexPointer(3, fl, sz, G[gd].S->pos);

                if (GL_has_program)
                    glVertexAttribPointerARB(6, 1, fl, 0, sz, &G[gd].S->mag);

                /* Render all stars. */

                c = node_draw(G[gd].N, 0, 0, V, G[gd].bound);
            }
            glPopClientAttrib();
            glPopAttrib();

            /* Render all child entities in this coordinate system. */

            draw_entity_list(id, W, a * get_entity_alpha(id));
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/

struct head
{
    int   N_num;
    int   S_num;
    float bound[6];
};

int parse_galaxy(const char *filename, struct galaxy *g)
{
    FILE *fp;

    if ((fp = fopen(filename, FMODE_RB)))
    {
        struct head H;

        if (fread(&H, sizeof (struct head), 1, fp) == 1)
        {
            int i;

            g->magnitude = 1.0;
            g->N_num     = ntohl(H.N_num);
            g->S_num     = ntohl(H.S_num);
            g->bound[0]  = ntohf(H.bound[0]);
            g->bound[1]  = ntohf(H.bound[1]);
            g->bound[2]  = ntohf(H.bound[2]);
            g->bound[3]  = ntohf(H.bound[3]);
            g->bound[4]  = ntohf(H.bound[4]);
            g->bound[5]  = ntohf(H.bound[5]);

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
    else error("Failure opening galaxy file '%s'", filename);

    return 0;
}

int write_galaxy(const char *filename, struct galaxy *g)
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
    else error("Failure opening galaxy file '%s' for writing", filename);

    return 0;
}

/*---------------------------------------------------------------------------*/

static void init_galaxy_prep(struct galaxy *g)
{
    if ((g->N = (struct node *) calloc(N_MAX, sizeof (struct node))))
        g->N_num = 0;

    if ((g->S = (struct star *) calloc(S_MAX, sizeof (struct star))))
        g->S_num = 0;
}

static void free_galaxy_prep(struct galaxy *g)
{
    if (g->N) free(g->N);
    if (g->S) free(g->S);

    memset(g, 0, sizeof (struct galaxy));
}

static void fini_galaxy_prep(struct galaxy *g)
{
    int i;

    /* Sort the stars into a BSP tree. */

    g->N_num = node_sort(g->N, 0, 1, g->S, 0, g->S_num, 0);

    /* Find the Outer Limits (please stand by). */

    g->bound[0] = g->bound[1] = g->bound[2] =  FLT_MAX;
    g->bound[3] = g->bound[4] = g->bound[5] =  FLT_MIN;

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

static int prep_parse_hip(struct star *S, int S_num, const char *filename)
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

static int prep_parse_tyc(struct star *S, int S_num, const char *filename)
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

void prep_tyc_galaxy(void)
{
    struct galaxy g;

    init_galaxy_prep(&g);
    {
        g.S_num = prep_parse_tyc(g.S, g.S_num, "examples/tyc2.dat");
    }
    fini_galaxy_prep(&g);

    write_galaxy("examples/galaxy_tyc.gal", &g);
    free_galaxy_prep(&g);
}

void prep_hip_galaxy(void)
{
    struct galaxy g;

    init_galaxy_prep(&g);
    {
        g.S_num = star_gimme_sol(g.S);
        g.S_num = prep_parse_hip(g.S, g.S_num, "examples/hip_main.dat");
    }
    fini_galaxy_prep(&g);

    write_galaxy("examples/galaxy_hip.gal", &g);
    free_galaxy_prep(&g);
}

/*---------------------------------------------------------------------------*/

int send_create_galaxy(const char *filename)
{
    int gd;

    if (G && (gd = alloc_galaxy()) >= 0)
    {
        /* If the file exists and is successfully read... */

        if ((parse_galaxy(filename, G + gd)))
        {
            G[gd].count = 1;

            /* Pack the object header. */

            pack_event(EVENT_CREATE_GALAXY);
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

            return send_create_entity(TYPE_GALAXY, gd);
        }
    }
    return -1;
}

void recv_create_galaxy(void)
{
    int gd = unpack_index();

    G[gd].count = 1;

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

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_galaxy_magnitude(int gd, float m)
{
    G[gd].count = 1;

    pack_event(EVENT_SET_GALAXY_MAGNITUDE);
    pack_index(gd);

    pack_float(m);

    G[gd].magnitude = m * get_viewport_scale();
}

void recv_set_galaxy_magnitude(void)
{
    int gd = unpack_index();

    G[gd].magnitude = unpack_float();
}

/*---------------------------------------------------------------------------*/
/* These may only be called by create_clone and delete_entity, respectively. */

void clone_galaxy(int gd)
{
    if (galaxy_exists(gd))
        G[gd].count++;
}

void delete_galaxy(int gd)
{
    if (galaxy_exists(gd))
    {
        G[gd].count--;

        if (G[gd].count == 0)
        {
            if (G[gd].S) free(G[gd].S);
            if (G[gd].N) free(G[gd].N);

            memset(G + gd, 0, sizeof (struct galaxy));
        }
    }
}

/*---------------------------------------------------------------------------*/
