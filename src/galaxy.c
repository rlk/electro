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
#include "buffer.h"
#include "entity.h"
#include "event.h"
#include "utility.h"
#include "galaxy.h"

/*---------------------------------------------------------------------------*/

#define N_MAX   32768
#define S_MAX 2621440

#define GMAXINIT 4

static struct galaxy *G;
static int            G_max;

static GLuint star_texture = 0;

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

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static void draw_galaxy_arrays(int gd)
{
    GLsizei sz = sizeof (struct star);

    if (GL_has_vertex_program)
        glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
                                   1, G[gd].magnitude, 0, 0, 0);
    /* Enable the star arrays. */

    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    /* Bind the vertex buffers. */

    if (GL_has_vertex_buffer_object)
    {
        glEnableVertexAttribArrayARB(6);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, G[gd].buffer);

        glColorPointer (3, GL_UNSIGNED_BYTE, sz, (GLvoid *) 0);
        glVertexPointer(3, GL_FLOAT,         sz, (GLvoid *) 4);

        glVertexAttribPointerARB(6, 1, GL_FLOAT, 0, sz, (GLvoid *) 16);
    }
    else
    {
        glColorPointer (3, GL_UNSIGNED_BYTE, sz, G[gd].S->col);
        glVertexPointer(3, GL_FLOAT,         sz, G[gd].S->pos);
    }
}

void draw_galaxy(int id, int gd, const struct frustum *F0, float a)
{
    struct frustum F1;

    if (galaxy_exists(gd))
    {
        glPushMatrix();
        {
            const float d[3] = { 0.0f, 0.0f, 0.0f };

            /* Apply the local coordinate system transformation. */

            transform_entity(id, &F1, F0, d);

            glPushAttrib(GL_ENABLE_BIT  |
                         GL_TEXTURE_BIT |
                         GL_COLOR_BUFFER_BIT);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            {
                /* Set up the GL state for star rendering. */

                glBindTexture(GL_TEXTURE_2D, star_texture);

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_LIGHTING);
                glDisable(GL_DEPTH_TEST);
                glEnable(GL_COLOR_MATERIAL);

                glBlendFunc(GL_ONE, GL_ONE);

                if (GL_has_point_sprite)
                {
                    glEnable(GL_POINT_SPRITE_ARB);
                    glTexEnvi(GL_POINT_SPRITE_ARB,
                              GL_COORD_REPLACE_ARB, GL_TRUE);
                }

                draw_galaxy_arrays(gd);

                /* Render all stars. */

                (void) node_draw(G[gd].N, 0, 0, &F1);
            }
            glPopClientAttrib();
            glPopAttrib();

            /* Render all child entities in this coordinate system. */

            draw_entity_list(id, &F1, a * get_entity_alpha(id));
        }
        glPopMatrix();
    }
}

int pick_galaxy(int id, int gd, const float p[3], const float v[3])
{
    float d = 0;

    return node_pick(G[gd].N, 0, G[gd].S, 0, p, v, &d);
}

/*---------------------------------------------------------------------------*/

struct head
{
    int   N_num;
    int   S_num;
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
    else error("Galaxy file '%s': %s", filename, system_error());

    return 0;
}

int write_galaxy(const char *filename, struct galaxy *g)
{
    FILE *fp;

    if ((fp = fopen(filename, FMODE_WB)))
    {
        struct head H;

        H.N_num = htonl(g->N_num);
        H.S_num = htonl(g->S_num);

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
    else error("Galaxy file '%s': %s", filename, system_error());

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
    /* Sort the stars into a BSP tree. */

    g->N_num = node_sort(g->N, 0, 1, g->S, 0, g->S_num, 0);
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

static void create_galaxy(int gd)
{
    if (GL_has_vertex_buffer_object)
    {
        glGenBuffersARB(1, &G[gd].buffer);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, G[gd].buffer);

        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        G[gd].S_num * sizeof (struct star),
                        G[gd].S, GL_STATIC_DRAW_ARB);
    }
}

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

            /* Pack the stars and BSP nodes. */

            pack_alloc(G[gd].S_num * sizeof (struct star), G[gd].S);
            pack_alloc(G[gd].N_num * sizeof (struct node), G[gd].N);

            create_galaxy(gd);

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

    /* Unpack the stars and BSP nodes. */

    G[gd].S = unpack_alloc(G[gd].S_num * sizeof (struct star));
    G[gd].N = unpack_alloc(G[gd].N_num * sizeof (struct node));
    
    create_galaxy(gd);

    /* Encapsulate this object in an entity. */

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_galaxy_magnitude(int gd, float m)
{
    pack_event(EVENT_SET_GALAXY_MAGNITUDE);
    pack_index(gd);

    pack_float(m);

    G[gd].magnitude = m; /* * get_viewport_scale(); */
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
            if (glIsBufferARB(G[gd].buffer))
                glDeleteBuffersARB(1, &G[gd].buffer);

            if (G[gd].S) free(G[gd].S);
            if (G[gd].N) free(G[gd].N);

            memset(G + gd, 0, sizeof (struct galaxy));
        }
    }
}

/*---------------------------------------------------------------------------*/

void get_star_position(int gd, int si, float p[3])
{
    if (galaxy_exists(gd) && 0 <= si && si < G[gd].S_num)
    {
        p[0] = G[gd].S[si].pos[0];
        p[1] = G[gd].S[si].pos[1];
        p[2] = G[gd].S[si].pos[2];
    }
    else p[0] = p[1] = p[2] = 0.0;
}

/*---------------------------------------------------------------------------*/
