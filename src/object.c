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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opengl.h"
#include "vector.h"
#include "matrix.h"
#include "buffer.h"
#include "entity.h"
#include "brush.h"
#include "event.h"
#include "utility.h"
#include "object.h"

static void init_object(int);
static void fini_object(int);

/*===========================================================================*/

struct vec2
{
    float u;
    float v;
};

struct vec3
{
    float x;
    float y;
    float z;
};

/*---------------------------------------------------------------------------*/

struct object_vert
{
    float v[3];
    float t[3];
    float b[3];
    float n[3];
    float u[2];
};

struct object_face
{
    int vi[3];
};

struct object_edge
{
    int vi[2];
};

struct object_mesh
{
    int brush;

    vector_t fv;
    vector_t ev;
};

struct object
{
    int count;
    int state;

    vector_t vv;
    vector_t mv;

    GLuint buffer;

    float aabb_cache[6];
    int   aabb_state;
};

static vector_t object;

/*---------------------------------------------------------------------------*/

static struct object *get_object(int i)
{
    return (struct object *) vecget(object, i);
}

static struct object_mesh *get_object_mesh(int i, int j)
{
    return (struct object_mesh *) vecget(get_object(i)->mv, j);
}

static struct object_vert *get_object_vert(int i, int j)
{
    return (struct object_vert *) vecget(get_object(i)->vv, j);
}

static struct object_face *get_object_face(int i, int j, int k)
{
    return (struct object_face *) vecget(get_object_mesh(i, j)->fv, k);
}

static struct object_edge *get_object_edge(int i, int j, int k)
{
    return (struct object_edge *) vecget(get_object_mesh(i, j)->ev, k);
}

static int new_object(void)
{
    int i, n = vecnum(object);

    for (i = 0; i < n; ++i)
        if (get_object(i)->count == 0)
            return i;

    return vecadd(object);
}

/*===========================================================================*/
/* OBJ loader caches                                                         */

static vector_t _uv;
static vector_t _nv;
static vector_t _vv;

static int uerr;
static int nerr;
static int verr;

/*---------------------------------------------------------------------------*/

static int read_face_indices(const char *line, int *vi, int *ui, int *ni)
{
    static char vert[MAXSTR];
    int n;

    *vi = 0;
    *ui = 0;
    *ni = 0;

    if (sscanf(line, "%s%n", vert, &n) >= 1)
    {
        if (sscanf(vert, "%d/%d/%d", vi, ui, ni) == 3) return n;
        if (sscanf(vert, "%d/%d",    vi, ui    ) == 2) return n;
        if (sscanf(vert, "%d//%d",   vi,     ni) == 2) return n;
        if (sscanf(vert, "%d",       vi        ) == 1) return n;
    }
    return 0;
}

static void read_face_vertices(vector_t vv, const char *line)
{
    const char *c = line;
    int dc;
    int vj, vc = vecnum(_vv);
    int uj, uc = vecnum(_uv);
    int nj, nc = vecnum(_nv);
    int i;

    /* Scan down the face string recording index set specifications. */

    while ((dc = read_face_indices(c, &vj, &uj, &nj)))
        if ((i = vecadd(vv)) >= 0)
        {
            struct object_vert *v = (struct object_vert *) vecget(vv, i);

            /* Convert a face index to a vector index. */

            int ui = (uj > 0) ? uj - 1 : uj + uc;
            int ni = (nj > 0) ? nj - 1 : nj + nc;
            int vi = (vj > 0) ? vj - 1 : vj + vc;

            /* Locate the indexed values in the vector caches. */

            struct vec2 *up = (0 <= ui && ui < uc) ? vecget(_uv, ui) : NULL;
            struct vec3 *np = (0 <= ni && ni < nc) ? vecget(_nv, ni) : NULL;
            struct vec3 *vp = (0 <= vi && vi < vc) ? vecget(_vv, vi) : NULL;

            /* Initialize the new vertex, defaulting on bad input. */

            v->u[0] = up ? up->u : 0.0f;
            v->u[1] = up ? up->v : 0.0f;

            v->n[0] = np ? np->x : 0.0f;
            v->n[1] = np ? np->y : 0.0f;
            v->n[2] = np ? np->z : 1.0f;

            v->v[0] = vp ? vp->x : 0.0f;
            v->v[1] = vp ? vp->y : 0.0f;
            v->v[2] = vp ? vp->z : 0.0f;

            /* Note bad indices. */

            if (uj && !up) uerr++;
            if (nj && !np) nerr++;
            if (vj && !vp) verr++;

            c += dc;
        }
}

