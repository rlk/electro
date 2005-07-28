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

static struct galaxy *get_galaxy(int i)
{
    return (struct galaxy *) vecget(galaxy, i);
}

static int new_galaxy(void)
{
    int i, n = vecnum(galaxy);

    for (i = 0; i < n; ++i)
        if (get_galaxy(i)->count == 0)
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

        H.N_num = host_to_net_int(g->N_num);
        H.S_num = host_to_net_int(g->S_num);

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
        struct galaxy *g = get_galaxy(i);

        /* If the file exists and is successfully read... */

        if ((parse_galaxy(filename, g)))
        {
            g->count = 1;

            /* Pack the object header. */

            send_event(EVENT_CREATE_GALAXY);
            send_index(g->S_num);
            send_index(g->N_num);

            /* Pack the stars and BSP nodes. */

            send_array(g->S, g->S_num, sizeof (struct star));
            send_array(g->N, g->N_num, sizeof (struct node));

            /* Encapsulate this object in an entity. */

            return send_create_entity(TYPE_GALAXY, i);
        }
    }
    return -1;
}

void recv_create_galaxy(void)
{
    struct galaxy *g = get_galaxy(new_galaxy());

    g->count = 1;

    /* Receive the object header. */

    g->S_num = recv_index();
    g->N_num = recv_index();

    /* Receive the stars and BSP nodes. */

    g->S = (struct star *) malloc(g->S_num * sizeof (struct star));
    g->N = (struct node *) malloc(g->N_num * sizeof (struct node));

    recv_array(g->S, g->S_num, sizeof (struct star));
    recv_array(g->N, g->N_num, sizeof (struct node));

    /* Encapsulate this object in an entity. */

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_galaxy_magnitude(int i, float m)
{
    send_event(EVENT_SET_GALAXY_MAGNITUDE);
    send_index(i);
    send_float(m);

    get_galaxy(i)->magnitude = m;
}

void recv_set_galaxy_magnitude(void)
{
    struct galaxy *g = get_galaxy(recv_index());

    g->magnitude = recv_float();
}

/*---------------------------------------------------------------------------*/
/* Galaxy object star query                                                  */

void get_star_position(int i, int j, float p[3])
{
    struct galaxy *g = get_galaxy(i);

    if (0 <= j && j < g->S_num)
    {
        p[0] = g->S[j].pos[0];
        p[1] = g->S[j].pos[1];
        p[2] = g->S[j].pos[2];
    }
    else p[0] = p[1] = p[2] = 0.0;
}

int pick_galaxy(int i, const float p[3], const float v[3])
{
    struct galaxy *g = get_galaxy(i);
    float d = 0;

    return node_pick(g->N, 0, g->S, 0, p, v, &d);
}

/*===========================================================================*/

static void init_galaxy(int i)
{
    struct galaxy *g = get_galaxy(i);

    if (g->state == 0)
    {
        /* Initialize the vertex buffer object. */

        if (GL_has_vertex_buffer_object)
        {
            glGenBuffersARB(1, &g->buffer);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, g->buffer);

            glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                            g->S_num * sizeof (struct star),
                            g->S, GL_STATIC_DRAW_ARB);
        }

        /* Initialize the star texture. */

        g->texture = star_make_texture();
        g->state   = 1;
    }
}

static void fini_galaxy(int i)
{
    struct galaxy *g = get_galaxy(i);

    if (g->state == 1)
    {
        /* Free the star texture. */

        if (glIsTexture(g->texture))
            glDeleteTextures(1, &g->texture);

        /* Free the vertex buffer object. */

        if (GL_has_vertex_buffer_object)
            if (glIsBufferARB(g->buffer))
                glDeleteBuffersARB(1, &g->buffer);

        g->texture = 0;
        g->buffer  = 0;
        g->state   = 0;
    }
}

/*---------------------------------------------------------------------------*/

