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

#include "opengl.h"
#include "frustum.h"
#include "utility.h"
#include "matrix.h"
#include "buffer.h"
#include "entity.h"
#include "event.h"
#include "vec.h"

#define OFFSET(i) ((char *) NULL + (i))

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

    short *p;
    int    w;
    int    h;
    int    n;
    int    o;

    GLuint vbo;
    GLuint ibo;

    struct vertex *scratch;
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

static void get_vertex(float v[4], float x, float y, float r)
{
    v[0] = sin(RAD(x)) * sin(RAD(y)) * r;
    v[1] =              -cos(RAD(y)) * r;
    v[2] = cos(RAD(x)) * sin(RAD(y)) * r;
    v[3] = 1.0f;
}

static void get_normal(float n[3], const float a[3],
                                   const float b[3],
                                   const float c[3])
{
    float x[3];
    float y[3];

    x[0] = b[0] - a[0];
    x[1] = b[1] - a[1];
    x[2] = b[2] - a[2];

    y[0] = c[0] - a[0];
    y[1] = c[1] - a[1];
    y[2] = c[2] - a[2];

    cross(n, x, y);
    normalize(n);
}

static void get_bounds(float v[4][3], const float M[16],
                       float x, float y, float w, float h, float r)
{
    float a[4][4];
    float b[4][4];

    get_vertex(a[0], x,     y,     r);
    get_vertex(a[1], x + w, y,     r);
    get_vertex(a[2], x + w, y + h, r);
    get_vertex(a[3], x,     y + h, r);

    mult_mat_vec(b[0], M, a[0]);
    mult_mat_vec(b[1], M, a[1]);
    mult_mat_vec(b[2], M, a[2]);
    mult_mat_vec(b[3], M, a[3]);

    v[0][0] = b[0][0] / b[0][3];
    v[0][1] = b[0][1] / b[0][3];
    v[0][2] = b[0][2] / b[0][3];

    v[1][0] = b[1][0] / b[1][3];
    v[1][1] = b[1][1] / b[1][3];
    v[1][2] = b[1][2] / b[1][3];

    v[2][0] = b[2][0] / b[2][3];
    v[2][1] = b[2][1] / b[2][3];
    v[2][2] = b[2][2] / b[2][3];

    v[3][0] = b[3][0] / b[3][3];
    v[3][1] = b[3][1] / b[3][3];
    v[3][2] = b[3][2] / b[3][3];
}

/*---------------------------------------------------------------------------*/

static struct vertex *init_scratch(int n)
{
    struct vertex *buf;

    if ((buf = (struct vertex *) malloc(n * n * sizeof (struct vertex))))
    {
        int i, r, c;

        for (i = 0, r = 0; r < n; ++r)
            for (c = 0; c < n; ++c, ++i)
            {
                buf[i].v[0] =  (float) c;
                buf[i].v[1] =  (float) 0;
                buf[i].v[2] = -(float) r;

                buf[i].n[0] = 0.0f;
                buf[i].n[1] = 1.0f;
                buf[i].n[2] = 0.0f;

                buf[i].t[0] = (float) c / n;
                buf[i].t[1] = (float) r / n;
            }
    }

    return buf;
}

static GLushort *init_indices(int n)
{
    GLushort *buf;

    if ((buf = (GLushort *) malloc((n - 1) * n * 2 * sizeof (GLushort))))
    {
        int i, r, c;

        for (i = 0, r = 0; r < n - 1; ++r)
            if ((r & 1) == 0)
            {
                for (c = 0; c < n; ++c)  /* Even-numbered row. */
                {
                    GLushort A = (r    ) * n + c;
                    GLushort B = (r + 1) * n + c;

                    buf[i++] = B;
                    buf[i++] = A;
                }
            }
            else
            {
                for (c = 0; c < n; ++c)  /* Odd-numbered row. */
                {
                    GLushort A = (r    ) * n + (n - c - 1);
                    GLushort B = (r + 1) * n + (n - c - 1);

                    buf[i++] = A;
                    buf[i++] = B;
                }
            }
    }

    return buf;
}

/*---------------------------------------------------------------------------*/

static GLuint init_vbo(int n, const struct vertex *data)
{
    GLuint vbo;
    GLsizei N = n * n * sizeof (struct vertex);

    glGenBuffersARB(1, &vbo);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, N, data, GL_STATIC_DRAW_ARB);

    opengl_check("init_vbo");
    return vbo;
}

static GLuint init_ibo(int n, const GLushort *data)
{
    GLuint ibo;
    GLsizei N = (n - 1) * n * 2 * sizeof (GLushort);

    glGenBuffersARB(1, &ibo);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ibo);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, N, data, GL_STATIC_DRAW_ARB);

    opengl_check("init_ibo");
    return ibo;
}

/*---------------------------------------------------------------------------*/

