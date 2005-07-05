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
#include "image.h"
#include "event.h"
#include "utility.h"
#include "object.h"

/*---------------------------------------------------------------------------*/

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

struct object_mtrl
{
    const char *name;
    int image;

    float d[4];
    float a[4];
    float s[4];
    float e[4];
    float x[1];
};

struct object_vert
{
    float t[2];
    float n[3];
    float v[3];
};

struct object_face
{
    int vi[3];
};

struct object_edge
{
    int vi[2];
};

struct object_surf
{
    int      mi;
    vector_t fv;
    vector_t ev;
};

struct object
{
    int count;
    int state;

    GLuint buffer;

    vector_t mv;
    vector_t vv;
    vector_t sv;

    float bound[6];
};

static vector_t object;

/*---------------------------------------------------------------------------*/
/*
#define O(i) ((struct object *) vecget(object, i))
*/

struct object *O(int i)
{
    return (struct object *) vecget(object, i);
}

static int new_object(void)
{
    int i, n = vecnum(object);

    for (i = 0; i < n; ++i)
        if (O(i)->count == 0)
            return i;

    return vecadd(object);
}

/*===========================================================================*/
/* OBJ loader caches                                                         */

static vector_t _tv;
static vector_t _nv;
static vector_t _vv;

/*---------------------------------------------------------------------------*/

static int new_mtrl(vector_t mv, const char *name)
{
    int i;

    if ((i = vecadd(mv)) >= 0)
    {
        struct object_mtrl *m = (struct object_mtrl *) vecget(mv, i);

        m->name  = name ? memdup(name, 1, strlen(name) + 1) : NULL;
        m->image = 0;

        /* Default diffuse */

        m->d[0] = 0.8f;
        m->d[1] = 0.8f;
        m->d[2] = 0.8f;
        m->d[3] = 1.0f;

        /* Default ambient */

        m->a[0] = 0.2f;
        m->a[1] = 0.2f;
        m->a[2] = 0.2f;
        m->a[3] = 1.0f;

        /* Default specular */

        m->s[0] = 0.0f;
        m->s[1] = 0.0f;
        m->s[2] = 0.0f;
        m->s[3] = 1.0f;

        /* Default emmisive */

        m->e[0] = 0.0f;
        m->e[1] = 0.0f;
        m->e[2] = 0.0f;
        m->e[3] = 1.0f;

        /* Default shininess */

        m->x[0] = 0.0f;

        return i;
    }
    return 0;
}

static struct object_mtrl *read_newmtl(vector_t mv, const char *line)
{
    char  name[MAXSTR];
    char *last;

    if ((last = strrchr(line, ' ')))
    {
        sscanf(last, "%s", name);
        return (struct object_mtrl *) vecget(mv, new_mtrl(mv, name));
    }
    else
        return (struct object_mtrl *) vecget(mv, 0);
}

static void read_map_Kd(struct object_mtrl *m, const char *line)
{
    char  name[MAXSTR];
    char *last;

    if ((last = strrchr(line, ' ')))
    {
        sscanf(last, "%s", name);
        m->image = send_create_image(name);
    }
    else
        m->image = 0;
}

static void read_Kd(struct object_mtrl *m, const char *line)
{
    sscanf(line, "%f %f %f", m->d, m->d + 1, m->d + 2);
}

static void read_Ka(struct object_mtrl *m, const char *line)
{
    sscanf(line, "%f %f %f", m->a, m->a + 1, m->a + 2);
}

static void read_Ks(struct object_mtrl *m, const char *line)
{
    sscanf(line, "%f %f %f", m->s, m->s + 1, m->s + 2);
}

static void read_Ke(struct object_mtrl *m, const char *line)
{
    sscanf(line, "%f %f %f", m->e, m->e + 1, m->e + 2);
}

static void read_Ns(struct object_mtrl *m, const char *line)
{
    sscanf(line, "%f", m->x);
}

static void read_d(struct object_mtrl *m, const char *line)
{
    sscanf(line, "%f", m->d + 3);
}