static void read_f(vector_t vv, vector_t fv, const char *line)
{
    int i, i0, i1, j;

    /* Scan down the face string recording index set specifications. */

    i0 = vecnum(vv);
    read_face_vertices(vv, line);
    i1 = vecnum(vv);

    /* Convert our N new vertices into N-2 new triangles. */

    for (i = i0; i < i1 - 2; ++i)
        if ((j = vecadd(fv)) >= 0)
        {
            struct object_face *f = (struct object_face *) vecget(fv, j);
         
            f->vi[0] = i0;
            f->vi[1] = i + 1;
            f->vi[2] = i + 2;
        }
}

/*---------------------------------------------------------------------------*/

static int read_edge_indices(const char *line, int *vi, int *ui)
{
    static char vert[MAXSTR];
    int n;

    *vi = 0;
    *ui = 0;

    if (sscanf(line, "%s%n", vert, &n) >= 1)
    {
        if (sscanf(vert, "%d/%d", vi, ui) == 2) return n;
        if (sscanf(vert, "%d",    vi    ) == 1) return n;
    }
    return 0;
}

static void read_edge_vertices(vector_t vv, const char *line)
{
    const char *c = line;
    int dc;
    int vj, vc = vecnum(_vv);
    int tj, tc = vecnum(_uv);
    int i;

    /* Scan down the face string recording index set specifications. */

    while ((dc = read_edge_indices(c, &vj, &tj)))
        if ((i = vecadd(vv)) >= 0)
        {
            struct object_vert *v = (struct object_vert *) vecget(vv, i);

            /* Convert an edge index to a vector index. */

            int ui = (tj > 0) ? tj - 1 : tj + tc;
            int vi = (vj > 0) ? vj - 1 : vj + vc;

            /* Locate the indexed values in the vector caches. */

            struct vec2 *up = (0 <= ui && ui < tc) ? vecget(_uv, ui) : NULL;
            struct vec3 *vp = (0 <= vi && vi < vc) ? vecget(_vv, vi) : NULL;

            /* Initialize the new vertex, defaulting on bad input. */

            v->u[0] = up ? up->u : 0.0f;
            v->u[1] = up ? up->v : 0.0f;

            v->n[0] = 0.0f;
            v->n[1] = 0.0f;
            v->n[2] = 1.0f;

            v->v[0] = vp ? vp->x : 0.0f;
            v->v[1] = vp ? vp->y : 0.0f;
            v->v[2] = vp ? vp->z : 0.0f;

            /* Note bad indices. */

            if (tj && !up) uerr++;
            if (vj && !vp) verr++;

            c += dc;
        }
}

static void read_l(vector_t vv, vector_t ev, const char *line)
{
    int i, i0, i1, j;

    /* Scan down the edge string recording index set specifications. */

    i0 = vecnum(vv);
    read_edge_vertices(vv, line);
    i1 = vecnum(vv);

    /* Convert our N new vertices into N-1 new edges. */

    for (i = i0; i < i1 - 1; ++i)
        if ((j = vecadd(ev)) >= 0)
        {
            struct object_edge *e = (struct object_edge *) vecget(ev, j);
                
            e->vi[0] = i;
            e->vi[1] = i + 1;
        }
}

/*---------------------------------------------------------------------------*/

static const char *parse_name(const char *line)
{
    static char name[MAXSTR];
    char *last;

    /* Extract a name parameter from the given OBJ line. */

    if ((last = strrchr(line, ' ')))
    {
        sscanf(last, "%s", name);
        return name;
    }
    return NULL;
}

static const char *read_mtllib(vector_t mv, const char *line)
{
    static char file[MAXSTR];

    sscanf(line, "%s", file);
    return file;
}

static struct object_mesh *read_usemtl(vector_t mv, const char *line,
                                                    const char *file)
{
    int i;

    if ((i = vecadd(mv)) >= 0)
    {
        struct object_mesh *m = (struct object_mesh *) vecget(mv, i);

        if (line)
            m->brush = send_create_brush(file, parse_name(line));
        else
            m->brush = 0;

        m->fv = vecnew(0, sizeof (struct object_face));
        m->ev = vecnew(0, sizeof (struct object_edge));

        return m;
    }
    return NULL;
}

static void read_vt(const char *line)
{
    int i;

    if ((i = vecadd(_uv)) >= 0)
    {
        struct vec2 *up = (struct vec2 *) vecget(_uv, i);
        sscanf(line, "%f %f", &up->u, &up->v);
    }
}

static void read_vn(const char *line)
{
    int i;

    if ((i = vecadd(_nv)) >= 0)
    {
        struct vec3 *np = (struct vec3 *) vecget(_nv, i);
        sscanf(line, "%f %f %f", &np->x, &np->y, &np->z);
    }
}

static void read_v(const char *line)
{
    int i;

    if ((i = vecadd(_vv)) >= 0)
    {
        struct vec3 *vp = (struct vec3 *) vecget(_vv, i);
        sscanf(line, "%f %f %f", &vp->x, &vp->y, &vp->z);
    }
}

