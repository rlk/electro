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
#include "buffer.h"
#include "entity.h"
#include "image.h"
#include "event.h"
#include "utility.h"
#include "object.h"

/*---------------------------------------------------------------------------*/
/* Object entity storage                                                     */

#define OMAXINIT 128

static struct object *O;
static int            O_max;

static int object_exists(int od)
{
    return (O && 0 <= od && od < O_max && O[od].count);
}

static int alloc_object(void)
{
    return balloc((void **) &O, &O_max, sizeof (struct object), object_exists);
}

/*---------------------------------------------------------------------------*/
/* Vector loader caches                                                      */

static float *tv = NULL;
static int    tc = 0;
static int    tm = 0;

static float *nv = NULL;
static int    nc = 0;
static int    nm = 0;

static float *vv = NULL;
static int    vc = 0;
static int    vm = 0;

static int   *iv = NULL;
static int    ic = 0;
static int    im = 0;

/*---------------------------------------------------------------------------*/
/* Object element loader caches                                              */

static struct object_mtrl *mtrlv = NULL;
static int                 mtrlc = 0;
static int                 mtrlm = 0;

static struct object_vert *vertv = NULL;
static int                 vertc = 0;
static int                 vertm = 0;

static struct object_face *facev = NULL;
static int                 facec = 0;
static int                 facem = 0;

static struct object_edge *edgev = NULL;
static int                 edgec = 0;
static int                 edgem = 0;

static struct object_surf *surfv = NULL;
static int                 surfc = 0;
static int                 surfm = 0;

/*---------------------------------------------------------------------------*/

static void *memdup(void *src, size_t num, size_t len)
{
    void *dst = NULL;

    if ((num * len > 0) && (dst = malloc(num * len)))
        memcpy(dst, src, num * len);

    return dst;
}

/*---------------------------------------------------------------------------*/

static int push_t(void)
{
    if (tc < tm)
        return tc++;
    else
    {
        size_t s = MAX(2 * tm, 256) * (sizeof (float) * 2);
        float *v;

        if ((v = (float *) realloc(tv, s)))
        {
            tv = v;
            tm = s / (sizeof (float) * 2);

            return tc++;
        }
    }
    return -1;
}

static int push_n(void)
{
    if (nc < nm)
        return nc++;
    else
    {
        size_t s = MAX(2 * nm, 256) * (sizeof (float) * 3);
        float *v;

        if ((v = (float *) realloc(nv, s)))
        {
            nv = v;
            nm = s / (sizeof (float) * 3);

            return nc++;
        }
    }
    return -1;
}

static int push_v(void)
{
    if (vc < vm)
        return vc++;
    else
    {
        size_t s = MAX(2 * vm, 256) * (sizeof (float) * 3);
        float *v;

        if ((v = (float *) realloc(vv, s)))
        {
            vv = v;
            vm = s / (sizeof (float) * 3);

            return vc++;
        }
    }
    return -1;
}

static int push_i(void)
{
    if (ic < im)
        return ic++;
    else
    {
        size_t s = MAX(2 * im, 256) * (sizeof (int) * 4);
        int *v;

        if ((v = (int *) realloc(iv, s)))
        {
            iv = v;
            im = s / (sizeof (int) * 4);

            return ic++;
        }
    }
    return -1;
}

/*---------------------------------------------------------------------------*/

static int push_mtrl(void)
{
    if (mtrlc < mtrlm)
        return mtrlc++;
    else
    {
        size_t s = MAX(2 * mtrlm, 4) * sizeof (struct object_mtrl);
        struct object_mtrl *v;

        if ((v = (struct object_mtrl *) realloc(mtrlv, s)))
        {
            mtrlv = v;
            mtrlm = s / sizeof (struct object_mtrl);

            return mtrlc++;
        }
    }
    return -1;
}

static int push_vert(void)
{
    if (vertc < vertm)
        return vertc++;
    else
    {
        size_t s = MAX(2 * vertm, 256) * sizeof (struct object_vert);
        struct object_vert *v;

        if ((v = (struct object_vert *) realloc(vertv, s)))
        {
            vertv = v;
            vertm = s / sizeof (struct object_vert);

            return vertc++;
        }
    }
    return -1;
}

