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
#include "shared.h"
#include "server.h"
#include "entity.h"
#include "object.h"

#define MAXSTR 256

/*---------------------------------------------------------------------------*/
/* Object entity storage                                                     */

static struct object *O     = NULL;
static int            O_max =   64;

static int object_exists(int od)
{
    return (O && ((od == 0) || (0 < od && od < O_max && O[od].vc)));
}

/*---------------------------------------------------------------------------*/
/* Vector loader caches                                                      */

#define MAXV 32768

static float vv[MAXV][3];
static float nv[MAXV][3];
static float tv[MAXV][2];
static int   iv[MAXV][4];

static int   vc;
static int   nc;
static int   tc;
static int   ic;

/*---------------------------------------------------------------------------*/
/* Object element loader caches                                              */

#define MAXMTRL    64
#define MAXVERT 32768
#define MAXFACE 16384
#define MAXSURF   128

static char               namev[MAXMTRL][MAXSTR];
static struct object_mtrl mtrlv[MAXMTRL];
static struct object_vert vertv[MAXVERT];
static struct object_face facev[MAXFACE];
static struct object_surf surfv[MAXSURF];

static int mtrlc;
static int vertc;
static int facec;
static int surfc;

/*---------------------------------------------------------------------------*/

static void *memdup(void *src, size_t num, size_t len)
{
    void *dst = NULL;

    if ((num * len > 0) && (dst = malloc(num * len)))
        memcpy(dst, src, num * len);

    return dst;
}

/*---------------------------------------------------------------------------*/

static void read_newmtl(const char *name)
{
    if (mtrlc < MAXMTRL)
    {
        mtrlc++;

        sscanf(name, "%s", namev[mtrlc]);

        /* Default diffuse */

        mtrlv[mtrlc].d[0] = 0.8f;
        mtrlv[mtrlc].d[1] = 0.8f;
        mtrlv[mtrlc].d[2] = 0.8f;
        mtrlv[mtrlc].d[3] = 0.8f;

        /* Default ambient */

        mtrlv[mtrlc].a[0] = 0.2f;
        mtrlv[mtrlc].a[1] = 0.2f;
        mtrlv[mtrlc].a[2] = 0.2f;
        mtrlv[mtrlc].a[3] = 1.0f;

        /* Default specular */

        mtrlv[mtrlc].s[0] = 0.0f;
        mtrlv[mtrlc].s[1] = 0.0f;
        mtrlv[mtrlc].s[2] = 0.0f;
        mtrlv[mtrlc].s[3] = 1.0f;

        /* Default emmisive */

        mtrlv[mtrlc].e[0] = 0.0f;
        mtrlv[mtrlc].e[1] = 0.0f;
        mtrlv[mtrlc].e[2] = 0.0f;
        mtrlv[mtrlc].e[3] = 1.0f;

        /* Default shininess */

        mtrlv[mtrlc].x[0] = 0.0f;
    }
}

static void read_map_Kd(const char *line)
{
}

static void read_Kd(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f %f %f", mtrlv[mtrlc].d + 0,
                                 mtrlv[mtrlc].d + 1,
                                 mtrlv[mtrlc].d + 2);
}

static void read_Ka(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f %f %f", mtrlv[mtrlc].a + 0,
                                 mtrlv[mtrlc].a + 1,
                                 mtrlv[mtrlc].a + 2);
}

static void read_Ks(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f %f %f", mtrlv[mtrlc].s + 0,
                                 mtrlv[mtrlc].s + 1,
                                 mtrlv[mtrlc].s + 2);
}

static void read_Ke(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f %f %f", mtrlv[mtrlc].e + 0,
                                 mtrlv[mtrlc].e + 1,
                                 mtrlv[mtrlc].e + 2);
}

static void read_Ns(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f", mtrlv[mtrlc].x + 0);
}

static void read_d(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f", mtrlv[mtrlc].d + 3);
}