/*---------------------------------------------------------------------------*/

static void calc_face_tbn(const struct object_face *f, vector_t vv)
{
    struct object_vert *v0 = (struct object_vert *) vecget(vv, f->vi[0]);
    struct object_vert *v1 = (struct object_vert *) vecget(vv, f->vi[1]);
    struct object_vert *v2 = (struct object_vert *) vecget(vv, f->vi[2]);

    float du1[2], dv1[3], t[3];
    float du2[2], dv2[3], b[3];

    /* Calculate texture coordinate differences. */

    du1[0] = v1->u[0] - v0->u[0];
    du1[1] = v1->u[1] - v0->u[1];

    du2[0] = v2->u[0] - v0->u[0];
    du2[1] = v2->u[1] - v0->u[1];

    /* Calculate vertex coordinate differences. */

    dv1[0] = v1->v[0] - v0->v[0];
    dv1[1] = v1->v[1] - v0->v[1];
    dv1[2] = v1->v[2] - v0->v[2];

    dv2[0] = v2->v[0] - v0->v[0];
    dv2[1] = v2->v[1] - v0->v[1];
    dv2[2] = v2->v[2] - v0->v[2];

    /* Calculate and accumulate the tangent vector. */

    t[0] = du1[1] * dv2[0] - du2[1] * dv1[0];
    t[1] = du1[1] * dv2[1] - du2[1] * dv1[1];
    t[2] = du1[1] * dv2[2] - du2[1] * dv1[2];

    v0->t[0] += t[0];
    v0->t[1] += t[1];
    v0->t[2] += t[2];
    v1->t[0] += t[0];
    v1->t[1] += t[1];
    v1->t[2] += t[2];
    v2->t[0] += t[0];
    v2->t[1] += t[1];
    v2->t[2] += t[2];

    /* Calculate and accumulate the bitangent vector. */

    b[0] = du1[0] * dv2[0] - du2[0] * dv1[0];
    b[1] = du1[0] * dv2[1] - du2[0] * dv1[1];
    b[2] = du1[0] * dv2[2] - du2[0] * dv1[2];

    v0->b[0] += b[0];
    v0->b[1] += b[1];
    v0->b[2] += b[2];
    v1->b[0] += b[0];
    v1->b[1] += b[1];
    v1->b[2] += b[2];
    v2->b[0] += b[0];
    v2->b[1] += b[1];
    v2->b[2] += b[2];
}

static void calc_mesh_tbn(const struct object_mesh *m, vector_t vv)
{
    int i;

    /* Compute tangent and bitangent for all vertices used by this mesh. */

    for (i = 0; i < vecnum(m->fv); ++i)
        calc_face_tbn((const struct object_face *) vecget(m->fv, i), vv);
}

static void calc_tbn(struct object *o)
{
    int i;

    /* Compute tangent and bitangent for all vertices used by this object. */

    for (i = 0; i < vecnum(o->mv); ++i)
        calc_mesh_tbn((const struct object_mesh *) vecget(o->mv, i), o->vv);

    /* Normalize all tangent and bitangent vectors. */

    for (i = 0; i < vecnum(o->vv); ++i)
    {
        normalize(((struct object_vert *) vecget(o->vv, i))->t);
        normalize(((struct object_vert *) vecget(o->vv, i))->b);
    }
}

/*---------------------------------------------------------------------------*/