static void read_mtl(vector_t mv, const char *filename)
{
    char L[MAXSTR];
    char W[MAXSTR];
    FILE *fin;
    int n = 0;

    if ((fin = open_file(filename, "r")))
    {
        struct object_mtrl *m = NULL;

        /* Process each line, invoking the handler for each keyword. */

        while (fgets(L, MAXSTR, fin))
            if (sscanf(L, "%s%n", W, &n) >= 1)
            {
                if      (!strcmp(W, "newmtl")) m = read_newmtl(mv, L + n);
                else if (!strcmp(W, "map_Kd"))     read_map_Kd(m,  L + n);

                else if (!strcmp(W, "Kd")) read_Kd(m, L + n);
                else if (!strcmp(W, "Ka")) read_Ka(m, L + n);
                else if (!strcmp(W, "Ks")) read_Ks(m, L + n);
                else if (!strcmp(W, "Ke")) read_Ke(m, L + n);
                else if (!strcmp(W, "Ns")) read_Ns(m, L + n);
                else if (!strcmp(W, "d"))  read_d (m, L + n);
            }

        fclose(fin);
    }
    else error("MTL file '%s': %s", filename, system_error());
}

static int find_mtl(vector_t mv, const char *name)
{
    int i, n = vecnum(mv);

    for (i = 0; i < n; ++i)
    {
        struct object_mtrl *m = (struct object_mtrl *) vecget(mv, i);

        if (m->name && strncmp(m->name, name, MAXSTR) == 0)
            return i;
    }

    error("MTL: Unknown material '%s'", name);
    return 0;
}

/*---------------------------------------------------------------------------*/

static int read_face_indices(const char *line, int *vi, int *ti, int *ni)
{
    static char vert[MAXSTR];
    int n;

    *vi = 0;
    *ti = 0;
    *ni = 0;

    if (sscanf(line, "%s%n", vert, &n) >= 1)
    {
        if (sscanf(vert, "%d/%d/%d", vi, ti, ni) == 3) return n;
        if (sscanf(vert, "%d/%d",    vi, ti    ) == 2) return n;
        if (sscanf(vert, "%d//%d",   vi,     ni) == 2) return n;
        if (sscanf(vert, "%d",       vi        ) == 1) return n;
    }
    return 0;
}