static void read_mtl(const char *filename)
{
    char line[MAXSTR];
    FILE *fin;

    if ((fin = fopen(filename, "r")))
    {
        /* Process each line, invoking the handler for each keyword. */

        while (fgets(line, MAXSTR, fin))
        {
            if      (strncmp(line, "newmtl", 6) == 0) read_newmtl(line + 7);
            else if (strncmp(line, "map_Kd", 6) == 0) read_map_Kd(line + 7);

            else if (strncmp(line, "Kd", 2) == 0) read_Kd(line + 3);
            else if (strncmp(line, "Ka", 2) == 0) read_Ka(line + 3);
            else if (strncmp(line, "Ks", 2) == 0) read_Ks(line + 3);
            else if (strncmp(line, "Ke", 2) == 0) read_Ke(line + 3);
            else if (strncmp(line, "Ns", 2) == 0) read_Ns(line + 3);
            else if (strncmp(line, "d",  1) == 0) read_d (line + 2);
        }

        /* Close out the last material being read. */

        read_newmtl("");

        fclose(fin);
    }
}

static int find_mtl(const char *name)
{
    int i;

    for (i = 0; i < mtrlc; ++i)
        if (strncmp(namev[i], name, MAXSTR) == 0)
            return i;

    return 0;
}

/*---------------------------------------------------------------------------*/

static int read_indices(const char *line, int *vi, int *ti, int *ni)
{
    int n;

    *vi = 0;
    *ti = 0;
    *ni = 0;

    /* Allow either vertex/texcoord/normal or missing texcoord. */

    if (sscanf(line, "%d/%d/%d%n", vi, ti, ni, &n) >= 3) return n;
    if (sscanf(line, "%d//%d%n",   vi,     ni, &n) >= 2) return n;

    return 0;
}

static void read_f(const char *line)
{
    const char *c = line;
    int dc, i, i0 = ic;

    /* Scan down the face string recording index set specifications. */

    while ((ic < MAXV) && (dc = read_indices(c, &iv[ic][0],
                                                &iv[ic][1],
                                                &iv[ic][2])))
    {
        iv[ic][3] = ic;

        /* If we've seen this index set before, note the associated vertex. */

        for (i = 0; i < ic; ++i)
            if (iv[ic][0] == iv[i][0] &&
                iv[ic][1] == iv[i][1] &&
                iv[ic][2] == iv[i][2])
            {
                iv[ic][3]  = iv[i][3];
                break;
            }

        /* If we haven't seen this index set, create a new vertex. */

        if (i == ic)
        {
            int vi = iv[ic][0];
            int ti = iv[ic][1];
            int ni = iv[ic][2];

            /* Initialize vector data defaults. */

            vertv[vertc].v[0] = 0.0f;
            vertv[vertc].v[1] = 0.0f;
            vertv[vertc].v[2] = 0.0f;

            vertv[vertc].t[0] = 0.0f;
            vertv[vertc].t[1] = 0.0f;

            vertv[vertc].n[0] = 0.0f;
            vertv[vertc].n[1] = 0.0f;
            vertv[vertc].n[2] = 1.0f;

            /* Copy specified vector data from the cache. */

            if (vi > 0)
            {
                vertv[vertc].v[0] = vv[vi -  1][0];
                vertv[vertc].v[1] = vv[vi -  1][1];
                vertv[vertc].v[2] = vv[vi -  1][2];
            }
            if (vi < 0)
            {
                vertv[vertc].v[0] = vv[vc - vi][0];
                vertv[vertc].v[1] = vv[vc - vi][1];
                vertv[vertc].v[2] = vv[vc - vi][2];
            }

            if (ti > 0)
            {
                vertv[vertc].t[0] = tv[ti -  1][0];
                vertv[vertc].t[1] = tv[ti -  1][1];
            }
            if (ti < 0)
            {
                vertv[vertc].t[0] = tv[tc - ti][0];
                vertv[vertc].t[1] = tv[tc - ti][1];
            }

            if (ni > 0)
            {
                vertv[vertc].n[0] = nv[ni -  1][0];
                vertv[vertc].n[1] = nv[ni -  1][1];
                vertv[vertc].n[2] = nv[ni -  1][2];
            }
            if (ni < 0)
            {
                vertv[vertc].n[0] = nv[nc - ni][0];
                vertv[vertc].n[1] = nv[nc - ni][1];
                vertv[vertc].n[2] = nv[nc - ni][2];
            }

            /* Associate the index set with the new vertex. */

            iv[ic][3] = vertc++;
        }

        c  += dc;
        ic +=  1;
    }

    /* Convert our N new index sets into N-2 new triangles. */

    for (i = i0; i < ic - 2 && facec < MAXFACE; ++i)
    {
        facev[facec].vi[0] = iv[i0   ][3];
        facev[facec].vi[1] = iv[i + 1][3];
        facev[facec].vi[2] = iv[i + 2][3];

        facec++;
    }
}