static int read_obj(const char *filename, struct object *o)
{
    char L[MAXSTR];
    char W[MAXSTR];
    FILE *fin;

    int i, j, n = 0;

    /* Initialize the object element vectors. */

    vector_t vv = o->vv = vecnew(0, sizeof (struct object_vert));
    vector_t mv = o->mv = vecnew(0, sizeof (struct object_mesh));

    if (filename)
    {
        path_push(get_file_path(filename));

        /* Initialize the loader vector caches. */

        _uv = vecnew(1024, sizeof (struct vec2));
        _nv = vecnew(1024, sizeof (struct vec3));
        _vv = vecnew(1024, sizeof (struct vec3));

        uerr = nerr = verr = 0;

        if ((fin = open_file(get_file_name(filename), "r")))
        {
            /* Create a default catch-all group using the default material. */

            struct object_mesh *m = read_usemtl(mv, NULL, NULL);
            const char         *F = NULL;
        
            /* Process each line, invoking the handler for each keyword. */

            while (fgets(L, MAXSTR, fin))
                if (sscanf(L, "%s%n", W, &n) >= 1)
                {
                    char *V = L + n;

                    if      (!strcmp(W, "mtllib")) F = read_mtllib(mv, V);
                    else if (!strcmp(W, "usemtl")) m = read_usemtl(mv, V, F);

                    else if (!strcmp(W, "f"))  read_f(vv, m->fv, V);
                    else if (!strcmp(W, "l"))  read_l(vv, m->ev, V);

                    else if (!strcmp(W, "vt")) read_vt(V);
                    else if (!strcmp(W, "vn")) read_vn(V);
                    else if (!strcmp(W, "v" )) read_v (V);
                }
            
            fclose(fin);
        }
        else error("OBJ file '%s': %s", filename, system_error());

        /* Remove any empty meshes. */

        for (i = 0; i < vecnum(o->mv); )
            if (vecnum(((struct object_mesh *) vecget(o->mv, i))->fv) ||
                vecnum(((struct object_mesh *) vecget(o->mv, i))->ev))
                i++;
            else
            {
                memmove(vecget(o->mv, i),
                        vecget(o->mv, i + 1),
                       (vecnum(o->mv) - i - 1) * sizeof (struct object_mesh));
                vecpop(o->mv);
            }

        /* Compute tangent and bitangent vectors. */

        calc_tbn(o);

        /* Sort meshes such that transparent ones appear last. */

        for (i = 0; i < vecnum(o->mv); ++i)
            for (j = i + 1; j < vecnum(o->mv); ++j)
            {
                struct object_mesh *mi = vecget(o->mv, i);
                struct object_mesh *mj = vecget(o->mv, j);
                struct object_mesh  mt;

                if (get_brush_t(mi->brush) > get_brush_t(mj->brush))
                {
                     mt = *mi;
                    *mi = *mj;
                    *mj =  mt;
                }
            }

        /* Release the loader caches. */

        vecdel(_vv);
        vecdel(_nv);
        vecdel(_uv);

        /* Report index errors. */

        if (uerr > 0)
            error("OBJ file '%s' has %d bad texture indices", filename, uerr);
        if (nerr > 0)
            error("OBJ file '%s' has %d bad normal indices",  filename, nerr);
        if (verr > 0)
            error("OBJ file '%s' has %d bad vertex indices",  filename, verr);

        path_pop();
    }
    return 1;
}

/*===========================================================================*/
/* Object constructors                                                       */

static int create_mesh(int i)
{
    int j;

    if ((j = vecadd(get_object(i)->mv)) >= 0)
    {
        struct object_mesh *m = get_object_mesh(i, j);

        m->fv = vecnew(0, sizeof (struct object_face));
        m->ev = vecnew(0, sizeof (struct object_edge));
    }
    return j;
}

static int create_vert(int i)
{
    int j;

    if ((j = vecadd(get_object(i)->vv)) >= 0)
    {
        struct object_vert *p = get_object_vert(i, j);

        p->n[0] = 0.0;
        p->n[0] = 0.0;
        p->n[1] = 1.0;

        get_object(i)->aabb_state = 0;
    }
    return j;
}

static int create_face(int i, int j)
{
    return vecadd(get_object_mesh(i, j)->fv);
}

static int create_edge(int i, int j)
{
    return vecadd(get_object_mesh(i, j)->ev);
}

/*---------------------------------------------------------------------------*/
/* Object destructors                                                        */

static void delete_edge(int i, int j, int k)
{
    struct object_mesh *m = get_object_mesh(i, j);

    /* Remove this edge from the edge vector. */

    memmove(vecget(m->ev, k),
            vecget(m->ev, k + 1), vecsiz(m->ev) * (vecnum(m->ev) - k - 1));

    vecpop(m->ev);
}

static void delete_face(int i, int j, int k)
{
    struct object_mesh *m = get_object_mesh(i, j);

    /* Remove this face from the face vector. */

    memmove(vecget(m->fv, k),
            vecget(m->fv, k + 1), vecsiz(m->fv) * (vecnum(m->fv) - k - 1));

    vecpop(m->fv);
}

static void delete_vert(int i, int j)
{
    struct object *o = get_object(i);

    int k;
    int l;

    /* Remove this vertex from the vertex vector. */

    memmove(vecget(o->vv, j),
            vecget(o->vv, j + 1), vecsiz(o->vv) * (vecnum(o->vv) - j - 1));

    vecpop(o->vv);

    /* Remove all references to this vertex from all meshes. */

    for (k = 0; k < get_mesh_count(i); ++k)
    {
        /* Delete all referencing faces.  Move later references down. */

        for (l = 0; l < get_face_count(i, k); ++l)
        {
            struct object_face *f = get_object_face(i, k, l);

            if (f->vi[0] == j || f->vi[1] == j || f->vi[2] == j)
                delete_face(i, k, l--);
            else
            {
                if (f->vi[0] > j) f->vi[0]--;
                if (f->vi[1] > j) f->vi[1]--;
                if (f->vi[2] > j) f->vi[2]--;
            }
        }

        /* Delete all referencing edges.  Move later references down. */

        for (l = 0; l < get_edge_count(i, k); ++l)
        {
            struct object_edge *e = get_object_edge(i, k, l);

            if (e->vi[0] == j || e->vi[1] == j)
                delete_edge(i, k, l--);
            else
            {
                if (e->vi[0] > j) e->vi[0]--;
                if (e->vi[1] > j) e->vi[1]--;
            }
        }
    }

    /* Invalidate the object's vertex buffer and bounding volume. */

    fini_object(i);
}