static int push_face(void)
{
    if (facec < facem)
        return facec++;
    else
    {
        size_t s = MAX(2 * facem, 256) * sizeof (struct object_face);
        struct object_face *v;

        if ((v = (struct object_face *) realloc(facev, s)))
        {
            facev = v;
            facem = s / sizeof (struct object_face);

            return facec++;
        }
    }
    return -1;
}

static int push_edge(void)
{
    if (edgec < edgem)
        return edgec++;
    else
    {
        size_t s = MAX(2 * edgem, 256) * sizeof (struct object_edge);
        struct object_edge *v;

        if ((v = (struct object_edge *) realloc(edgev, s)))
        {
            edgev = v;
            edgem = s / sizeof (struct object_edge);

            return edgec++;
        }
    }
    return -1;
}

static int push_surf(void)
{
    if (surfc < surfm)
        return surfc++;
    else
    {
        size_t s = MAX(2 * surfm, 256) * sizeof (struct object_surf);
        struct object_surf *v;

        if ((v = (struct object_surf *) realloc(surfv, s)))
        {
            surfv = v;
            surfm = s / sizeof (struct object_surf);

            return surfc++;
        }
    }
    return -1;
}

/*---------------------------------------------------------------------------*/

static int read_newmtl(const char *name)
{
    char str[MAXSTR];
    int  i;

    if ((i = push_mtrl()) >= 0)
    {
        sscanf(name, "%s", str);

        mtrlv[i].name  = memdup(str, 1, strlen(str) + 1);
        mtrlv[i].image = 0;

        /* Default diffuse */

        mtrlv[i].d[0] = 0.8f;
        mtrlv[i].d[1] = 0.8f;
        mtrlv[i].d[2] = 0.8f;
        mtrlv[i].d[3] = 0.8f;

        /* Default ambient */

        mtrlv[i].a[0] = 0.2f;
        mtrlv[i].a[1] = 0.2f;
        mtrlv[i].a[2] = 0.2f;
        mtrlv[i].a[3] = 1.0f;

        /* Default specular */

        mtrlv[i].s[0] = 0.0f;
        mtrlv[i].s[1] = 0.0f;
        mtrlv[i].s[2] = 0.0f;
        mtrlv[i].s[3] = 1.0f;

        /* Default emmisive */

        mtrlv[i].e[0] = 0.0f;
        mtrlv[i].e[1] = 0.0f;
        mtrlv[i].e[2] = 0.0f;
        mtrlv[i].e[3] = 1.0f;

        /* Default shininess */

        mtrlv[i].x[0] = 0.0f;
    }

    return i;
}

static void read_map_Kd(const char *line, int i)
{
    if (mtrlv)
    {
        char name[MAXSTR];

        if (sscanf(line, "%s", name) == 1)
            mtrlv[i].image = send_create_image(name);
    }
}

static void read_Kd(const char *line, int i)
{
    if (mtrlv) sscanf(line, "%f %f %f", mtrlv[i].d + 0,
                                        mtrlv[i].d + 1,
                                        mtrlv[i].d + 2);
}

static void read_Ka(const char *line, int i)
{
    if (mtrlv) sscanf(line, "%f %f %f", mtrlv[i].a + 0,
                                        mtrlv[i].a + 1,
                                        mtrlv[i].a + 2);
}

static void read_Ks(const char *line, int i)
{
    if (mtrlv) sscanf(line, "%f %f %f", mtrlv[i].s + 0,
                                        mtrlv[i].s + 1,
                                        mtrlv[i].s + 2);
}

static void read_Ke(const char *line, int i)
{
    if (mtrlv) sscanf(line, "%f %f %f", mtrlv[i].e + 0,
                                        mtrlv[i].e + 1,
                                        mtrlv[i].e + 2);
}

static void read_Ns(const char *line, int i)
{
    if (mtrlv) sscanf(line, "%f", mtrlv[i].x + 0);
}