/*---------------------------------------------------------------------------*/

static void read_g(void)
{
    if (surfc >= 0)
    {
        /* Close out the existing surface by copying the face cache. */

        surfv[surfc].fc = facec;
        surfv[surfc].fv =
            (struct object_face *) memdup(facev,
                                          facec, sizeof (struct object_face));
        facec = 0;
    }

    if (surfc < MAXSURF)
    {
        /* Initialize a new empty surface. */

        surfc++;

        surfv[surfc].mi =    0;
        surfv[surfc].fc =    0;
        surfv[surfc].fv = NULL;
    }
}

static void read_mtllib(const char *line)
{
    char file[MAXSTR];

    sscanf(line, "%s", file);
    read_mtl(file);
}

static void read_usemtl(const char *line)
{
    if (surfc >= 0)
    {
        char name[MAXSTR];

        sscanf(line, "%s", name);
        surfv[surfc].mi = find_mtl(name);
    }
}

static void read_vt(const char *line)
{
    if (tc < MAXV && sscanf(line, "%f %f", &tv[tc][0],
                                           &tv[tc][1]) == 2) tc++;
}

static void read_vn(const char *line)
{
    if (nc < MAXV && sscanf(line, "%f %f %f", &nv[nc][0],
                                              &nv[nc][1],
                                              &nv[nc][2]) == 3) nc++;
}

static void read_v(const char *line)
{
    if (vc < MAXV && sscanf(line, "%f %f %f", &vv[vc][0],
                                              &vv[vc][1],
                                              &vv[vc][2]) == 3) vc++;
}

/*---------------------------------------------------------------------------*/

static void read_obj(const char *filename, struct object *o)
{
    char line[MAXSTR];
    FILE *fin;

    /* Initialize the vector and element caches. */

    vc = 0;
    tc = 0;
    nc = 0;
    ic = 0;

    mtrlc = -1;
    vertc =  0;
    facec =  0;
    surfc = -1;

    if ((fin = fopen(filename, "r")))
    {
        /* Process each line, invoking the handler for each keyword. */

        while (fgets(line, MAXSTR, fin))
        {
            if      (strncmp(line, "mtllib", 6) == 0) read_mtllib(line + 7);
            else if (strncmp(line, "usemtl", 6) == 0) read_usemtl(line + 7);

            else if (strncmp(line, "g",  1) == 0) read_g ();
            else if (strncmp(line, "f",  1) == 0) read_f (line + 2);
            else if (strncmp(line, "vt", 2) == 0) read_vt(line + 3);
            else if (strncmp(line, "vn", 2) == 0) read_vn(line + 3);
            else if (strncmp(line, "v",  1) == 0) read_v (line + 2);
        }
        fclose(fin);
    }

    /* Close out the last group being read. */

    read_g();

    /* Close out the object by copying the element caches. */

    o->vv = (struct object_vert *) memdup(vertv,
                                          vertc, sizeof (struct object_vert));
    o->mv = (struct object_mtrl *) memdup(mtrlv,
                                          mtrlc, sizeof (struct object_mtrl));
    o->sv = (struct object_surf *) memdup(surfv,
                                          surfc, sizeof (struct object_surf));

    o->vc = vertc;
    o->mc = mtrlc;
    o->sc = surfc;
}