static short *load_terrain(const char *filename, int w, int h)
{
    FILE *fp;
    short *p;

    if ((p = (short *) malloc((w * h + w * h / 3 + 1) * sizeof (short))))
    {
        short *D = p;
        short *S = p;
        int r, W = w;
        int c, H = h;

        /* Read the full-resolution data set. */

        if ((fp = fopen(filename, "rb")))
        {
            fread(p, sizeof (short), w * h, fp);
            fclose(fp);
        }

        /* Generate box-filtered mipmaps. */

        while (W > 0 && H > 0 && (W & 1) == 0 && (H & 1) == 0)
        {
            D += W * H;
            W /= 2;
            H /= 2;

            for (r = 0; r < H; ++r)
                for (c = 0; c < W; ++c)
                {
                    int ka = (int) S[(r * 2    ) * W * 2 + (c * 2    )];
                    int kb = (int) S[(r * 2    ) * W * 2 + (c * 2 + 1)];
                    int kc = (int) S[(r * 2 + 1) * W * 2 + (c * 2    )];
                    int kd = (int) S[(r * 2 + 1) * W * 2 + (c * 2 + 1)];

                    D[r * W + c] = (short) ((ka + kb + kc + kd) / 4);
                }

            S = D;
        }
    }

    return p;
}

/*===========================================================================*/

int send_create_terrain(const char *filename, int w, int h, int n)
{
    int l = strlen(filename);
    int i;

    if ((i = new_terrain()))
    {
        /* If the file exists and is successfully loaded... */

        if ((terrain[i].p = load_terrain(filename, w, h)))
        {
            terrain[i].count = 1;
            terrain[i].w     = w;
            terrain[i].h     = h;
            terrain[i].n     = n;
            terrain[i].o     = 3396000;
            terrain[i].o     = 8;

            /* Pack the header and data. */

            send_event(EVENT_CREATE_TERRAIN);
            send_value(terrain[i].w);
            send_value(terrain[i].h);
            send_value(terrain[i].n);

            send_value(l);
            send_array(filename, l + 1, 1);

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
        char filename[MAXSTR];

        int w = recv_value();
        int h = recv_value();
        int n = recv_value();
        int l = recv_value();

        recv_array(filename, l + 1, 1);

        /* HACK: Clients load data from disk instead of broadcast. */

        if ((terrain[i].p = load_terrain(filename, w, h)))
        {
            terrain[i].count = 1;
            terrain[i].w     = w;
            terrain[i].h     = h;
            terrain[i].n     = n;

            /* Encapsulate this object in an entity. */

            recv_create_entity();
        }
    }
}

/*===========================================================================*/

static void init_terrain(int i)
{
    if (terrain[i].state == 0)
    {
        struct vertex *scratch = init_scratch(terrain[i].n);
        GLushort      *indices = init_indices(terrain[i].n);

        terrain[i].vbo = init_vbo(terrain[i].n, scratch);
        terrain[i].ibo = init_ibo(terrain[i].n, indices);

        free(indices);
        free(scratch);

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
#ifdef SNIP
static void draw_data(GLuint vbo, GLuint ibo, GLsizei n)
{
    GLsizei N = (n - 1) * n * 2;
    GLsizei S = sizeof (struct vertex);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB,         vbo);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ibo);

    glVertexPointer  (3, GL_FLOAT, S, OFFSET(0));
    glNormalPointer  (   GL_FLOAT, S, OFFSET(12));
    glTexCoordPointer(2, GL_FLOAT, S, OFFSET(24));

    /* Draw filled polygons. */

    glEnable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLE_STRIP, N, GL_UNSIGNED_SHORT, OFFSET(0));

    /* Draw wire frame. */

    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glColor3f(1.0f, 1.0f, 0.0f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLE_STRIP, N, GL_UNSIGNED_SHORT, OFFSET(0));
}
#endif
/*---------------------------------------------------------------------------*/

static float len(const float a[3], const float b[3])
{
    return sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]));
}

static float get_value(float v[4][3])
{
    float km;

    float k0 = len(v[0], v[1]);
    float k1 = len(v[1], v[2]);
    float k2 = len(v[2], v[3]);
    float k3 = len(v[3], v[0]);

    km = MAX(k1, k0);
    km = MAX(k2, km);
    km = MAX(k3, km);

    return km * 2.0f;
}

static int is_visible(float v[4][3])
{
    const float k = 1.0f;

    /* This must use the AABB of the verts, not the verts themselves. */

    if (v[0][0] < -k && v[1][0] < -k && v[2][0] < -k && v[3][0] < -k) return 0;
    if (v[0][1] < -k && v[1][1] < -k && v[2][1] < -k && v[3][1] < -k) return 0;

    if (v[0][0] >  k && v[1][0] >  k && v[2][0] >  k && v[3][0] >  k) return 0;
    if (v[0][1] >  k && v[1][1] >  k && v[2][1] >  k && v[3][1] >  k) return 0;

    return 1;
}