static void delete_mesh(int i, int j)
{
    struct object      *o = get_object(i);
    struct object_mesh *m = get_object_mesh(i, j);

    /* Release this mesh's resources. */

    vecdel(m->fv);
    vecdel(m->ev);

    /* Remove this mesh from the mesh vector. */

    memmove(vecget(o->mv, j),
            vecget(o->mv, j + 1), vecsiz(o->mv) * (vecnum(o->mv) - j - 1));

    vecpop(o->mv);
}

/*---------------------------------------------------------------------------*/
/* Object normalization                                                      */

static void normal(float n[3], const float a[3],
                               const float b[3],
                               const float c[3])
{
    float u[3];
    float v[3];

    u[0] = b[0] - a[0];
    u[1] = b[1] - a[1];
    u[2] = b[2] - a[2];

    v[0] = c[0] - a[0];
    v[1] = c[1] - a[1];
    v[2] = c[2] - a[2];

    cross(n, u, v);
    normalize(n);
}

static void normal_mesh(int i)
{
    int j;
    int k;

    float n[3];

    /* Zero the normals of all vertices of this mesh. */

    for (j = 0; j < get_vert_count(i); ++j)
    {
        struct object_vert *v = get_object_vert(i, j);

        v->n[0] = 0.0f;
        v->n[1] = 0.0f;
        v->n[2] = 0.0f;
    }

    /* Compute and accumulate the normals of all faces of this mesh. */

    for (j = 0; j < get_mesh_count(i); ++j)
        for (k = 0; k < get_face_count(i, j); ++k)
        {
            struct object_face *f  = get_object_face(i, j, k);
            struct object_vert *v0 = get_object_vert(i, f->vi[0]);
            struct object_vert *v1 = get_object_vert(i, f->vi[1]);
            struct object_vert *v2 = get_object_vert(i, f->vi[2]);

            normal(n, v0->v, v1->v, v2->v);

            v0->n[0] += n[0];
            v0->n[1] += n[1];
            v0->n[2] += n[2];

            v1->n[0] += n[0];
            v1->n[1] += n[1];
            v1->n[2] += n[2];

            v2->n[0] += n[0];
            v2->n[1] += n[1];
            v2->n[2] += n[2];
        }

    /* Normalize all computed normals. */

    for (k = 0; k < get_vert_count(i); ++k)
        normalize(get_object_vert(i, k)->n);
}


/*---------------------------------------------------------------------------*/
/* Object query                                                              */

int get_mesh(int i, int j)
{
    if (0 <= j && j < get_mesh_count(i))
        return get_object_mesh(i, j)->brush;
    else
        return -1;
}

void get_vert(int i, int j, float v[3], float n[3], float u[2])
{
    if (0 <= j && j < get_vert_count(i))
    {
        struct object_vert *p = get_object_vert(i, j);

        v[0] = p->v[0];
        v[1] = p->v[1];
        v[2] = p->v[2];

        n[0] = p->n[0];
        n[1] = p->n[1];
        n[2] = p->n[2];

        u[0] = p->u[0];
        u[1] = p->u[1];
    }
}

void get_face(int i, int j, int k, int vi[3])
{
    if (0 <= k && k < get_face_count(i, j))
    {
        struct object_face *f = get_object_face(i, j, k);

        vi[0] = f->vi[0];
        vi[1] = f->vi[1];
        vi[2] = f->vi[2];
    }
}

void get_edge(int i, int j, int k, int vi[2])
{
    if (0 <= k && k < get_edge_count(i, j))
    {
        struct object_edge *e = get_object_edge(i, j, k);

        vi[0] = e->vi[0];
        vi[1] = e->vi[1];
    }
}

/*---------------------------------------------------------------------------*/
/* Object counters                                                           */

int get_mesh_count(int i)
{
    return vecnum(get_object(i)->mv);
}

int get_vert_count(int i)
{
    return vecnum(get_object(i)->vv);
}

int get_face_count(int i, int j)
{
    return vecnum(get_object_mesh(i, j)->fv);
}

int get_edge_count(int i, int j)
{
    return vecnum(get_object_mesh(i, j)->ev);
}

/*===========================================================================*/

