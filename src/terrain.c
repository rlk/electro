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

#define DEFAULT_SIZE 2048
#define DEFAULT_BIAS 0.5f
#define DEFAULT_MAGN 2.0f

/* NOTE: This code includes a great many hard-coded values that apply only   */
/* to the Mars MOLA height map and color map.  This should be generalized,   */
/* but probably won't be, as it is made obsolete by another application.     */

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
    float a;

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
    float  magn;

    GLuint ibo;
    GLuint tex[4][2];

    int head;
    int tail;

    short *min;
    short *max;

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

static void get_vertex(float v[3], double x, double y, double r)
{
    v[0] = (float) (sin(RAD(x)) * sin(RAD(y)) * r);
    v[1] = (float) (             -cos(RAD(y)) * r);
    v[2] = (float) (cos(RAD(x)) * sin(RAD(y)) * r);
}

static void get_normal(float n[3], float v[3][3])
{
    float d[2][3];

    d[0][0] = v[1][0] - v[0][0];
    d[0][1] = v[1][1] - v[0][1];
    d[0][2] = v[1][2] - v[0][2];
    
    d[1][0] = v[2][0] - v[0][0];
    d[1][1] = v[2][1] - v[0][1];
    d[1][2] = v[2][2] - v[0][2];
    
    cross(n, d[0], d[1]);

    normalize(n);
}

static float get_bounds(int i, float b[6], float x, float y, float a)
{
    float u[8][3];
    int j;

    int cx = (int) (x + a / 2);
    int cy = (int) (y + a / 2);

    short min = terrain[i].min[360 * cy + cx];
    short max = terrain[i].max[360 * cy + cx];

    get_vertex(u[0], x,     y,     terrain[i].o + min * terrain[i].magn);
    get_vertex(u[1], x + a, y,     terrain[i].o + min * terrain[i].magn);
    get_vertex(u[2], x + a, y + a, terrain[i].o + min * terrain[i].magn);
    get_vertex(u[3], x,     y + a, terrain[i].o + min * terrain[i].magn);
    get_vertex(u[4], x,     y,     terrain[i].o + max * terrain[i].magn);
    get_vertex(u[5], x + a, y,     terrain[i].o + max * terrain[i].magn);
    get_vertex(u[6], x + a, y + a, terrain[i].o + max * terrain[i].magn);
    get_vertex(u[7], x,     y + a, terrain[i].o + max * terrain[i].magn);

    b[0] = b[3] = u[0][0];
    b[1] = b[4] = u[0][1];
    b[2] = b[5] = u[0][2];

    for (j = 1; j < 8; ++j)
    {
        b[0] = MIN(b[0], u[j][0]);
        b[1] = MIN(b[1], u[j][1]);
        b[2] = MIN(b[2], u[j][2]);
        b[3] = MAX(b[3], u[j][0]);
        b[4] = MAX(b[4], u[j][1]);
        b[5] = MAX(b[5], u[j][2]);
    }

    return (float) sin(RAD(cy));
}

/*---------------------------------------------------------------------------*/

static GLuint init_vbo(int n, const struct vertex *data)
{
    GLuint vbo;
    GLsizei N = (n + 1) * (n + 1) * sizeof (struct vertex);

    glGenBuffersARB(1, &vbo);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, N, data, GL_STATIC_DRAW_ARB);

    opengl_check("init_vbo");
    return vbo;
}

static GLuint init_ibo(int n, const GLushort *data)
{
    GLuint ibo;
    GLsizei N = n * (n + 1) * 2 * sizeof (GLushort);

    glGenBuffersARB(1, &ibo);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ibo);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, N, data, GL_STATIC_DRAW_ARB);

    opengl_check("init_ibo");
    return ibo;
}

static GLuint init_texture(const char *filename, int w, int h)
{
    GLuint   tex = 0;
    GLubyte *buf;
    FILE    *fp;

    if ((buf = (GLubyte *) malloc(8 * (w / 4) * (h / 4))))
    {
        if ((fp = fopen(filename, "rb")))
        {
            size_t sz = 8 * (w / 4) * (h / 4);

            if (fread(buf, 1, sz, fp) < sz)
                error("'%s' load truncated", filename);

            fclose(fp);
        }

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR);

        glTexParameterf(GL_TEXTURE_1D, GL_GENERATE_MIPMAP, GL_TRUE);

        glCompressedTexImage2DARB(GL_TEXTURE_2D, 0,
                                  GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                  w, h, 0, 8 * (w / 4) * (h / 4), buf);

        free(buf);
    }

    return tex;
}

/*---------------------------------------------------------------------------*/