static int count;

static void draw_page(int i, float x, float y, float w, float h)
{
    float v[4][4], n[3];

    get_vertex(v[0], x,     y,     terrain[i].o);
    get_vertex(v[1], x + w, y,     terrain[i].o);
    get_vertex(v[2], x + w, y + h, terrain[i].o);
    get_vertex(v[3], x,     y + h, terrain[i].o);

    if (y == 0)
        get_normal(n, v[0], v[2], v[3]);
    else
        get_normal(n, v[3], v[0], v[1]);

/*    glBegin(GL_QUADS);*/
    glBegin(GL_LINE_LOOP);
    {
        glNormal3fv(n);

        glVertex3fv(v[0]);
        glVertex3fv(v[1]);
        glVertex3fv(v[2]);
        glVertex3fv(v[3]);
    }
    glEnd();

    count++;
}

/*---------------------------------------------------------------------------*/

#define MAXPAGE 4096

struct page
{
    float x;
    float y;
    float w;
    float h;
};

static struct page queue[MAXPAGE];
static int         queue_b;
static int         queue_e;

static void enqueue_page(float x, float y, float w, float h)
{
    int queue_n = (queue_b + 1) % MAXPAGE;

    if (queue_n != queue_e)
    {
        queue[queue_b].x = x;
        queue[queue_b].y = y;
        queue[queue_b].w = w;
        queue[queue_b].h = h;

        queue_b = queue_n;
    }
/*  else fprintf(stderr, "terrain page queue overflow\n"); */
}

static int dequeue_page(float *x, float *y, float *w, float *h)
{
    if (queue_e != queue_b)
    {
        *x = queue[queue_e].x;
        *y = queue[queue_e].y;
        *w = queue[queue_e].w;
        *h = queue[queue_e].h;

        queue_e = (queue_e + 1) % MAXPAGE;

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static void do_stuff(const float M[16], int i, float p, float t)
{
    float v[4][3], x, y, w, h, k;

    while (dequeue_page(&x, &y, &w, &h))
    {
        get_bounds(v, M, x, y, w, h, terrain[i].o);

        if (is_visible(v) && (k = get_value(v)) > 0.0f)
        {
            if (k < 1.0f)
                glColor4f(0.8f, 0.8f, 0.8f, (k - 0.5f) * 2.0f);
            else
                glColor4f(0.8f, 0.8f, 0.8f, 1.0f);

            draw_page(i, x, y, w, h);

            if (k > 1.0f && w > 1.0f)
            {
                float xm = x + w / 2;
                float ym = y + h / 2;

                float x0 = (t < xm) ? x : xm;
                float x1 = (t < xm) ? xm : x;
                float y0 = (p < ym) ? y : ym;
                float y1 = (p < ym) ? ym : y;

                enqueue_page(x0, y0, w / 2, h / 2);
                enqueue_page(x1, y0, w / 2, h / 2);
                enqueue_page(x0, y1, w / 2, h / 2);
                enqueue_page(x1, y1, w / 2, h / 2);
            }
        }
    }
}

static void draw_terrain(int i, int j, int f, float a)
{
    init_terrain(i);

    glPushMatrix();
    {
        float P[16], M[16], X[16], v[3];
        float t;
        float p;

        /* Apply the local coordinate system transformation. */

        transform_entity(j);

        /* Acquire the model-view-projection matrix. */

        glGetFloatv(GL_PROJECTION_MATRIX, P);
        glGetFloatv(GL_MODELVIEW_MATRIX,  M);

        mult_mat_mat(X, P, M);

        get_viewpoint(v);
        normalize(v);

        t = DEG(atan2(v[0], v[2]));
        p = DEG(acos(-v[1]));

        if (t < 0) t += 360.0f;

        /* Draw. */

        glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        {
/*
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
*/
            glEnable(GL_COLOR_MATERIAL);
            glEnable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glDisable(GL_DEPTH_TEST);
/*
            enqueue_page(  0.0f,  0.0f, 90.0f, 90.0f);
            enqueue_page( 90.0f,  0.0f, 90.0f, 90.0f);
            enqueue_page(180.0f,  0.0f, 90.0f, 90.0f);
            enqueue_page(270.0f,  0.0f, 90.0f, 90.0f);
*/
            enqueue_page(  0.0f, 90.0f, 90.0f, 90.0f);
/*
            enqueue_page( 90.0f, 90.0f, 90.0f, 90.0f);
            enqueue_page(180.0f, 90.0f, 90.0f, 90.0f);
            enqueue_page(270.0f, 90.0f, 90.0f, 90.0f);
*/
            count = 0;
            do_stuff(X, i, p, t);
            printf("%d\n", count);

/*
            glBindBufferARB(GL_ARRAY_BUFFER_ARB,         0);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
*/
        }
        glPopClientAttrib();
        glPopAttrib();

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

            if (terrain[i].p) free(terrain[i].p);

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