static void read_d(const char *line, int i)
{
    if (mtrlv) sscanf(line, "%f", mtrlv[i].d + 3);
}

static void read_mtl(const char *filename)
{
    char line[MAXSTR];
    FILE *fin;

    if ((fin = fopen(filename, "r")))
    {
        int i = -1;

        /* Process each line, invoking the handler for each keyword. */

        while (fgets(line, MAXSTR, fin))
        {
            if      (!strncmp(line, "newmtl", 6)) i = read_newmtl(line + 7);
            else if (!strncmp(line, "map_Kd", 6))     read_map_Kd(line + 7, i);

            else if (!strncmp(line, "Kd", 2)) read_Kd(line + 3, i);
            else if (!strncmp(line, "Ka", 2)) read_Ka(line + 3, i);
            else if (!strncmp(line, "Ks", 2)) read_Ks(line + 3, i);
            else if (!strncmp(line, "Ke", 2)) read_Ke(line + 3, i);
            else if (!strncmp(line, "Ns", 2)) read_Ns(line + 3, i);
            else if (!strncmp(line, "d",  1)) read_d (line + 2, i);
        }

        fclose(fin);
    }
    else error("MTL file '%s': %s", filename, system_error());
}

static int find_mtl(const char *name)
{
    int i;

    for (i = 0; i < mtrlc; ++i)
        if (strncmp(mtrlv[i].name, name, MAXSTR) == 0)
            return i;

    error("MTL: Unknown material '%s'", name);
    return 0;
}

/*---------------------------------------------------------------------------*/

static int read_indices(const char *line, int *vi, int *ti, int *ni)
{
    char vert[MAXSTR];
    int n = 0;

    *vi = 0;
    *ti = 0;
    *ni = 0;

    if (sscanf(line, "%s%n", vert, &n) >= 1)
    {
        char *texc = strstr(vert, "/") + 1;
        char *norm = strstr(texc, "/") + 1;

        *vi = atoi(vert);
        *ti = atoi(texc);
        *ni = atoi(norm);
    }
    return n;
}

static void read_vertices(const char *line)
{
    const char *c = line;
    int dc, i, j;

    int vi;
    int ti;
    int ni;

    /* Scan down the face string recording index set specifications. */

    while ((dc = read_indices(c, &vi, &ti, &ni)) && ((i = push_i()) >= 0))
    {
        iv[i * 4 + 0] = vi;
        iv[i * 4 + 1] = ti;
        iv[i * 4 + 2] = ni;

        /* If we've seen this index set before, note the associated vertex. */
        /*
        for (k = 0; k < i; ++k)
            if (iv[i * 4 + 0] == iv[k * 4 + 0] &&
                iv[i * 4 + 1] == iv[k * 4 + 1] &&
                iv[i * 4 + 2] == iv[k * 4 + 2])
            {
                iv[i * 4 + 3]  = iv[k * 4 + 3];
                break;
            }
        */


        /* If we haven't seen this index set, create a new vertex. */

        if (/*i == k &&*/ (j = push_vert()) >= 0)
        {
            /* Initialize vector data defaults. */

            vertv[j].v[0] = 0.0f;
            vertv[j].v[1] = 0.0f;
            vertv[j].v[2] = 0.0f;

            vertv[j].t[0] = 0.0f;
            vertv[j].t[1] = 0.0f;

            vertv[j].n[0] = 0.0f;
            vertv[j].n[1] = 0.0f;
            vertv[j].n[2] = 1.0f;

            /* Copy specified vector data from the cache. */

            if (ti > 0)
            {
                vertv[j].t[0] = tv[(ti -  1) * 2 + 0];
                vertv[j].t[1] = tv[(ti -  1) * 2 + 1];
            }
            if (ti < 0)
            {
                vertv[j].t[0] = tv[(tc - ti) * 2 + 0];
                vertv[j].t[1] = tv[(tc - ti) * 2 + 1];
            }

            if (ni > 0)
            {
                vertv[j].n[0] = nv[(ni -  1) * 3 + 0];
                vertv[j].n[1] = nv[(ni -  1) * 3 + 1];
                vertv[j].n[2] = nv[(ni -  1) * 3 + 2];
            }
            if (ni < 0)
            {
                vertv[j].n[0] = nv[(nc - ni) * 3 + 0];
                vertv[j].n[1] = nv[(nc - ni) * 3 + 1];
                vertv[j].n[2] = nv[(nc - ni) * 3 + 2];
            }

            if (vi > 0)
            {
                vertv[j].v[0] = vv[(vi -  1) * 3 + 0];
                vertv[j].v[1] = vv[(vi -  1) * 3 + 1];
                vertv[j].v[2] = vv[(vi -  1) * 3 + 2];
            }
            if (vi < 0)
            {
                vertv[j].v[0] = vv[(vc - vi) * 3 + 0];
                vertv[j].v[1] = vv[(vc - vi) * 3 + 1];
                vertv[j].v[2] = vv[(vc - vi) * 3 + 2];
            }

            /* Associate the index set with the new vertex. */

            iv[i * 4 + 3] = j;
        }

        c += dc;
    }
}