static int search_cache(int i, float x, float y, float a)
{
    int j;

    for (j = terrain[i].head; j >= 0; j = terrain[i].cache[j].next)
        if (terrain[i].cache[j].x == x &&
            terrain[i].cache[j].y == y &&
            terrain[i].cache[j].a == a)
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
            buf[i].a = 0.0f;

            buf[i].vbo = init_vbo(n, NULL);

            buf[i].next = (i == m - 1) ? -1 : i + 1;
            buf[i].prev = (i == 0    ) ? -1 : i - 1;
        }
    }

    return buf;
}

static struct vertex *init_scratch(int n)
{
    size_t sz = (n + 1) * (n + 1) * sizeof (struct vertex);

    struct vertex *buf;

    if ((buf = (struct vertex *) malloc(sz)))
    {
        int i, r, c;

        for (i = 0, r = 0; r <= n; ++r)
            for (c = 0; c <= n; ++c, ++i)
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
    size_t sz = n * (n + 1) * 2 * sizeof (GLushort);

    GLushort *buf;

    if ((buf = (GLushort *) malloc(sz)))
    {
        int i, r, c;

        for (i = 0, r = 0; r < n; ++r)
            if ((r & 1) == 0)
            {
                for (c = 0; c <= n; ++c)  /* Even-numbered row. */
                {
                    GLushort A = (r + 1) * (n + 1) + c;
                    GLushort B = (r    ) * (n + 1) + c;

                    buf[i++] = A;
                    buf[i++] = B;
                }
            }
            else
            {
                for (c = 0; c <= n; ++c)  /* Odd-numbered row. */
                {
                    GLushort A = (r    ) * (n + 1) + (n - c);
                    GLushort B = (r + 1) * (n + 1) + (n - c);

                    buf[i++] = A;
                    buf[i++] = B;
                }
            }
    }

    return buf;
}

/*---------------------------------------------------------------------------*/

static void load_scratch(int i, float x, float y, float a)
{
    int j, r, c;

    struct vertex *buf = terrain[i].scratch;

    /* Determine the proper mipmap level for an area of the given size. */

    short *P = terrain[i].p;
    int    W = terrain[i].w;
    int    H = terrain[i].h;
    int    d = 0;

    float s0 = (float) (fmod(x, 90.0f) / 90.0);
    float t0 = (float) (fmod(y, 90.0f) / 90.0);

    while ((360.0f / W) < (a / terrain[i].n))
    {
        P += W * H;
        W /= 2;
        H /= 2;
        d += 1;
    }

    /* Compute vertex positions from raw data.*/

    for (j = 0, r = 0; r <= terrain[i].n; ++r)
        for (c = 0; c <= terrain[i].n; ++c, ++j)
        {
            float da = a / terrain[i].n;

            float X = x + da * c;
            float Y = y + da * r;
            float s = terrain[i].magn;

            int cc = MIN(W - 1, (int) (X * W / 360.0f));
            int rr = MIN(H - 1, (int) (Y * H / 180.0f));

            int cR = (cc + 1 >= W) ? (cc + 1 - W) : (cc + 1);
            int rT = (rr + 1 >= H) ?        H - 1 : (rr + 1);

            double r0 = terrain[i].o + s * P[rr * W + cc];
            double r1 = terrain[i].o + s * P[rr * W + cR];
            double r2 = terrain[i].o + s * P[rT * W + cc];

            float v[3][3];
            float n[3];

            get_vertex(v[0], X, Y, r0);

            if (Y > 178.0f)
            {
                n[0] =  0.0f;
                n[1] = +1.0f;
                n[2] =  0.0f;
            }
            else if (Y < 2.0f)
            {
                n[0] =  0.0f;
                n[1] = -1.0f;
                n[2] =  0.0f;
            }
            else
            {
                get_vertex(v[1], X + da, Y,      r1);
                get_vertex(v[2], X,      Y + da, r2);
                get_normal(n, v);
            }

            buf[j].v[0] = v[0][0];
            buf[j].v[1] = v[0][1];
            buf[j].v[2] = v[0][2];

            buf[j].n[0] = n[0];
            buf[j].n[1] = n[1];
            buf[j].n[2] = n[2];

            buf[j].t[0] =        (s0 + da * c / 90.0f);
            buf[j].t[1] = 1.0f - (t0 + da * r / 90.0f);
        }
}

int load_terrain(int i, const char *filename, int w, int h)
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
            size_t sz = w * h;

            if (fread(p, sizeof (short), sz, fp) < sz)
                error("'%s' load truncated", filename);