static void draw_arrays(int i)
{
    struct galaxy *g = get_galaxy(i);
    GLsizei sz = sizeof (struct star);

    if (GL_has_vertex_program)
        glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
                                   1, g->magnitude, 0, 0, 0);

    /* Enable the star arrays. */

    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    /* Bind the vertex buffers. */

    if (GL_has_vertex_buffer_object)
    {
        glEnableVertexAttribArrayARB(6);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, g->buffer);

        glColorPointer (3, GL_UNSIGNED_BYTE, sz, (GLvoid *) 0);
        glVertexPointer(3, GL_FLOAT,         sz, (GLvoid *) 4);

        glVertexAttribPointerARB(6, 1, GL_FLOAT, 0, sz, (GLvoid *) 16);
    }
    else
    {
        glColorPointer (3, GL_UNSIGNED_BYTE, sz, g->S->col);
        glVertexPointer(3, GL_FLOAT,         sz, g->S->pos);
    }
}

static void draw_galaxy(int j, int i, int f, float a)
{
    struct galaxy *g = get_galaxy(i);
    struct frustum F;

    init_galaxy(i);

    glPushMatrix();
    {
        /* Apply the local coordinate system transformation. */

        transform_entity(j);
        get_frustum(&F);

        /* Supply the view position as a vertex program parameter. */

        if (GL_has_vertex_program)
            glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0,
                                       F.p[0], F.p[1], F.p[2], F.p[3]);

        glPushAttrib(GL_ENABLE_BIT       |
                     GL_TEXTURE_BIT      |
                     GL_DEPTH_BUFFER_BIT |
                     GL_COLOR_BUFFER_BIT);
        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        {
            /* Set up the GL state for star rendering. */

            glBindTexture(GL_TEXTURE_2D, g->texture);

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glEnable(GL_COLOR_MATERIAL);

            glDepthMask(GL_FALSE);
            glBlendFunc(GL_ONE, GL_ONE);

            if (GL_has_point_sprite)
            {
                glEnable(GL_POINT_SPRITE_ARB);
                glTexEnvi(GL_POINT_SPRITE_ARB,
                          GL_COORD_REPLACE_ARB, GL_TRUE);
            }

            draw_arrays(i);

            /* Render all stars. */

            node_draw(g->N, 0, 0, &F);
        }
        glPopClientAttrib();
        glPopAttrib();

        /* Render all child entities in this coordinate system. */

        draw_entity_tree(j, f, a * get_entity_alpha(j));
    }
    glPopMatrix();
}

static void aabb_galaxy(int i, float aabb[6])
{
    struct galaxy *g = get_galaxy(i);

    if (g->N)
    {
        aabb[0] = g->N->bound[0];
        aabb[1] = g->N->bound[1];
        aabb[2] = g->N->bound[2];
        aabb[3] = g->N->bound[3];
        aabb[4] = g->N->bound[4];
        aabb[5] = g->N->bound[5];
    }
    else memset(aabb, 0, 6 * sizeof (float));
}

/*---------------------------------------------------------------------------*/
/* These may only be called by create_clone and delete_entity, respectively. */

static void dupe_galaxy(int i)
{
    get_galaxy(i)->count++;
}

static void free_galaxy(int i)
{
    struct galaxy *g = get_galaxy(i);

    if (--g->count == 0)
    {
        fini_galaxy(i);

        if (g->S) free(g->S);
        if (g->N) free(g->N);

        memset(g, 0, sizeof (struct galaxy));
    }
}

/*===========================================================================*/

static struct entity_func galaxy_func = {
    "galaxy",
    init_galaxy,
    fini_galaxy,
    aabb_galaxy,
    draw_galaxy,
    dupe_galaxy,
    free_galaxy,
};

struct entity_func *startup_galaxy(void)
{
    if ((galaxy = vecnew(MIN_GALAXIES, sizeof (struct galaxy))))
        return &galaxy_func;
    else
        return NULL;
}
