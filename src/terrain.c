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
#include <math.h>

#include "opengl.h"
#include "frustum.h"
#include "matrix.h"
#include "buffer.h"
#include "entity.h"
#include "event.h"
#include "vec.h"

/*---------------------------------------------------------------------------*/

struct vertex
{
    float v[3];
    float n[3];
    float t[2];
};

struct terrain
{
    int count;
    int state;

    short *P;
    int    w;
    int    h;

    GLuint vbo;
    GLuint ibo;
};

static struct terrain *terrain;

/*---------------------------------------------------------------------------*/

static unsigned int new_terrain(void)
{
    unsigned int i;
    void        *v;

    if ((i = vec_add(terrain, sizeof (struct terrain))))
    {
        memset(terrain +i, 0, sizeof (struct terrain));
        return i;
    }

    if ((v = vec_gro(terrain, sizeof (struct terrain))))
    {
        terrain = (struct terrain *) v;
        return new_terrain();
    }
    return 0;
}

/*===========================================================================*/

static int load_terrain(const char *filename, struct terrain *t)
{
    return 1;
}

/*===========================================================================*/

int send_create_terrain(const char *filename, int w, int h)
{
    int i;

    if ((i = new_terrain()))
    {
        /* If the file exists and is successfully loaded... */

        if ((load_terrain(filename, terrain + i)))
        {
            terrain[i].count = 1;
            terrain[i].w     = w;
            terrain[i].h     = h;

            /* Pack the header and data. */

            send_event(EVENT_CREATE_TERRAIN);
            send_value(terrain[i].w);
            send_value(terrain[i].h);
            send_array(terrain[i].P, w * h, sizeof (short));

            /* Encapsulate this object in an entity. */

            return send_create_entity(TYPE_TERRAIN, i);
        }
    }
    return -1;
}

void recv_create_terrain(void)
{
    int i;

    if ((i = new_terrain()))
    {
        int w = recv_value();
        int h = recv_value();

        terrain[i].count = 1;

        terrain[i].w = w;
        terrain[i].h = h;
        terrain[i].P = (short *) malloc(w * h * sizeof (short));

        recv_array(terrain[i].P, w * h, sizeof (short));

        /* Encapsulate this object in an entity. */

        recv_create_entity();
    }
}

/*===========================================================================*/

/*

Given (x, y, w, h) determine the closest mipmap level and initialize a
VBO for it.

*/

static void init_terrain(int i)
{
    if (terrain[i].state == 0)
    {
        struct vertex *V;
        GLushort      *I;

        int r, h = terrain[i].h;
        int c, w = terrain[i].w;

        float R = 1.0f;

        /* Initialize the vertex buffer and index buffer objects. */

        if (GL_has_vertex_buffer_object)
        {
            glGenBuffersARB(1, &terrain[i].vbo);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, terrain[i].vbo);

            if ((V = (struct vertex *) glMapBufferARB(GL_ARRAY_BUFFER_ARB,
                                                      GL_WRITE_ONLY_ARB)))
            {
                for (r = 0; r < h; ++r)
                    for (c = 0; c < w; ++c)
                    {
                        struct vertex *v = V + (r * w) + c;

                        v->v[0] =  R * cos(RAD(c)) * sin(RAD(r));
                        v->v[1] = -R               * cos(RAD(r));
                        v->v[2] =  R * sin(RAD(c)) * sin(RAD(r));

                        v->n[0] = 0.0f;
                        v->n[1] = 0.0f;
                        v->n[2] = 1.0f;

                        v->t[0] = (float) c / (float) w;
                        v->t[1] = (float) r / (float) h;
                    }

                glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
            }

        }

        terrain[i].state  = 1;
    }
}

static void fini_terrain(int i)
{
    if (terrain[i].state == 1)
    {
        terrain[i].state  = 0;
    }
}

/*---------------------------------------------------------------------------*/

static void draw_terrain(int i, int j, int f, float a)
{
    float V[6][4];
    float p[4];

    init_terrain(i);

    glPushMatrix();
    {
        /* Apply the local coordinate system transformation. */

        transform_entity(j);

        get_viewfrust(V);
        get_viewpoint(p);

        /* Draw here. */

        /* Render all child entities in this coordinate system. */

        draw_entity_tree(j, f, a * get_entity_alpha(j));
    }
    glPopMatrix();
}

static void aabb_terrain(int i, float aabb[6])
{
    memset(aabb, 0, 6 * sizeof (float));
}

/*---------------------------------------------------------------------------*/
/* These may only be called by create_clone and delete_entity, respectively. */

static void dupe_terrain(int i)
{
    terrain[i].count++;
}

static void free_terrain(int i)
{
    if (terrain[i].count > 0)
    {
        terrain[i].count--;

        if (terrain[i].count == 0)
        {
            fini_terrain(i);

            if (terrain[i].P) free(terrain[i].P);

            memset(terrain + i, 0, sizeof (struct terrain));
        }
    }
}

/*===========================================================================*/

static struct entity_func terrain_func = {
    "terrain",
    init_terrain,
    fini_terrain,
    aabb_terrain,
    draw_terrain,
    dupe_terrain,
    free_terrain,
};

struct entity_func *startup_terrain(void)
{
    if ((terrain = vec_new(MIN_TERRAINS, sizeof (struct terrain))))
        return &terrain_func;
    else
        return NULL;
}