            fclose(fp);
        }

        /* Generate mipmaps. */

        while (W > 0 && H > 0 && (W & 1) == 0 && (H & 1) == 0)
        {
            D += W * H;
            W /= 2;
            H /= 2;

            for (r = 0; r < H; ++r)
                for (c = 0; c < W; ++c)
                {
#ifdef MINFILTER
                    short ka = S[(r * 2    ) * W * 2 + (c * 2    )];
                    short kb = S[(r * 2    ) * W * 2 + (c * 2 + 1)];
                    short kc = S[(r * 2 + 1) * W * 2 + (c * 2    )];
                    short kd = S[(r * 2 + 1) * W * 2 + (c * 2 + 1)];

                    D[r * W + c] = MIN(MIN(ka, kb), MIN(kc, kd));
#else
                    int ka = (int) S[(r * 2    ) * W * 2 + (c * 2    )];
                    int kb = (int) S[(r * 2    ) * W * 2 + (c * 2 + 1)];
                    int kc = (int) S[(r * 2 + 1) * W * 2 + (c * 2    )];
                    int kd = (int) S[(r * 2 + 1) * W * 2 + (c * 2 + 1)];

                    D[r * W + c] = (short) ((ka + kb + kc + kd) / 4);
#endif
                }

            S = D;
        }

        terrain[i].p = p;

        /* Compute the min and max caches. */

        if ((terrain[i].min = (short *) malloc(360 * 180 * sizeof (short))) &&
            (terrain[i].max = (short *) malloc(360 * 180 * sizeof (short))))
        {
            for (r = 0; r < 180; ++r)
                for (c = 0; c < 360; ++c)
                {
                    terrain[i].min[360 * r + c] =  32767;
                    terrain[i].max[360 * r + c] = -32768;
                }

            for (r = 0; r < h; ++r)
                for (c = 0; c < w; ++c)
                {
                    int R = 180 * r / h;
                    int C = 360 * c / w;

                    short h = terrain[i].p[r * w + c];

                    if (terrain[i].min[360 * R + C] >= h)
                        terrain[i].min[360 * R + C]  = h;
                    if (terrain[i].max[360 * R + C] <= h)
                        terrain[i].max[360 * R + C]  = h;
                }
        }

        return 1;
    }
    return 0;
}

/*===========================================================================*/