int send_create_object(const char *filename)
{
    int i;

    if ((i = new_object()) >= 0)
    {
        struct object      *o = get_object(i);
        struct object_mesh *m = NULL;

        if (read_obj(filename, o))
        {
            int j, n = vecnum(o->mv);

            o->count = 1;

            /* Send the object header. */

            send_event(EVENT_CREATE_OBJECT);
            send_index(n);

            /* Send the vertices and meshes. */

            send_vector(o->vv);

            for (j = 0; j < n; ++j)
            {
                m = (struct object_mesh *) vecget(o->mv, j);

                send_index (m->brush);
                send_vector(m->fv);
                send_vector(m->ev);
            }

            /* Encapsulate this object in an entity. */

            return send_create_entity(TYPE_OBJECT, i);
        }
    }
    return -1;
}

void recv_create_object(void)
{
    /* Unpack the object header. */

    int i = new_object();
    int n = recv_index();

    int j, k;

    struct object      *o = get_object(i);
    struct object_mesh *m = NULL;

    o->count = 1;

    /* Unpack the vertices and meshes. */

    o->vv = recv_vector();
    o->mv = vecnew(n, sizeof (struct object_mesh));

    for (j = 0; j < n; ++j)
        if ((k = vecadd(o->mv)) >= 0)
        {
            m = (struct object_mesh *) vecget(o->mv, k);

            m->brush = recv_index();
            m->fv    = recv_vector();
            m->ev    = recv_vector();
        }

    /* Encapsulate this object in an entity. */

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

int send_create_mesh(int i)
{
    send_event(EVENT_CREATE_MESH);
    send_index(i);

    return create_mesh(i);
}

void recv_create_mesh(void)
{
    int i = recv_index();

    create_mesh(i);
}

/*---------------------------------------------------------------------------*/

int send_create_vert(int i)
{
    send_event(EVENT_CREATE_VERT);
    send_index(i);

    return create_vert(i);
}

void recv_create_vert(void)
{
    int i = recv_index();

    create_vert(i);
}

/*---------------------------------------------------------------------------*/

int send_create_face(int i, int j)
{
    send_event(EVENT_CREATE_FACE);
    send_index(i);
    send_index(j);

    return create_face(i, j);
}

void recv_create_face(void)
{
    int i = recv_index();
    int j = recv_index();

    create_face(i, j);
}

/*---------------------------------------------------------------------------*/

int send_create_edge(int i, int j)
{
    send_event(EVENT_CREATE_EDGE);
    send_index(i);
    send_index(j);

    return create_edge(i, j);
}

void recv_create_edge(void)
{
    int i = recv_index();
    int j = recv_index();

    create_edge(i, j);
}

/*---------------------------------------------------------------------------*/

void send_delete_mesh(int i, int j)
{
    send_event(EVENT_DELETE_MESH);
    send_index(i);
    send_index(j);

    delete_mesh(i, j);
}

void recv_delete_mesh(void)
{
    int i = recv_index();
    int j = recv_index();

    delete_mesh(i, j);
}

/*---------------------------------------------------------------------------*/

void send_delete_vert(int i, int j)
{
    send_event(EVENT_DELETE_VERT);
    send_index(i);
    send_index(j);

    delete_vert(i, j);
}

void recv_delete_vert(void)
{
    int i = recv_index();
    int j = recv_index();

    delete_vert(i, j);
}

/*---------------------------------------------------------------------------*/

void send_delete_face(int i, int j, int k)
{
    send_event(EVENT_DELETE_FACE);
    send_index(i);
    send_index(j);
    send_index(k);

    delete_face(i, j, k);
}

void recv_delete_face(void)
{
    int i = recv_index();
    int j = recv_index();
    int k = recv_index();

    delete_face(i, j, k);
}

/*---------------------------------------------------------------------------*/

void send_delete_edge(int i, int j, int k)
{
    send_event(EVENT_DELETE_EDGE);
    send_index(i);
    send_index(j);
    send_index(k);

    delete_edge(i, j, k);
}

void recv_delete_edge(void)
{
    int i = recv_index();
    int j = recv_index();
    int k = recv_index();

    delete_edge(i, j, k);
}

/*---------------------------------------------------------------------------*/

void send_normal_mesh(int i)
{
    send_event(EVENT_NORMAL_MESH);
    send_index(i);

    normal_mesh(i);
}

void recv_normal_mesh(void)
{
    int i = recv_index();

    normal_mesh(i);
}

/*---------------------------------------------------------------------------*/

void send_set_mesh(int i, int j, int k)
{
    struct object_mesh *m = get_object_mesh(i, j);

    dupe_create_brush(k);
    send_delete_brush(m->brush);

    send_event(EVENT_SET_MESH);
    send_index(i);
    send_index(j);
    send_index((m->brush = k));
}

void recv_set_mesh(void)
{
    int i = recv_index();
    int j = recv_index();
    int k = recv_index();

    get_object_mesh(i, j)->brush = k;
}

/*---------------------------------------------------------------------------*/

void send_set_vert(int i, int j, float v[3], float n[3], float u[2])
{
    struct object_vert *p = get_object_vert(i, j);

    p->v[0] = v[0];
    p->v[1] = v[1];
    p->v[2] = v[2];

    p->n[0] = n[0];
    p->n[1] = n[1];
    p->n[2] = n[2];

    p->u[0] = u[0];
    p->u[1] = u[1];

    send_event(EVENT_SET_VERT);
    send_index(i);
    send_index(j);
    send_array(p->v, 3, sizeof (float));
    send_array(p->n, 3, sizeof (float));
    send_array(p->u, 2, sizeof (float));

    fini_object(i);
}

void recv_set_vert(void)
{
    int i = recv_index();
    int j = recv_index();

    struct object_vert *p = get_object_vert(i, j);

    recv_array(p->v, 3, sizeof (float));
    recv_array(p->n, 3, sizeof (float));
    recv_array(p->u, 2, sizeof (float));

    fini_object(i);
}

/*---------------------------------------------------------------------------*/

void send_set_face(int i, int j, int k, int vi[3])
{
    struct object_face *f = get_object_face(i, j, k);

    f->vi[0] = vi[0];
    f->vi[1] = vi[1];
    f->vi[2] = vi[2];

    send_event(EVENT_SET_FACE);
    send_index(i);
    send_index(j);
    send_index(k);
    send_array(f->vi, 3, sizeof (int));
}

void recv_set_face(void)
{
    int i = recv_index();
    int j = recv_index();
    int k = recv_index();

    recv_array(get_object_face(i, j, k)->vi, 3, sizeof (int));
}

/*---------------------------------------------------------------------------*/

void send_set_edge(int i, int j, int k, int vi[2])
{
    struct object_edge *e = get_object_edge(i, j, k);

    e->vi[0] = vi[0];
    e->vi[1] = vi[1];

    send_event(EVENT_SET_EDGE);
    send_index(i);
    send_index(j);
    send_index(k);
    send_array(e->vi, 2, sizeof (int));
}

void recv_set_edge(void)
{
    int i = recv_index();
    int j = recv_index();
    int k = recv_index();

    recv_array(get_object_edge(i, j, k)->vi, 2, sizeof (int));
}

/*===========================================================================*/

static void init_object(int i)
{
    struct object *o = get_object(i);

    if (o->state == 0)
    {
        /* Initialize the buffer object. */
    
        if (GL_has_vertex_buffer_object)
        {
            glGenBuffersARB(1, &o->buffer);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, o->buffer);

            glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                            vecnum(o->vv) * sizeof (struct object_vert),
                            vecbuf(o->vv), GL_STATIC_DRAW_ARB);
        }
    
        o->state = 1;
    }
}