static void read_f(const char *line)
{
    int i, j, i0 = ic;

    /* Scan down the face string recording index set specifications. */

    read_vertices(line);

    /* Convert our N new index sets into N-2 new triangles. */

    for (i = i0; i < ic - 2; ++i)
        if ((j = push_face()) >= 0)
        {
            facev[j].vi[0] = iv[(i0   ) * 4 + 3];
            facev[j].vi[1] = iv[(i + 1) * 4 + 3];
            facev[j].vi[2] = iv[(i + 2) * 4 + 3];
        }
}

static void read_l(const char *line)
{
    int i, j, i0 = ic;

    /* Scan down the edge string recording index set specifications. */

    read_vertices(line);

    /* Convert our N new index sets into N-2 new triangles. */

    for (i = i0; i < ic - 1; ++i)
        if ((j = push_edge()) >= 0)
        {
            edgev[j].vi[0] = iv[(i + 0) * 4 + 3];
            edgev[j].vi[1] = iv[(i + 1) * 4 + 3];
        }
}

/*---------------------------------------------------------------------------*/

static int read_g(int i, int m)
{
    int j;

    if (i >= 0)
    {
        /* Close the existing surface by copying the face and edge cache. */

        surfv[i].fc = facec;
        surfv[i].ec = edgec;

        surfv[i].fv =
            (struct object_face *) memdup(facev,
                                          facec, sizeof (struct object_face));
        surfv[i].ev =
            (struct object_edge *) memdup(edgev,
                                          edgec, sizeof (struct object_edge));

        facec = 0;
        edgec = 0;
    }

    if ((j = push_surf()) >= 0)
    {
        /* Initialize a new empty surface. */

        surfv[j].mi =    m;
        surfv[j].fc =    0;
        surfv[j].ec =    0;
        surfv[j].fv = NULL;
    }

    return j;
}

static void read_mtllib(const char *line)
{
    char file[MAXSTR];

    sscanf(line, "%s", file);
    read_mtl(file);
}

static int read_usemtl(const char *line, int i)
{
    char name[MAXSTR];
    int m = -1;

    if (sscanf(line, "%s", name) == 1)
        m = find_mtl(name);

    if (i >= 0)
        surfv[i].mi = m;

    return m;
}

static void read_vt(const char *line)
{
    int i;

    if ((i = push_t()) >= 0)
        sscanf(line, "%f %f", tv + i * 2 + 0,
                              tv + i * 2 + 1);
}

static void read_vn(const char *line)
{
    int i;

    if ((i = push_n()) >= 0)
        sscanf(line, "%f %f %f", nv + i * 3 + 0,
                                 nv + i * 3 + 1,
                                 nv + i * 3 + 2);
}

static void read_v(const char *line)
{
    int i;

    if ((i = push_v()) >= 0)
        sscanf(line, "%f %f %f", vv + i * 3 + 0,
                                 vv + i * 3 + 1,
                                 vv + i * 3 + 2);
}

/*---------------------------------------------------------------------------*/