int send_create_terrain(const char *filename, int w, int h, int n)
{
    int l = strlen(filename);
    int i;

    if ((i = new_terrain()))
    {
        /* If the file exists and is successfully loaded... */

        if (load_terrain(i, filename, w, h))
        {
            terrain[i].count = 1;
            terrain[i].w     = w;
            terrain[i].h     = h;
            terrain[i].n     = n;
            terrain[i].m     = DEFAULT_SIZE;
            terrain[i].o     = 3396000;
            terrain[i].bias  = DEFAULT_BIAS;
            terrain[i].magn  = DEFAULT_MAGN;

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

        if (load_terrain(i, filename, w, h))
        {
            terrain[i].count = 1;
            terrain[i].w     = w;
            terrain[i].h     = h;
            terrain[i].n     = n;
            terrain[i].m     = m;
            terrain[i].o     = 3396000;
            terrain[i].bias  = DEFAULT_BIAS;
            terrain[i].magn  = DEFAULT_MAGN;

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
        GLushort *indices;

        indices    = init_indices(terrain[i].n);
        terrain[i].ibo = init_ibo(terrain[i].n, indices);
        free(indices);

        terrain[i].scratch = init_scratch(terrain[i].n);
        terrain[i].cache   = init_cache  (terrain[i].n, terrain[i].m);

        terrain[i].tail    = terrain[i].m - 1;
        terrain[i].head    = 0;
        terrain[i].state   = 1;

        terrain[i].tex[0][0] = init_texture("mars2.dxt", 4096, 4096);
        terrain[i].tex[1][0] = init_texture("mars3.dxt", 4096, 4096);
        terrain[i].tex[2][0] = init_texture("mars0.dxt", 4096, 4096);
        terrain[i].tex[3][0] = init_texture("mars1.dxt", 4096, 4096);
        terrain[i].tex[0][1] = init_texture("mars6.dxt", 4096, 4096);
        terrain[i].tex[1][1] = init_texture("mars7.dxt", 4096, 4096);
        terrain[i].tex[2][1] = init_texture("mars4.dxt", 4096, 4096);
        terrain[i].tex[3][1] = init_texture("mars5.dxt", 4096, 4096);
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
    GLsizei N = n * (n + 1) * 2;
    GLsizei S = sizeof (struct vertex);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);

    glVertexPointer  (3, GL_FLOAT, S, OFFSET(0));
    glNormalPointer  (   GL_FLOAT, S, OFFSET(12));
    glTexCoordPointer(2, GL_FLOAT, S, OFFSET(24));

    /* Draw filled polygons. */

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

/*  k = MAX(fabs(c[0] - c[2]), fabs(c[1] - c[3])) * bias; */
    k = 0.5f * (float) (fabs(c[0] - c[2]) + fabs(c[1] - c[3])) * bias;

    return k;
}

static void draw_page(int i, float x, float y, float a)
{
    int j, n = terrain[i].n;

    /* Search for this page in the page cache. */

    if ((j = search_cache(i, x, y, a)) == -1)
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

        load_scratch(i, x, y, a);

        terrain[i].cache[j].x = x;
        terrain[i].cache[j].y = y;
        terrain[i].cache[j].a = a;

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, terrain[i].cache[j].vbo);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        (n + 1) * (n + 1) * sizeof (struct vertex),
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
    float a;
    GLuint o;
};

static struct area queue[MAXAREA];
static int         queue_b;
static int         queue_e;

static void enqueue_area(float x, float y, float a, GLuint o)
{
    int queue_n = (queue_b + 1) % MAXAREA;

    if (queue_n != queue_e)
    {
        queue[queue_b].x = x;
        queue[queue_b].y = y;
        queue[queue_b].a = a;
        queue[queue_b].o = o;

        queue_b = queue_n;
    }
    else fprintf(stderr, "terrain area queue overflow\n");
}

static int dequeue_area(float *x, float *y, float *a, GLuint *o)
{
    if (queue_e != queue_b)
    {
        *x = queue[queue_e].x;
        *y = queue[queue_e].y;
        *a = queue[queue_e].a;
        *o = queue[queue_e].o;

        queue_e = (queue_e + 1) % MAXAREA;

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static void draw_areas(float V[6][4], const float M[16],
                       int i, float p, float t)
{
    float b[6], x, y, a;
    GLuint o;

    while (dequeue_area(&x, &y, &a, &o))
    {
        float kk = get_bounds(i, b, x, y, a);

        if (test_frustum(V, b) >= 0)
        {
            float k = get_value(b, M, terrain[i].bias);

            if (k > 1.0f && a > 360.0f * terrain[i].n / terrain[i].w)
            {
                float xm = x + a / 2;
                float ym = y + a / 2;

                float x0 = (t > xm) ? x : xm;
                float x1 = (t > xm) ? xm : x;
                float y0 = (p > ym) ? y : ym;
                float y1 = (p > ym) ? ym : y;

                enqueue_area(x0, y0, a / 2, o);
                enqueue_area(x1, y0, a / 2, o);
                enqueue_area(x0, y1, a / 2, o);
                enqueue_area(x1, y1, a / 2, o);
            }
            else
            {
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glBindTexture(GL_TEXTURE_2D, o);

                draw_page(i, x, y, a);
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

        /* Set the far clipping plane to the center of the planet. */
        /* TODO: fix */

/*      V[5][3] = 0.0f; */

        /* Determine the viewpoint in cylindrical coordinates. */

        normalize(v);

        t = (float) DEG(atan2(v[0], v[2]));
        p = (float) DEG(acos(-v[1]));

        if (t < 0) t += 360.0f;

        /* Draw. */

        glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        {
            float y0 = (p > 90.0f) ?  0.0f : 90.0f;
            float y1 = (p > 90.0f) ? 90.0f :  0.0f;

            float x0 = (float) fmod(floor(t / 90.0f) * 90.0 + 180.0, 360.0);
            float x1 = (float) fmod(floor(t / 90.0f) * 90.0 +  90.0, 360.0);
            float x2 = (float) fmod(floor(t / 90.0f) * 90.0 + 270.0, 360.0);
            float x3 = (float) fmod(floor(t / 90.0f) * 90.0 +   0.0, 360.0);

            int t0 = (int) (y0 / 90.0f);
            int t1 = (int) (y1 / 90.0f);

            int s0 = (int) (x0 / 90.0f);
            int s1 = (int) (x1 / 90.0f);
            int s2 = (int) (x2 / 90.0f);
            int s3 = (int) (x3 / 90.0f);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glEnable(GL_COLOR_MATERIAL);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_CULL_FACE);

            enqueue_area(x0, y0, 90.0f, terrain[i].tex[s0][t0]);
            enqueue_area(x1, y0, 90.0f, terrain[i].tex[s1][t0]);
            enqueue_area(x2, y0, 90.0f, terrain[i].tex[s2][t0]);
            enqueue_area(x3, y0, 90.0f, terrain[i].tex[s3][t0]);
            enqueue_area(x0, y1, 90.0f, terrain[i].tex[s0][t1]);
            enqueue_area(x1, y1, 90.0f, terrain[i].tex[s1][t1]);
            enqueue_area(x2, y1, 90.0f, terrain[i].tex[s2][t1]);
            enqueue_area(x3, y1, 90.0f, terrain[i].tex[s3][t1]);

            count = 0;

            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, terrain[i].ibo);
            draw_areas(V, X, i, p, t);

/*          printf("%d\n", count); */

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