static void read_face_vertices(vector_t vv, const char *line)
{
    const char *c = line;
    int dc;
    int vi;
    int ti;
    int ni;
    int i;

    /* Scan down the face string recording index set specifications. */

    while ((dc = read_face_indices(c, &vi, &ti, &ni)))
        if ((i = vecadd(vv)) >= 0)
        {
            struct object_vert *v = (struct object_vert *) vecget(vv, i);

            struct vec2 *tp = NULL;
            struct vec3 *np = NULL;
            struct vec3 *vp = NULL;

            /* Locate the indexed value in the vector caches. */

            if      (ti > 0) tp = (struct vec2 *) vecget(_tv, ti - 1);
            else if (ti < 0) tp = (struct vec2 *) vecget(_tv, vecnum(_tv)+ti);
            if      (ni > 0) np = (struct vec3 *) vecget(_nv, ni - 1);
            else if (ni < 0) np = (struct vec3 *) vecget(_nv, vecnum(_nv)+ni);
            if      (vi > 0) vp = (struct vec3 *) vecget(_vv, vi - 1);
            else if (vi < 0) vp = (struct vec3 *) vecget(_vv, vecnum(_vv)+vi);

            /* Initialize the new vertex. */

            v->t[0] = tp ? tp->u : 0.0f;
            v->t[1] = tp ? tp->v : 0.0f;

            v->n[0] = np ? np->x : 0.0f;
            v->n[1] = np ? np->y : 0.0f;
            v->n[2] = np ? np->z : 1.0f;

            v->v[0] = vp ? vp->x : 0.0f;
            v->v[1] = vp ? vp->y : 0.0f;
            v->v[2] = vp ? vp->z : 0.0f;

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

static int read_edge_indices(const char *line, int *vi, int *ti)
{
    static char vert[MAXSTR];
    int n;

    *vi = 0;
    *ti = 0;

    if (sscanf(line, "%s%n", vert, &n) >= 1)
    {
        if (sscanf(vert, "%d/%d", vi, ti) == 2) return n;
        if (sscanf(vert, "%d",    vi    ) == 1) return n;
    }
    return 0;
}

static void read_edge_vertices(vector_t vv, const char *line)
{
    const char *c = line;
    int dc;
    int vi;
    int ti;
    int i;

    /* Scan down the face string recording index set specifications. */

    while ((dc = read_edge_indices(c, &vi, &ti)))
        if ((i = vecadd(vv)) >= 0)
        {
            struct object_vert *v = (struct object_vert *) vecget(vv, i);

            struct vec2 *tp = NULL;
            struct vec3 *vp = NULL;

            /* Locate the indexed value in the vector caches. */

            if      (ti > 0) tp = (struct vec2 *) vecget(_tv, ti - 1);
            else if (ti < 0) tp = (struct vec2 *) vecget(_tv, vecnum(_tv)+ti);
            if      (vi > 0) vp = (struct vec3 *) vecget(_vv, vi - 1);
            else if (vi < 0) vp = (struct vec3 *) vecget(_vv, vecnum(_vv)+vi);

            /* Initialize the new vertex. */

            v->t[0] = tp ? tp->u : 0.0f;
            v->t[1] = tp ? tp->v : 0.0f;

            v->n[0] = 0.0f;
            v->n[1] = 0.0f;
            v->n[2] = 1.0f;

            v->v[0] = vp ? vp->x : 0.0f;
            v->v[1] = vp ? vp->y : 0.0f;
            v->v[2] = vp ? vp->z : 0.0f;

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

static struct object_surf *read_g(vector_t sv, int mi)
{
    int i;

    if ((i = vecadd(sv)) >= 0)
    {
        struct object_surf *s = (struct object_surf *) vecget(sv, i);

        s->mi = mi;
        s->fv = vecnew(256, sizeof (struct object_face));
        s->ev = vecnew(256, sizeof (struct object_edge));

        return s;
    }
    return NULL;
}

static void read_mtllib(vector_t mv, const char *line)
{
    char file[MAXSTR];
    int n = 0;
    int s = 0;

    while (sscanf(line + s, "%s%n", file, &n) == 1)
    {
        read_mtl(mv, file);
        s += n;
    }
}

static int read_usemtl(vector_t mv, const char *line)
{
    char name[MAXSTR];

    if (sscanf(line, "%s", name) == 1)
        return find_mtl(mv, name);
    else
        return -1;
}

static struct object_surf *read_s(vector_t mv, vector_t sv, const char *line)
{
    int i;

    if ((i = vecadd(sv)) >= 0)
    {
        struct object_surf *s = (struct object_surf *) vecget(sv, i);

        s->mi = read_usemtl(mv, line);
        s->fv = vecnew(256, sizeof (struct object_face));
        s->ev = vecnew(256, sizeof (struct object_edge));

        return s;
    }
    return NULL;
}

static void read_vt(const char *line)
{
    int i;

    if ((i = vecadd(_tv)) >= 0)
    {
        struct vec2 *tp = (struct vec2 *) vecget(_tv, i);
        sscanf(line, "%f %f", &tp->u, &tp->v);
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

static int read_obj(const char *filename, struct object *o)
{
    char L[MAXSTR];
    char W[MAXSTR];
    FILE *fin;

    int i;
    int r = 0;
    int n = 0;

    /* Initialize the loader vector caches. */

    _tv   = vecnew(1024, sizeof (struct vec2));
    _nv   = vecnew(1024, sizeof (struct vec3));
    _vv   = vecnew(1024, sizeof (struct vec3));

    /* Initialize the object element vectors. */

    o->mv = vecnew(4,    sizeof (struct object_mtrl));
    o->vv = vecnew(1024, sizeof (struct object_vert));
    o->sv = vecnew(4,    sizeof (struct object_surf));

    if ((fin = open_file(filename, "r")))
    {
        /* Create a default catch-all group using the default material. */

        struct object_surf *s = read_g(o->sv, new_mtrl(o->mv, NULL));

        /* Process each l, invoking the handler for each keyword. */

        while (fgets(L, MAXSTR, fin))
            if (sscanf(L, "%s%n", W, &n) >= 1)
            {
                if      (!strcmp(W, "mtllib"))     read_mtllib(o->mv, L + n);

                else if (!strcmp(W, "usemtl")) s = read_s(o->mv, o->sv, L + n);
                else if (!strcmp(W, "f"))          read_f(o->vv, s->fv, L + n);
                else if (!strcmp(W, "l"))          read_l(o->vv, s->ev, L + n);

                else if (!strcmp(W, "vt"))         read_vt(L + n);
                else if (!strcmp(W, "vn"))         read_vn(L + n);
                else if (!strcmp(W, "v"))          read_v (L + n);
            }

        fclose(fin);

        r = 1;
    }
    else error("OBJ file '%s': %s", filename, system_error());

    /* Find the object's bounding box. */

    if (vecnum(o->vv) > 0)
    {
        const float *v = ((struct object_vert *) vecget(o->vv, 0))->v;

        o->bound[0] = o->bound[3] = v[0];
        o->bound[1] = o->bound[4] = v[1];
        o->bound[2] = o->bound[5] = v[2];
    }

    for (i = 0; i < vecnum(o->vv); ++i)
    {
        const float *v = ((struct object_vert *) vecget(o->vv, i))->v;

        o->bound[0] = MIN(v[0], o->bound[0]);
        o->bound[1] = MIN(v[1], o->bound[1]);
        o->bound[2] = MIN(v[2], o->bound[2]);
        o->bound[3] = MAX(v[0], o->bound[3]);
        o->bound[4] = MAX(v[1], o->bound[4]);
        o->bound[5] = MAX(v[2], o->bound[5]);
    }

    /* Release the loader caches. */

    vecdel(_vv);
    vecdel(_nv);
    vecdel(_tv);

    return r;
}

/*---------------------------------------------------------------------------*/

int send_create_object(const char *filename)
{
    struct object_surf *s;
    int i;
    int j;

    if ((i = new_object()) >= 0)
    {
        /* If the file exists and is successfully read... */

        if ((read_obj(filename, O(i))))
        {
            int n = vecnum(O(i)->sv);

            O(i)->count = 1;

            /* Send the object header. */

            send_event(EVENT_CREATE_OBJECT);
            send_index(n);
            send_array(O(i)->bound, 6, sizeof (float));

            /* Send the vertices and materials. */

            send_vector(O(i)->vv);
            send_vector(O(i)->mv);

            /* Send each of the surfaces. */

            for (j = 0; j < n; ++j)
            {
                s = (struct object_surf *) vecget(O(i)->sv, j);

                send_index(s->mi);

                send_vector(s->fv);
                send_vector(s->ev);
            }

            /* Encapsulate this object in an entity. */

            return send_create_entity(TYPE_OBJECT, i);
        }
    }
    return -1;
}

void recv_create_object(void)
{
    struct object_surf *s;

    /* Unpack the object header. */

    int i = new_object();
    int n = recv_index();
    int j, k;

    O(i)->count = 1;

    recv_array(O(i)->bound, 6, sizeof (float));

    /* Unpack the vertices and materials. */

    O(i)->vv = recv_vector();
    O(i)->mv = recv_vector();

    /* Unpack each surface. */

    O(i)->sv = vecnew(n, sizeof (struct object_surf));

    for (j = 0; j < n; ++j)
        if ((k = vecadd(O(i)->sv)) >= 0)
        {
            s = (struct object_surf *) vecget(O(i)->sv, k);

            s->mi = recv_index();
            s->fv = recv_vector();
            s->ev = recv_vector();
        }

    /* Encapsulate this object in an entity. */

    recv_create_entity();
}

/*===========================================================================*/

static void init_object(int i)
{
    if (O(i)->state == 0)
    {
        /* Initialize the buffer object. */
    
        if (GL_has_vertex_buffer_object)
        {
            glGenBuffersARB(1, &O(i)->buffer);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, O(i)->buffer);

            glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                            vecnum(O(i)->vv) * sizeof (struct object_vert),
                            vecbuf(O(i)->vv), GL_STATIC_DRAW_ARB);
        }
    
        O(i)->state = 1;
    }
}

static void fini_object(int i)
{
    if (O(i)->state == 1)
    {
        /* Free the vertex buffer object. */

        if (GL_has_vertex_buffer_object)
            if (glIsBufferARB(O(i)->buffer))
                glDeleteBuffersARB(1, &O(i)->buffer);

        O(i)->buffer = 0;
        O(i)->state  = 0;
    }
}

/*---------------------------------------------------------------------------*/

static void draw_surface(const struct object_surf *s,
                         const struct object_mtrl *m, int flag, float alpha)
{
    float d[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    draw_image(m->image);

    /* Modulate the diffuse color by the current alpha. */

    d[0] = m->d[0];
    d[1] = m->d[1];
    d[2] = m->d[2];
    d[3] = m->d[3] * alpha;

    /* Apply the material properties. */

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,      d);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   m->a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  m->s);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  m->e);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, m->x);

    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        /* If this object is transparent, don't write depth      */
        /* and render back-facing and front-facing separately.   */

        if ((d[3] < 1) || (flag & FLAG_TRANSPARENT))
        {
            glDepthMask(GL_FALSE);
            glCullFace(GL_FRONT);

            if (vecnum(s->fv) > 0)
                glDrawElements(GL_TRIANGLES, 3 * vecnum(s->fv),
                               GL_UNSIGNED_INT,  vecbuf(s->fv));

            glCullFace(GL_BACK);
        }

        /* Render all faces and edges. */

        if (vecnum(s->fv) > 0)
            glDrawElements(GL_TRIANGLES, 3 * vecnum(s->fv),
                           GL_UNSIGNED_INT,  vecbuf(s->fv));
        if (vecnum(s->ev) > 0)
            glDrawElements(GL_LINES,     2 * vecnum(s->ev),
                           GL_UNSIGNED_INT,  vecbuf(s->ev));
    }
    glPopAttrib();
}