static int read_obj(const char *filename, struct object *o)
{
    char line[MAXSTR];
    FILE *fin;

    /* Initialize the vector and element caches. */

    vc    = 0;
    tc    = 0;
    nc    = 0;
    ic    = 0;

    mtrlc = 0;
    vertc = 0;
    facec = 0;
    edgec = 0;
    surfc = 0;

    if ((fin = fopen(filename, "r")))
    {
        int i = -1;
        int m = -1;

        /* Process each line, invoking the handler for each keyword. */

        while (fgets(line, MAXSTR, fin))
        {
            if      (!strncmp(line, "mtllib", 6))     read_mtllib(line + 7);
            else if (!strncmp(line, "usemtl", 6)) m = read_usemtl(line + 7, i);

            else if (!strncmp(line, "g",  1)) i = read_g(i, m);
            else if (!strncmp(line, "f",  1)) read_f (line + 2);
            else if (!strncmp(line, "l",  1)) read_l (line + 2);
            else if (!strncmp(line, "vt", 2)) read_vt(line + 3);
            else if (!strncmp(line, "vn", 2)) read_vn(line + 3);
            else if (!strncmp(line, "v",  1)) read_v (line + 2);
        }

        /* Close out the last group being read. */

        i = read_g(i, m);

        /* Close out the object by copying the element caches. */

        o->vv = (struct object_vert *)
            memdup(vertv, vertc, sizeof (struct object_vert));
        o->mv = (struct object_mtrl *)
            memdup(mtrlv, mtrlc, sizeof (struct object_mtrl));
        o->sv = (struct object_surf *)
            memdup(surfv, surfc, sizeof (struct object_surf));

        o->vc = vertc;
        o->mc = mtrlc;
        o->sc = i;

        fclose(fin);

        return 1;
    }
    else error("OBJ file '%s': %s", filename, system_error());

    return 0;
}

/*---------------------------------------------------------------------------*/

int init_object(void)
{
    if ((O = (struct object *) calloc(OMAXINIT, sizeof (struct object))))
        O_max = OMAXINIT;

    return 1;
}

void draw_object(int id, int od, const struct frustum *F0, float a)
{
    GLsizei stride = sizeof (struct object_vert);
    struct frustum F1;

    if (object_exists(od))
    {
        glPushMatrix();
        {
            const float d[3] = { 0.0f, 0.0f, 0.0f };

            /* Apply the local coordinate system transformation. */

            transform_entity(id, &F1, F0, d);

            /* Render this object. */

            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glPushAttrib(GL_LIGHTING_BIT |
                         GL_TEXTURE_BIT  |
                         GL_DEPTH_BUFFER_BIT);
            {
                int si;

                if (GL_has_vertex_buffer_object)
                {
                    glBindBufferARB(GL_ARRAY_BUFFER_ARB, O[od].buffer);
                    glInterleavedArrays(GL_T2F_N3F_V3F, stride, 0);
                }
                else
                    glInterleavedArrays(GL_T2F_N3F_V3F, stride, O[od].vv);

                /* If this object is transparent, don't write depth. */

                if (a * get_entity_alpha(id) < 1.0)
                    glDepthMask(GL_FALSE);

                for (si = 0; si < O[od].sc; ++si)
                {
                    float d[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                    float z[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

                    if (O[od].sv[si].mi >= 0)
                    {
                        struct object_mtrl *m = O[od].mv + O[od].sv[si].mi;
                        float d[4];

                        draw_image(m->image);

                        /* Modulate the diffuse color by the current alpha. */

                        d[0] = m->d[0];
                        d[1] = m->d[1];
                        d[2] = m->d[2];
                        d[3] = m->d[3] * a * get_entity_alpha(id);

                        glColor4fv(d);

                        /* Apply the material properties. */

                        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,      d);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   m->a);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  m->s);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  m->e);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, m->x);
                    }
                    else
                    {
                        glBindTexture(GL_TEXTURE_2D, 0);

                        d[3] = a * get_entity_alpha(id);

                        glColor4fv(d);

                        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   d);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   z);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  z);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  z);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, d);
                    }

                    /* Draw everything. */

                    if (O[od].sv[si].fc > 0)
                        glDrawElements(GL_TRIANGLES, 3 * O[od].sv[si].fc,
                                       GL_UNSIGNED_INT,  O[od].sv[si].fv);
                    if (O[od].sv[si].ec > 0)
                        glDrawElements(GL_LINES,     2 * O[od].sv[si].ec,
                                       GL_UNSIGNED_INT,  O[od].sv[si].ev);
                }
            }
            glPopAttrib();
            glPopClientAttrib();

            /* Render all child entities in this coordinate system. */

            draw_entity_list(id, &F1, a * get_entity_alpha(id));
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/

