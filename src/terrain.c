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

static int count;

/*---------------------------------------------------------------------------*/

struct vertex
{
    float v[3];
    float n[3];
    float t[2];
};

struct page
{
    float x;
    float y;
    float w;
    float h;

    GLuint vbo;

    int next;
    int prev;
};

struct terrain
{
    int count;
    int state;

    short *p;
    int    w;
    int    h;
    int    n;
    int    m;
    int    o;
    float  bias;

    GLuint ibo;

    int head;
    int tail;

    struct page   *cache;
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

static void get_bounds(float b[6], float x, float y, float w, float h, float r)
{
    float u[4][4];

    get_vertex(u[0], x,     y,     r);
    get_vertex(u[1], x + w, y,     r);
    get_vertex(u[2], x + w, y + h, r);
    get_vertex(u[3], x,     y + h, r);

    b[0] = MIN(MIN(u[0][0], u[1][0]), MIN(u[2][0], u[3][0]));
    b[1] = MIN(MIN(u[0][1], u[1][1]), MIN(u[2][1], u[3][1]));
    b[2] = MIN(MIN(u[0][2], u[1][2]), MIN(u[2][2], u[3][2]));
    b[3] = MAX(MAX(u[0][0], u[1][0]), MAX(u[2][0], u[3][0]));
    b[4] = MAX(MAX(u[0][1], u[1][1]), MAX(u[2][1], u[3][1]));
    b[5] = MAX(MAX(u[0][2], u[1][2]), MAX(u[2][2], u[3][2]));
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

static int search_cache(int i, float x, float y, float w, float h)
{
    int j;

    for (j = terrain[i].head; j >= 0; j = terrain[i].cache[j].next)
        if (terrain[i].cache[j].x == x &&
            terrain[i].cache[j].y == y &&
            terrain[i].cache[j].w == w &&
            terrain[i].cache[j].h == h)
            return j;

    return -1;
}

static struct page *init_cache(int n, int m)
{
    struct page *buf;

    if ((buf = (struct page *) malloc(m * sizeof (struct page))))
    {
        int i;

        for (i = 0; i < m; ++i)
        {
            buf[i].x = 0.0f;
            buf[i].y = 0.0f;
            buf[i].w = 0.0f;
            buf[i].h = 0.0f;

            buf[i].vbo = init_vbo(n, NULL);

            buf[i].next = (i == m - 1) ? -1 : i + 1;
            buf[i].prev = (i == 0    ) ? -1 : i - 1;
        }
    }

    return buf;
}

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

static void load_scratch(int i, float x, float y, float w, float h)
{
    int j, r, c, n = terrain[i].n;

    struct vertex *buf = terrain[i].scratch;

    for (j = 0, r = 0; r < n; ++r)
        for (c = 0; c < n; ++c, ++j)
        {
            float X = x + w * c / n;
            float Y = y + h * r / n;

            float v[4];

            get_vertex(v, X, Y, terrain[i].o);

            buf[j].v[0] = v[0];
            buf[j].v[1] = v[1];
            buf[j].v[2] = v[2];

            buf[j].n[0] = 0.0f;
            buf[j].n[1] = 1.0f;
            buf[j].n[2] = 0.0f;

            buf[j].t[0] = X / 360.0f;
            buf[j].t[1] = Y / 180.0f;
        }
}

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
            terrain[i].m     = 128;
            terrain[i].o     = 3396000;
            terrain[i].bias  = 0.5f;

            /* Pack the header and data. */

            send_event(EVENT_CREATE_TERRAIN);
            send_value(terrain[i].w);
            send_value(terrain[i].h);
            send_value(terrain[i].n);
            send_value(terrain[i].m);

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
        int m = recv_value();
        int l = recv_value();

        recv_array(filename, l + 1, 1);

        /* HACK: Clients load data from disk instead of broadcast. */

        if ((terrain[i].p = load_terrain(filename, w, h)))
        {
            terrain[i].count = 1;
            terrain[i].w     = w;
            terrain[i].h     = h;
            terrain[i].n     = n;
            terrain[i].m     = m;

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
        GLushort *indices  = init_indices(terrain[i].n);
        terrain[i].scratch = init_scratch(terrain[i].n);

        terrain[i].ibo   = init_ibo  (terrain[i].n, indices);
        terrain[i].cache = init_cache(terrain[i].n, terrain[i].m);

        terrain[i].head = 0;
        terrain[i].tail = terrain[i].m - 1;

        free(indices);

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

static void draw_data(GLuint vbo, GLsizei n)
{
    GLsizei N = (n - 1) * n * 2;
    GLsizei S = sizeof (struct vertex);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);

    glVertexPointer  (3, GL_FLOAT, S, OFFSET(0));
    glNormalPointer  (   GL_FLOAT, S, OFFSET(12));
    glTexCoordPointer(2, GL_FLOAT, S, OFFSET(24));

    /* Draw filled polygons. */
/*
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLE_STRIP, N, GL_UNSIGNED_SHORT, OFFSET(0));
*/
    /* Draw wire frame. */
/*
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glColor3f(1.0f, 1.0f, 0.0f);
*/
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLE_STRIP, N, GL_UNSIGNED_SHORT, OFFSET(0));
}

/*---------------------------------------------------------------------------*/

static float get_value(float b[6], const float M[16], float bias)
{
    float u[8][4];
    float v[8][4];
    float c[4];
    float k;

    u[0][0] = b[0]; u[0][1] = b[1]; u[0][2] = b[2]; u[0][3] = 1.0f;
    u[1][0] = b[3]; u[1][1] = b[1]; u[1][2] = b[2]; u[1][3] = 1.0f;
    u[2][0] = b[0]; u[2][1] = b[4]; u[2][2] = b[2]; u[2][3] = 1.0f;
    u[3][0] = b[3]; u[3][1] = b[4]; u[3][2] = b[2]; u[3][3] = 1.0f;
    u[4][0] = b[0]; u[4][1] = b[1]; u[4][2] = b[5]; u[4][3] = 1.0f;
    u[5][0] = b[3]; u[5][1] = b[1]; u[5][2] = b[5]; u[5][3] = 1.0f;
    u[6][0] = b[0]; u[6][1] = b[4]; u[6][2] = b[5]; u[6][3] = 1.0f;
    u[7][0] = b[3]; u[7][1] = b[4]; u[7][2] = b[5]; u[7][3] = 1.0f;

    mult_mat_vec(v[0], M, u[0]);
    mult_mat_vec(v[1], M, u[1]);
    mult_mat_vec(v[2], M, u[2]);
    mult_mat_vec(v[3], M, u[3]);
    mult_mat_vec(v[4], M, u[4]);
    mult_mat_vec(v[5], M, u[5]);
    mult_mat_vec(v[6], M, u[6]);
    mult_mat_vec(v[7], M, u[7]);

    v[0][0] /= v[0][3];
    v[1][0] /= v[1][3];
    v[2][0] /= v[2][3];
    v[3][0] /= v[3][3];
    v[4][0] /= v[4][3];
    v[5][0] /= v[5][3];
    v[6][0] /= v[6][3];
    v[7][0] /= v[7][3];

    v[0][1] /= v[0][3];
    v[1][1] /= v[1][3];
    v[2][1] /= v[2][3];
    v[3][1] /= v[3][3];
    v[4][1] /= v[4][3];
    v[5][1] /= v[5][3];
    v[6][1] /= v[6][3];
    v[7][1] /= v[7][3];

    c[0] = MIN(v[1][0], v[0][0]);
    c[0] = MIN(v[2][0], c[0]);
    c[0] = MIN(v[3][0], c[0]);
    c[0] = MIN(v[4][0], c[0]);
    c[0] = MIN(v[5][0], c[0]);
    c[0] = MIN(v[6][0], c[0]);
    c[0] = MIN(v[7][0], c[0]);

    c[1] = MIN(v[1][1], v[0][1]);
    c[1] = MIN(v[2][1], c[1]);
    c[1] = MIN(v[3][1], c[1]);
    c[1] = MIN(v[4][1], c[1]);
    c[1] = MIN(v[5][1], c[1]);
    c[1] = MIN(v[6][1], c[1]);
    c[1] = MIN(v[7][1], c[1]);

    c[2] = MAX(v[1][0], v[0][0]);
    c[2] = MAX(v[2][0], c[2]);
    c[2] = MAX(v[3][0], c[2]);
    c[2] = MAX(v[4][0], c[2]);
    c[2] = MAX(v[5][0], c[2]);
    c[2] = MAX(v[6][0], c[2]);
    c[2] = MAX(v[7][0], c[2]);

    c[3] = MAX(v[1][1], v[0][1]);
    c[3] = MAX(v[2][1], c[3]);
    c[3] = MAX(v[3][1], c[3]);
    c[3] = MAX(v[4][1], c[3]);
    c[3] = MAX(v[5][1], c[3]);
    c[3] = MAX(v[6][1], c[3]);
    c[3] = MAX(v[7][1], c[3]);

    k = MAX(fabs(c[0] - c[2]), fabs(c[1] - c[3])) * bias;

    return k;
}

static void draw_page(int i, float x, float y, float w, float h)
{
    int j;

    /* Search for this page in the page cache. */

    if ((j = search_cache(i, x, y, w, h)) == -1)
    {
        struct page *cache = terrain[i].cache;

        int prev = cache[terrain[i].tail].prev;

        /* This is a new page.  Promote the oldest cache line. */

        cache[cache[terrain[i].tail].prev].next = -1;

        cache[terrain[i].head].prev = terrain[i].tail;
        cache[terrain[i].tail].next = terrain[i].head;
        cache[terrain[i].tail].prev = -1;

        terrain[i].head = terrain[i].tail;
        terrain[i].tail = prev;

        /* Initialize this cache line's VBO to the given area. */

        j = terrain[i].head;

        printf("cache %3d gets %8.3f %8.3f %8.3f %8.3f\n", j, x, y, w, h);

        load_scratch(i, x, y, w, h);

        terrain[i].cache[j].x = x;
        terrain[i].cache[j].y = y;
        terrain[i].cache[j].w = w;
        terrain[i].cache[j].h = h;

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, terrain[i].cache[j].vbo);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        terrain[i].n * terrain[i].n * sizeof (struct vertex),
                        terrain[i].scratch, GL_STATIC_DRAW_ARB);
    }

    draw_data(terrain[i].cache[j].vbo, terrain[i].n);

    count++;
}

