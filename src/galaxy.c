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
#include "vector.h"
#include "matrix.h"
#include "buffer.h"
#include "entity.h"
#include "event.h"
#include "utility.h"
#include "galaxy.h"
#include "node.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

struct galaxy
{
    int    count;
    int    state;
    int    S_num;
    int    N_num;
    GLuint buffer;
    GLuint texture;
    float  magnitude;

    struct star *S;
    struct node *N;
};

static vector_t galaxy;

/*---------------------------------------------------------------------------*/

#define N_MAX   32768
#define S_MAX 2621440

#define G(i) ((struct galaxy *) vecget(galaxy, i))

static int new_galaxy(void)
{
    int i, n = vecnum(galaxy);

    for (i = 0; i < n; ++i)
        if (G(i)->count == 0)
            return i;

    return vecadd(galaxy);
}

/*===========================================================================*/
/* Galaxy file I/O                                                           */

struct head
{
    int   N_num;
    int   S_num;
};

int parse_galaxy(const char *filename, struct galaxy *g)
{
    FILE *fp;
    int i;

    if ((fp = open_file(filename, FMODE_RB)))
    {
        struct head H;

        if (fread(&H, sizeof (struct head), 1, fp) == 1)
        {
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

    if ((fp = open_file(filename, FMODE_WB)))
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
/* Raw stellar catalog readers                                               */

static int prep_parse_hip(struct star *S, int S_num, const char *filename)
{
    struct stat buf;
    FILE *fp;

    if (stat_file(filename, &buf) == 0)
    {
        if ((fp = open_file(filename, "r")))
        {
            int i, n = (int) buf.st_size / STAR_HIP_RECLEN;

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

    if (stat_file(filename, &buf) == 0)
    {
        if ((fp = open_file(filename, "r")))
        {
            int i, n = (int) buf.st_size / STAR_TYC_RECLEN;

            for (i = 0; i < n && S_num < S_MAX; i++)
                S_num += star_parse_tyc(S + S_num, fp);

            fclose(fp);
        }
    }
    return S_num;
}

/*---------------------------------------------------------------------------*/
/* Stellar catalog to galaxy file preprocessing                              */

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
    g->N_num = node_sort(g->N, 0, 1, g->S, 0, g->S_num, 0);
}

/*---------------------------------------------------------------------------*/

void prep_tyc_galaxy(const char *dat_name,
                     const char *gal_name)
{
    struct galaxy g;

    init_galaxy_prep(&g);
    {
        g.S_num = prep_parse_tyc(g.S, g.S_num, dat_name);
    }
    fini_galaxy_prep(&g);

    write_galaxy(gal_name, &g);
    free_galaxy_prep(&g);
}

void prep_hip_galaxy(const char *dat_name,
                     const char *gal_name)
{
    struct galaxy g;

    init_galaxy_prep(&g);
    {
        g.S_num = star_gimme_sol(g.S);
        g.S_num = prep_parse_hip(g.S, g.S_num, dat_name);
    }
    fini_galaxy_prep(&g);

    write_galaxy(gal_name, &g);
    free_galaxy_prep(&g);
}

/*===========================================================================*/

int send_create_galaxy(const char *filename)
{
    int i;

    if ((i = new_galaxy()) >= 0)
    {
        /* If the file exists and is successfully read... */

        if ((parse_galaxy(filename, G(i))))
        {
            G(i)->count = 1;

            /* Pack the object header. */

            pack_event(EVENT_CREATE_GALAXY);
            pack_index(G(i)->S_num);
            pack_index(G(i)->N_num);

            /* Pack the stars and BSP nodes. */

            pack_alloc(G(i)->S_num * sizeof (struct star), G(i)->S);
            pack_alloc(G(i)->N_num * sizeof (struct node), G(i)->N);

            /* Encapsulate this object in an entity. */

            return send_create_entity(TYPE_GALAXY, i);
        }
    }
    return -1;
}

void recv_create_galaxy(void)
{
    int i = new_galaxy();

    G(i)->count = 1;

    /* Unpack the object header. */

    G(i)->S_num = unpack_index();
    G(i)->N_num = unpack_index();

    /* Unpack the stars and BSP nodes. */

    G(i)->S = unpack_alloc(G(i)->S_num * sizeof (struct star));
    G(i)->N = unpack_alloc(G(i)->N_num * sizeof (struct node));
    
    /* Encapsulate this object in an entity. */

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_galaxy_magnitude(int i, float m)
{
    pack_event(EVENT_SET_GALAXY_MAGNITUDE);
    pack_index(i);
    pack_float(m);

    G(i)->magnitude = m;
}

void recv_set_galaxy_magnitude(void)
{
    int i = unpack_index();

    G(i)->magnitude = unpack_float();
}

/*---------------------------------------------------------------------------*/
/* Galaxy object star query                                                  */

void get_star_position(int i, int j, float p[3])
{
    if (0 <= j && j < G(i)->S_num)
    {
        p[0] = G(i)->S[j].pos[0];
        p[1] = G(i)->S[j].pos[1];
        p[2] = G(i)->S[j].pos[2];
    }
    else p[0] = p[1] = p[2] = 0.0;
}

int pick_galaxy(int i, const float p[3], const float v[3])
{
    float d = 0;

    return node_pick(G(i)->N, 0, G(i)->S, 0, p, v, &d);
}

/*===========================================================================*/

static void draw_arrays(int i)
{
    GLsizei sz = sizeof (struct star);

    if (GL_has_vertex_program)
        glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
                                   1, G(i)->magnitude, 0, 0, 0);

    /* Enable the star arrays. */

    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    /* Bind the vertex buffers. */

    if (GL_has_vertex_buffer_object)
    {
        glEnableVertexAttribArrayARB(6);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, G(i)->buffer);

        glColorPointer (3, GL_UNSIGNED_BYTE, sz, (GLvoid *) 0);
        glVertexPointer(3, GL_FLOAT,         sz, (GLvoid *) 4);

        glVertexAttribPointerARB(6, 1, GL_FLOAT, 0, sz, (GLvoid *) 16);
    }
    else
    {
        glColorPointer (3, GL_UNSIGNED_BYTE, sz, G(i)->S->col);
        glVertexPointer(3, GL_FLOAT,         sz, G(i)->S->pos);
    }
}

static void draw_galaxy(int j, int i, const float M[16],
                                      const float I[16],
                                      const struct frustum *F, float a)
{
    glPushMatrix();
    {
        float N[16];
        float J[16];

        struct frustum E;

        /* Apply the local coordinate system transformation. */

        transform_entity(j, N, M, J, I);

        m_pfrm(E.V[0], N, F->V[0]);
        m_pfrm(E.V[1], N, F->V[1]);
        m_pfrm(E.V[2], N, F->V[2]);
        m_pfrm(E.V[3], N, F->V[3]);
        m_xfrm(E.p,    J, F->p);

        /* Supply the view position as a vertex program parameter. */

        if (GL_has_vertex_program)
            glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0,
                                       -E.p[0], -E.p[1], -E.p[2], -E.p[3]);

        glPushAttrib(GL_ENABLE_BIT  |
                     GL_TEXTURE_BIT |
                     GL_COLOR_BUFFER_BIT);
        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        {
            /* Set up the GL state for star rendering. */

            glBindTexture(GL_TEXTURE_2D, G(i)->texture);

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

            draw_arrays(i);

            /* Render all stars. */

            node_draw(G(i)->N, 0, 0, &E);
        }
        glPopClientAttrib();
        glPopAttrib();

        /* Render all child entities in this coordinate system. */

        draw_entity_tree(j, N, J, F, a * get_entity_alpha(j));
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

static void init_galaxy(int i)
{
    if (G(i)->state == 0)
    {
        /* Initialize the vertex buffer object. */

        if (GL_has_vertex_buffer_object)
        {
            glGenBuffersARB(1, &G(i)->buffer);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, G(i)->buffer);

            glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                            G(i)->S_num * sizeof (struct star),
                            G(i)->S, GL_STATIC_DRAW_ARB);
        }

        /* Initialize the star texture. */

        G(i)->texture = star_make_texture();
        G(i)->state   = 1;
    }
}

static void fini_galaxy(int i)
{
    if (G(i)->state == 1)
    {
        /* Free the star texture. */

        if (glIsTexture(G(i)->texture))
            glDeleteTextures(1, &G(i)->texture);

        /* Free the vertex buffer object. */

        if (GL_has_vertex_buffer_object)
            if (glIsBufferARB(G(i)->buffer))
                glDeleteBuffersARB(1, &G(i)->buffer);

        G(i)->texture = 0;
        G(i)->buffer  = 0;
        G(i)->state   = 0;
    }
}

/*---------------------------------------------------------------------------*/
/* These may only be called by create_clone and delete_entity, respectively. */

static void dupe_galaxy(int i)
{
    G(i)->count++;
}

static void free_galaxy(int i)
{
    if (--G(i)->count == 0)
    {
        fini_galaxy(i);

        if (G(i)->S) free(G(i)->S);
        if (G(i)->N) free(G(i)->N);

        memset(G(i), 0, sizeof (struct galaxy));
    }
}

/*===========================================================================*/

static struct entity_func galaxy_func = {
    "galaxy",
    init_galaxy,
    fini_galaxy,
    draw_galaxy,
    dupe_galaxy,
    free_galaxy,
};

struct entity_func *startup_galaxy(void)
{
    if ((galaxy = vecnew(4, sizeof (struct galaxy))))
        return &galaxy_func;
    else
        return NULL;
}