static void fini_object(int i)
{
    struct object *o = get_object(i);

    if (o->state == 1)
    {
        /* Free the vertex buffer object. */

        if (GL_has_vertex_buffer_object)
            if (glIsBufferARB(o->buffer))
                glDeleteBuffersARB(1, &o->buffer);

        o->buffer = 0;
        o->state  = 0;
    }
}

/*---------------------------------------------------------------------------*/

static void draw_vert(GLubyte *base)
{
    GLsizei s = sizeof (struct object_vert);

    /* Enable all necessary vertex attribute pointers. */

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableVertexAttribArrayARB(6);
    glEnableVertexAttribArrayARB(7);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    /* Bind all vertex attribute pointers. */

    glVertexPointer         (3,    GL_FLOAT,    s, base);
    glVertexAttribPointerARB(6, 3, GL_FLOAT, 0, s, base + 3  * sizeof (float));
    glVertexAttribPointerARB(7, 3, GL_FLOAT, 0, s, base + 6  * sizeof (float));
    glNormalPointer         (      GL_FLOAT,    s, base + 9  * sizeof (float));
    glTexCoordPointer       (2,    GL_FLOAT,    s, base + 12 * sizeof (float));
}

static void draw_mesh(const struct object_mesh *m, float alpha)
{
    glPushAttrib(GL_TEXTURE_BIT);
    {
        int transparent = draw_brush(m->brush, alpha);

        /* If this object is transparent then don't write depth. */
        /* Render back and front faces separately.               */

        if (transparent)
        {
            glPushAttrib(GL_DEPTH_BUFFER_BIT);
            glDepthMask(GL_FALSE);

            glCullFace(GL_FRONT);

            if (vecnum(m->fv) > 0)
                glDrawElements(GL_TRIANGLES, 3 * vecnum(m->fv),
                               GL_UNSIGNED_INT,  vecbuf(m->fv));

            glCullFace(GL_BACK);
        }

        /* Render all faces and edges. */

        if (vecnum(m->fv) > 0)
            glDrawElements(GL_TRIANGLES, 3 * vecnum(m->fv),
                           GL_UNSIGNED_INT,  vecbuf(m->fv));
        if (vecnum(m->ev) > 0)
            glDrawElements(GL_LINES,     2 * vecnum(m->ev),
                           GL_UNSIGNED_INT,  vecbuf(m->ev));

        if (transparent)
            glPopAttrib();

        if (GL_has_shader_objects)
            glUseProgramObjectARB(0);
    }
    glPopAttrib();
}

