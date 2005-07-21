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
};

static vector_t object;

/*---------------------------------------------------------------------------*/

static struct object *get_object(int i)
{
    return (struct object *) vecget(object, i);
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

static vector_t _tv;
static vector_t _nv;
static vector_t _vv;

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

        if (line) m->brush = send_create_brush(file, parse_name(line));

        m->fv = vecnew(1, sizeof (struct object_face));
        m->ev = vecnew(1, sizeof (struct object_edge));

        return m;
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

    int n = 0;
    int r = 0;

    /* Initialize the loader vector caches. */

    _tv = vecnew(1024, sizeof (struct vec2));
    _nv = vecnew(1024, sizeof (struct vec3));
    _vv = vecnew(1024, sizeof (struct vec3));

    /* Initialize the object element vectors. */

    o->vv = vecnew(1, sizeof (struct object_vert));
    o->mv = vecnew(1, sizeof (struct object_mesh));

    if ((fin = open_file(filename, "r")))
    {
        /* Create a default catch-all group using the default material. */

        struct object_mesh *m = read_usemtl(o->mv, NULL, NULL);
        const char         *F = NULL;
        
        /* Process each line, invoking the handler for each keyword. */

        while (fgets(L, MAXSTR, fin))
            if (sscanf(L, "%s%n", W, &n) >= 1)
            {
                char *V = L + n;

                if      (!strcmp(W, "mtllib")) F = read_mtllib(o->mv, V);
                else if (!strcmp(W, "usemtl")) m = read_usemtl(o->mv, V, F);

                else if (!strcmp(W, "f"))  read_f(o->vv, m->fv, V);
                else if (!strcmp(W, "l"))  read_l(o->vv, m->ev, V);

                else if (!strcmp(W, "vt")) read_vt(V);
                else if (!strcmp(W, "vn")) read_vn(V);
                else if (!strcmp(W, "v" )) read_v (V);

                r++;
            }

        fclose(fin);
    }
    else error("OBJ file '%s': %s", filename, system_error());

    /* Release the loader caches. */

    vecdel(_vv);
    vecdel(_nv);
    vecdel(_tv);

    return r;
}

/*---------------------------------------------------------------------------*/

int send_create_object(const char *filename)
{
    int i;

    if ((i = new_object()) >= 0)
    {
        struct object      *o = get_object(i);
        struct object_mesh *m = NULL;

        /* If the file exists and is successfully read... */

        if ((read_obj(filename, o)))
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

static void draw_mesh(const struct object_mesh *m, float alpha)
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
}

static void draw_object(int j, int i, int f, float a)
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

        if (test_entity_bbox(j) >= 0)
        {
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT);
            {
                GLsizei stride = sizeof (struct object_vert);

                int k, n = vecnum(o->mv);

                /* Bind a vertex buffer or array. */

                if (GL_has_vertex_buffer_object)
                {
                    glBindBufferARB(GL_ARRAY_BUFFER_ARB, o->buffer);
                    glInterleavedArrays(GL_T2F_N3F_V3F, stride, 0);
                }
                else
                    glInterleavedArrays(GL_T2F_N3F_V3F, stride,
                                        vecget(o->vv, 0));

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

static int bbox_object(int i, float bound[6])
{
    struct object *o = get_object(i);

    int j, n = vecnum(o->vv);

    /* Find the object's bounding box. */

    if (n > 0)
    {
        const float *v = ((struct object_vert *) vecget(o->vv, 0))->v;

        bound[0] = bound[3] = v[0];
        bound[1] = bound[4] = v[1];
        bound[2] = bound[5] = v[2];
    }
    else return 0;

    for (j = 0; j < n; ++j)
    {
        const float *v = ((struct object_vert *) vecget(o->vv, j))->v;

        bound[0] = MIN(v[0], bound[0]);
        bound[1] = MIN(v[1], bound[1]);
        bound[2] = MIN(v[2], bound[2]);
        bound[3] = MAX(v[0], bound[3]);
        bound[4] = MAX(v[1], bound[4]);
        bound[5] = MAX(v[2], bound[5]);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void dupe_object(int i)
{
    get_object(i)->count++;
}

static void free_object(int i)
{
    struct object *o = get_object(i);

    if (--o->count == 0)
    {
        int j;

        fini_object(i);

        for (j = 0; j < vecnum(o->mv); ++j)
        {
            struct object_mesh *m = vecget(o->mv, j);

            free_brush(m->brush);

            vecdel(m->fv);
            vecdel(m->ev);
        }

        vecdel(o->vv);
        vecdel(o->mv);
        memset(o, 0, sizeof (struct object));
    }
}

/*===========================================================================*/

static struct entity_func object_func = {
    "object",
    init_object,
    fini_object,
    bbox_object,
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