static void draw_object(int j, int i, float a)
{
    GLsizei stride = sizeof (struct object_vert);

    int k, n = vecnum(O(i)->sv);
    struct object_surf *s;
    struct object_mtrl *m;

    init_object(i);

    glPushMatrix();
    {
        struct frustum F;

        /* Apply the local coordinate system transformation. */

        transform_entity(j);
        get_frustum(&F);

        /* Render this object. */

        if (tst_frustum(&F, O(i)->bound) >= 0)
        {
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT);
            {
                /* Bind a vertex buffer or array. */

                if (GL_has_vertex_buffer_object)
                {
                    glBindBufferARB(GL_ARRAY_BUFFER_ARB, O(i)->buffer);
                    glInterleavedArrays(GL_T2F_N3F_V3F, stride, 0);
                }
                else
                    glInterleavedArrays(GL_T2F_N3F_V3F, stride,
                                        vecget(O(i)->vv, 0));

                /* Draw each surface. */

                for (k = 0; k < n; ++k)
                {
                    s = (struct object_surf *) vecget(O(i)->sv, k);
                    m = (struct object_mtrl *) vecget(O(i)->mv, s->mi);
  
                    draw_surface(s, m, get_entity_flag(j),
                                       get_entity_alpha(j) * a);
                }
            }
            glPopAttrib();
            glPopClientAttrib();
        }

        /* Render all child entities in this coordinate system. */

        draw_entity_tree(j, a * get_entity_alpha(j));
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

static void dupe_object(int i)
{
    O(i)->count++;
}

static void free_object(int i)
{
    int j;

    if (--O(i)->count == 0)
    {
        fini_object(i);

        for (j = 0; j < vecnum(O(i)->mv); ++j)
            free_image(((struct object_mtrl *) vecget(O(i)->mv, j))->image);

        for (j = 0; j < vecnum(O(i)->sv); ++j)
        {
            vecdel(((struct object_surf *) vecget(O(i)->sv, j))->fv);
            vecdel(((struct object_surf *) vecget(O(i)->sv, j))->ev);
        }

        vecdel(O(i)->mv);
        vecdel(O(i)->vv);
        vecdel(O(i)->sv);

        memset(O(i), 0, sizeof (struct object));
    }
}

/*===========================================================================*/

static struct entity_func object_func = {
    "object",
    init_object,
    fini_object,
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