static void create_object(int od)
{
    if (GL_has_vertex_buffer_object)
    {
        glGenBuffersARB(1, &O[od].buffer);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, O[od].buffer);

        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        O[od].vc * sizeof (struct object_vert),
                        O[od].vv, GL_STATIC_DRAW_ARB);
    }
}

int send_create_object(const char *filename)
{
    int od;
    int si;

    if (O && (od = alloc_object()) >= 0)
    {
        /* If the file exists and is successfully read... */

        if ((read_obj(filename, O + od)))
        {
            O[od].count = 1;

            /* Pack the object header. */

            pack_event(EVENT_CREATE_OBJECT);
            pack_index(od);
            pack_index(O[od].vc);
            pack_index(O[od].mc);
            pack_index(O[od].sc);

            /* Pack the vertices and materials. */

            pack_alloc(O[od].vc * sizeof (struct object_vert), O[od].vv);
            pack_alloc(O[od].mc * sizeof (struct object_mtrl), O[od].mv);

            /* Pack each of the surfaces. */

            for (si = 0; si < O[od].sc; ++si)
            {
                struct object_surf *s = O[od].sv + si;

                pack_index(s->mi);
                pack_index(s->fc);
                pack_index(s->ec);
                pack_alloc(s->fc * sizeof (struct object_face), s->fv);
                pack_alloc(s->ec * sizeof (struct object_edge), s->ev);
            }

            create_object(od);

            /* Encapsulate this object in an entity. */

            return send_create_entity(TYPE_OBJECT, od);
        }
    }
    return -1;
}

void recv_create_object(void)
{
    int od = unpack_index();
    int si;

    O[od].count = 1;

    /* Unpack the object header. */

    O[od].vc = unpack_index();
    O[od].mc = unpack_index();
    O[od].sc = unpack_index();

    /* Unpack the vertices and materials.  Allocate space for surfaces. */

    O[od].vv = unpack_alloc(O[od].vc * sizeof (struct object_vert));
    O[od].mv = unpack_alloc(O[od].mc * sizeof (struct object_mtrl));

    O[od].sv = (struct object_surf *)
        calloc(O[od].sc, sizeof (struct object_surf));

    /* Unpack each surface. */

    for (si = 0; si < O[od].sc; ++si)
    {
        struct object_surf *s = O[od].sv + si;

        s->mi = unpack_index();
        s->fc = unpack_index();
        s->ec = unpack_index();
        s->fv = unpack_alloc(s->fc * sizeof (struct object_face));
        s->ev = unpack_alloc(s->ec * sizeof (struct object_edge));
    }

    create_object(od);

    /* Encapsulate this object in an entity. */

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/
/* These may only be called by create_clone and delete_entity, respectively. */

void clone_object(int od)
{
    if (object_exists(od))
        O[od].count++;
}

void delete_object(int od)
{
    if (object_exists(od))
    {
        O[od].count--;

        if (O[od].count == 0)
        {
            int si;

            if (glIsBufferARB(O[od].buffer))
                glDeleteBuffersARB(1, &O[od].buffer);

            for (si = 0; si < O[od].sc; ++si)
                if (O[od].sv[si].fv) free(O[od].sv[si].fv);

            if (O[od].mv) free(O[od].mv);
            if (O[od].vv) free(O[od].vv);

            memset(O + od, 0, sizeof (struct object));
        }
    }
}

/*---------------------------------------------------------------------------*/