/*---------------------------------------------------------------------------*/

#define MAXAREA 4096

struct area
{
    float x;
    float y;
    float w;
    float h;
    int   d;
};

static struct area queue[MAXAREA];
static int         queue_b;
static int         queue_e;

static void enqueue_area(float x, float y, float w, float h, int d)
{
    int queue_n = (queue_b + 1) % MAXAREA;

    if (queue_n != queue_e)
    {
        queue[queue_b].x = x;
        queue[queue_b].y = y;
        queue[queue_b].w = w;
        queue[queue_b].h = h;
        queue[queue_b].d = d;

        queue_b = queue_n;
    }
    else fprintf(stderr, "terrain area queue overflow\n");
}

static int dequeue_area(float *x, float *y, float *w, float *h, int *d)
{
    if (queue_e != queue_b)
    {
        *x = queue[queue_e].x;
        *y = queue[queue_e].y;
        *w = queue[queue_e].w;
        *h = queue[queue_e].h;
        *d = queue[queue_e].d;

        queue_e = (queue_e + 1) % MAXAREA;

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static void draw_areas(float V[6][4], const float M[16],
                       int i, float p, float t)
{
    float b[6], x, y, w, h;
    int d;

    while (dequeue_area(&x, &y, &w, &h, &d))
    {
        get_bounds(b, x, y, w, h, terrain[i].o);

        if (test_frustum(V, b) >= 0)
        {
            float k = get_value(b, M, terrain[i].bias);

            if (d > 0 && k < 1.0f)
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

                enqueue_area(x0, y0, w / 2, h / 2, d + 1);
                enqueue_area(x1, y0, w / 2, h / 2, d + 1);
                enqueue_area(x0, y1, w / 2, h / 2, d + 1);
                enqueue_area(x1, y1, w / 2, h / 2, d + 1);
            }
        }
    }
}

static void draw_terrain(int i, int j, int f, float a)
{
    init_terrain(i);

    glPushMatrix();
    {
        float V[6][4], P[16], M[16], X[16], v[3];
        float t;
        float p;

        /* Apply the local coordinate system transformation. */

        transform_entity(j);

        /* Acquire the model-view-projection matrix. */

        glGetFloatv(GL_PROJECTION_MATRIX, P);
        glGetFloatv(GL_MODELVIEW_MATRIX,  M);

        mult_mat_mat(X, P, M);

        get_viewfrust(V);
        get_viewpoint(v);

        /* Set the far clipping plane to the center of the planet.  Neat! */

        V[5][3] = 0.0f;

        /* Determine the viewpoint in cylindrical coordinates. */

        normalize(v);

        t = DEG(atan2(v[0], v[2]));
        p = DEG(acos(-v[1]));

        if (t < 0) t += 360.0f;

        /* Draw. */

        glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        {
            float y0 = (p < 90.0f) ?  0.0f : 90.0f;
            float y1 = (p < 90.0f) ? 90.0f :  0.0f;

            float x0 = fmod(floor(t / 90.0f) * 90.0f + 180.0f, 360.0f);
            float x1 = fmod(floor(t / 90.0f) * 90.0f +  90.0f, 360.0f);
            float x2 = fmod(floor(t / 90.0f) * 90.0f + 270.0f, 360.0f);
            float x3 = fmod(floor(t / 90.0f) * 90.0f +   0.0f, 360.0f);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, terrain[i].ibo);

            glEnable(GL_COLOR_MATERIAL);
            glEnable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

/*          glDisable(GL_DEPTH_TEST); */

            enqueue_area(x0, y0, 90.0f, 90.0f, 0);
            enqueue_area(x1, y0, 90.0f, 90.0f, 0);
            enqueue_area(x2, y0, 90.0f, 90.0f, 0);
            enqueue_area(x3, y0, 90.0f, 90.0f, 0);
            enqueue_area(x0, y1, 90.0f, 90.0f, 0);
            enqueue_area(x1, y1, 90.0f, 90.0f, 0);
            enqueue_area(x2, y1, 90.0f, 90.0f, 0);
            enqueue_area(x3, y1, 90.0f, 90.0f, 0);

            count = 0;
            draw_areas(V, X, i, p, t);
            printf("%d\n", count);

            glBindBufferARB(GL_ARRAY_BUFFER_ARB,         0);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
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