static void draw_object(int i, int j, int f, float a)
{
    struct object      *o = get_object(i);
    struct object_mesh *m = NULL;

    float alpha = get_entity_alpha(j) * a;

    init_object(i);

    glPushMatrix();
    {
        /* Apply the local coordinate system transformation. */

        transform_entity(j);

        /* Render this object. */

        if (test_entity_aabb(j) >= 0)
        {
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT);
            {
                int k, n = vecnum(o->mv);

                /* Bind a vertex buffer or array. */

                if (GL_has_vertex_buffer_object)
                {
                    glBindBufferARB(GL_ARRAY_BUFFER_ARB, o->buffer);
                    draw_vert(0);
                }
                else
                    draw_vert(vecget(o->vv, 0));

                /* Draw each surface. */

                for (k = 0; k < n; ++k)
                {
                    m = (struct object_mesh *) vecget(o->mv, k);
  
                    if (vecnum(m->fv) > 0 || vecnum(m->ev) > 0)
                        draw_mesh(m, alpha);
                }
            }
            glPopAttrib();
            glPopClientAttrib();
        }

        /* Render all child entities in this coordinate system. */

        draw_entity_tree(j, f, a * get_entity_alpha(j));
    }
    glPopMatrix();
}

static void aabb_object(int i, float aabb[6])
{
    struct object *o = get_object(i);

    /* If the bounding box cache is invalid... */

    if (o->aabb_state == 0)
    {
        int j, n = vecnum(o->vv);

        /* Find the object's bounding box. */

        if (n > 0)
        {
            const float *v = ((struct object_vert *) vecget(o->vv, 0))->v;

            o->aabb_cache[0] = v[0];
            o->aabb_cache[1] = v[1];
            o->aabb_cache[2] = v[2];
            o->aabb_cache[3] = v[0];
            o->aabb_cache[4] = v[1];
            o->aabb_cache[5] = v[2];
        }
        else memset(o->aabb_cache, 0, 6 * sizeof (float));

        for (j = 0; j < n; ++j)
        {
            const float *v = ((struct object_vert *) vecget(o->vv, j))->v;

            o->aabb_cache[0] = MIN(v[0], o->aabb_cache[0]);
            o->aabb_cache[1] = MIN(v[1], o->aabb_cache[1]);
            o->aabb_cache[2] = MIN(v[2], o->aabb_cache[2]);
            o->aabb_cache[3] = MAX(v[0], o->aabb_cache[3]);
            o->aabb_cache[4] = MAX(v[1], o->aabb_cache[4]);
            o->aabb_cache[5] = MAX(v[2], o->aabb_cache[5]);
        }

        o->aabb_state = 1;
    }

    /* Return the current bounding box. */

    aabb[0] = o->aabb_cache[0];
    aabb[1] = o->aabb_cache[1];
    aabb[2] = o->aabb_cache[2];
    aabb[3] = o->aabb_cache[3];
    aabb[4] = o->aabb_cache[4];
    aabb[5] = o->aabb_cache[5];
}

/*---------------------------------------------------------------------------*/

static void dupe_object(int i)
{
    get_object(i)->count++;
}

static void free_object(int i)
{
    struct object *o = get_object(i);

    if (o->count > 0)
    {
        o->count--;

        if (o->count == 0)
        {
            int j;
        
            fini_object(i);

            for (j = 0; j < vecnum(o->mv); ++j)
            {
                struct object_mesh *m = vecget(o->mv, j);

                send_delete_brush(m->brush);

                vecdel(m->fv);
                vecdel(m->ev);
            }

            vecdel(o->vv);
            vecdel(o->mv);
            memset(o, 0, sizeof (struct object));
        }
    }
}

/*===========================================================================*/

static struct entity_func object_func = {
    "object",
    init_object,
    fini_object,
    aabb_object,
    draw_object,
    dupe_object,
    free_object,
};

struct entity_func *startup_object(void)
{
    if ((object = vecnew(MIN_OBJECTS, sizeof (struct object))))
        return &object_func;
    else
        return NULL;
}