/*---------------------------------------------------------------------------*/
/* TODO: This is not strictly correct MPI type usage.  Fix. */

static void object_share_surf(struct object_surf *s)
{
    mpi_share_integer(1, &s->mi);
    mpi_share_integer(1, &s->fc);

    if (!mpi_isroot())
    {
        if (s->fc)
            s->fv = (struct object_face *)
                calloc(s->fc, sizeof (struct object_face));
    }

    if (s->fc && s->fv)
        mpi_share_byte(s->fc * sizeof (struct object_face), s->fv);
}

static void object_share(struct object *o)
{
    int si;

    mpi_share_integer(1, &o->vc);
    mpi_share_integer(1, &o->mc);
    mpi_share_integer(1, &o->sc);

    if (!mpi_isroot())
    {
        if (o->vc)
            o->vv = (struct object_vert *)
                calloc(o->vc, sizeof (struct object_vert));
        if (o->mc)
            o->mv = (struct object_mtrl *)
                calloc(o->mc, sizeof (struct object_mtrl));
        if (o->sc)
            o->sv = (struct object_surf *)
                calloc(o->sc, sizeof (struct object_surf));
    }

    if (o->vc && o->vv)
        mpi_share_byte(o->vc * sizeof (struct object_vert), o->vv);
    if (o->mc && o->mv)
        mpi_share_byte(o->mc * sizeof (struct object_mtrl), o->mv);

    for (si = 0; si < o->sc; ++si)
        object_share_surf(o->sv + si);
}

/*---------------------------------------------------------------------------*/

int object_create(const char *filename)
{
    int od;

    if (O && (od = buffer_unused(O_max, object_exists)) >= 0)
    {
        /* Initialize the new object. */

        if (mpi_isroot())
        {
            read_obj(filename, O + od);
            server_send(EVENT_OBJECT_CREATE);
        }

        /* Syncronize the new object. */

        mpi_share_integer(1, &od);

        object_share(O + od);

        /* Encapsulate this new object in an entity. */

        return entity_create(TYPE_OBJECT, od);
    }
    else if ((O = buffer_expand(O, &O_max, sizeof (struct object))))
        return object_create(filename);

    return -1;
}

/*---------------------------------------------------------------------------*/

void object_render(int id, int od)
{
    GLsizei stride = sizeof (struct object_vert);

    if (object_exists(od))
    {
        glPushMatrix();
        {
            /* Apply the local coordinate system transformation. */

            entity_transform(id);

            /* Render this object. */

            glPushAttrib(GL_LIGHTING_BIT);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            {
                int si;

                glInterleavedArrays(GL_T2F_N3F_V3F, stride, O[od].vv);

                for (si = 0; si < O[od].sc; ++si)
                {
                    const struct object_mtrl *m = O[od].mv + O[od].sv[si].mi;

                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   m->d);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   m->a);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  m->s);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  m->e);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, m->x);

                    glDrawElements(GL_TRIANGLES, 3 * O[od].sv[si].fc,
                                   GL_UNSIGNED_INT,  O[od].sv[si].fv);
                }
            }
            glPopClientAttrib();
            glPopAttrib();

            /* Render all child entities in this coordinate system. */

            entity_traversal(id);
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by the entity delete function. */

void object_delete(int od)
{
    int si;

    mpi_share_integer(1, &od);

    for (si = 0; si < O[od].sc; ++si)
        if (O[od].sv[si].fv) free(O[od].sv[si].fv);

    if (O[od].mv) free(O[od].mv);
    if (O[od].vv) free(O[od].vv);

    memset(O + od, 0, sizeof (struct object));
}

/*---------------------------------------------------------------------------*/
