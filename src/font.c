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

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include <stdlib.h>
#include <string.h>

#include "opengl.h"
#include "vector.h"
#include "buffer.h"
#include "utility.h"
#include "event.h"
#include "font.h"

/*---------------------------------------------------------------------------*/

#define MINGLYPH  32
#define MAXGLYPH 128
#define NUMGLYPH (MAXGLYPH - MINGLYPH)

#define SCALE 1024

#define TO_DBL(n) (((GLdouble) (n) / 64.0) / SCALE)

/*---------------------------------------------------------------------------*/

struct point
{
    GLdouble p[3];
};

struct glyph
{
    vector_t points;
    GLdouble space;
    GLuint   list;
};

struct font
{
    char     *filename;
    GLdouble  epsilon;
    GLboolean outline;

    GLdouble     kerns[NUMGLYPH][NUMGLYPH]; 
    struct glyph glyph[NUMGLYPH];
    int          state;
};

/*---------------------------------------------------------------------------*/

static vector_t font;
static int      curr;

static FT_Library library;

/*---------------------------------------------------------------------------*/

#define F(i) ((struct font *) vecget(font, i))

/*===========================================================================*/

static void new_point(GLdouble p[3], FT_Vector *to)
{
    p[0] = TO_DBL(to->x);
    p[1] = TO_DBL(to->y);
    p[2] = 0;
}

static void add_point(vector_t points, const GLdouble p[3])
{
    struct point *P;

    if ((P = (struct point *) vecget(points, vecadd(points))))
    {
        P->p[0] = p[0];
        P->p[1] = p[1];
        P->p[2] = p[2];
    }
}

static void get_point(GLdouble p[3], vector_t points)
{
    struct point *P;

    if ((P = (struct point *) vecget(points, vecnum(points) - 1)))
    {
        p[0] = P->p[0];
        p[1] = P->p[1];
        p[2] = P->p[2];
    }
}

/*---------------------------------------------------------------------------*/

void linear(GLdouble p[3], const GLdouble a[3],
                           const GLdouble b[3], GLdouble u)

{
    const GLdouble v = 1.0 - u;

    p[0] = a[0] * v + b[0] * u;
    p[1] = a[1] * v + b[1] * u;
    p[2] = a[2] * v + b[2] * u;
}

void conic(GLdouble p[3], const GLdouble a[3],
                          const GLdouble b[3],
                          const GLdouble c[3], GLdouble u)
{
    const GLdouble v = 1.0 - u;

    const GLdouble ka =       v * v;
    const GLdouble kb = 2.0 * u * v;
    const GLdouble kc =       u * u;

    p[0] = a[0] * ka + b[0] * kb + c[0] * kc;
    p[1] = a[1] * ka + b[1] * kb + c[1] * kc;
    p[2] = a[2] * ka + b[2] * kb + c[2] * kc;
}

void cubic(GLdouble p[3], const GLdouble a[3],
                          const GLdouble b[3],
                          const GLdouble c[3],
                          const GLdouble d[3], GLdouble u)
{
    const GLdouble v = 1.0 - u;

    const GLdouble ka =       v * v * v;
    const GLdouble kb = 3.0 * u * v * v;
    const GLdouble kc = 3.0 * u * u * v;
    const GLdouble kd =       u * u * u;

    p[0] = a[0] * ka + b[0] * kb + c[0] * kc + d[0] * kd;
    p[1] = a[1] * ka + b[1] * kb + c[1] * kc + d[1] * kd;
    p[2] = a[2] * ka + b[2] * kb + c[2] * kc + d[2] * kd;
}

/*---------------------------------------------------------------------------*/

static GLdouble epsilon = 0.001;

static void add_conic(const GLdouble la[3],
                      const GLdouble lc[3],
                      const GLdouble ca[3],
                      const GLdouble cb[3],
                      const GLdouble cc[3], GLdouble u0, GLdouble u1,
                      vector_t points)
{
    GLdouble lp[3];
    GLdouble cp[3];

    GLdouble uu = (u0 + u1) / 2;
    GLdouble d[3];

    linear(lp, la,     lc, 0.5);
    conic (cp, ca, cb, cc, uu);

    d[0] = lp[0] - cp[0];
    d[1] = lp[1] - cp[1];
    
    if (d[0] * d[0] + d[1] * d[1] > epsilon * epsilon)
    {
        add_conic(la, cp, ca, cb, cc, u0, uu, points);
        add_conic(cp, lc, ca, cb, cc, uu, u1, points);
    }
    else
        add_point(points, lc);
}


/*---------------------------------------------------------------------------*/

static int move_to(FT_Vector *to, vector_t points)
{
    GLdouble p[3];

    new_point(p, to);
    add_point(points, p);

    return 0;
}

static int line_to(FT_Vector *to, vector_t points)
{
    GLdouble p[3];

    new_point(p, to);
    add_point(points, p);

    return 0;
}

static int conic_to(FT_Vector *ct, FT_Vector *to, vector_t points)
{
    GLdouble a[3], b[3], c[3];

    get_point(a, points);
    new_point(b, ct);
    new_point(c, to);
    add_conic(a, c, a, b, c, 0, 1, points);

    return 0;
}

static int cubic_to(FT_Vector *control1,
                    FT_Vector *control2, FT_Vector *to, vector_t points)
{
    error("TTF only.  Cubic bezier curves are not supported.");
    return 0;
}

/*---------------------------------------------------------------------------*/

static void read_glyph(struct glyph *glyph, FT_GlyphSlot slot)
{
    FT_Outline_Funcs funcs;

    funcs.move_to  = (FT_Outline_MoveToFunc)  move_to;
    funcs.line_to  = (FT_Outline_LineToFunc)  line_to;
    funcs.conic_to = (FT_Outline_ConicToFunc) conic_to;
    funcs.cubic_to = (FT_Outline_CubicToFunc) cubic_to;
    funcs.shift    = 0;
    funcs.delta    = 0;

    glyph->points = vecnew(16, sizeof (struct point));
    glyph->space  = TO_DBL(slot->advance.x);

    FT_Outline_Decompose(&slot->outline, &funcs, glyph->points);
}

/*===========================================================================*/

void send_set_font(const char *filename, float e, int o)
{
    int r, c, i, n = vecnum(font);

    epsilon = (GLdouble) e;

    /* If the requested font is already loaded, select it and return. */

    for (i = 0; i < n; ++i)
        if (F(i)->epsilon == e &&
            F(i)->outline == o &&
            strcmp(F(i)->filename, filename) == 0)
        {
            curr = i;
            return;
        }

    /* Otherwise, load the new font. */

    if ((i = vecadd(font)) >= 0)
    {
        FT_Vector v;
        FT_Face face;
        void *buffer;
        size_t size;

        /* Find and load the font file. */

        if ((buffer = load_file(filename, FMODE_RB, &size)))
        {
            if ((FT_New_Memory_Face(library, buffer, size, 0, &face)) == 0)
            {
                /* This scale is arbitrary.  Final linegap is 1 GL unit. */

                FT_Set_Pixel_Sizes(face, 0, SCALE);

                /* Extract all necessary glyphs. */

                for (c = MINGLYPH; c < MAXGLYPH; ++c)
                    if (FT_Load_Glyph(face, FT_Get_Char_Index(face, c), 0) ==0)
                        read_glyph(&F(i)->glyph[c - MINGLYPH], face->glyph);

                /* Retrieve all kerning pairs. */

                for (r = MINGLYPH; r < MAXGLYPH; ++r)
                    for (c = MINGLYPH; c < MAXGLYPH; ++c)
                    {
                        FT_Get_Kerning(face, FT_Get_Char_Index(face, r),
                                             FT_Get_Char_Index(face, c),
                                             FT_KERNING_UNFITTED, &v);
                        F(i)->kerns[r - MINGLYPH][c - MINGLYPH] = TO_DBL(v.x);
                    }

                /* Select the successfully loaded font as current. */

                F(i)->filename = memdup(filename, strlen(filename) + 1, 1);
                F(i)->epsilon  = e;
                F(i)->outline  = o;
                curr           = i;

                /* Pack the font on the broadcast queue. */

                send_event(EVENT_SET_FONT);
                send_float(F(i)->epsilon);
                send_index(F(i)->outline);
                send_array(F(i)->kerns, NUMGLYPH * NUMGLYPH, sizeof (double));

                for (c = 0; c < NUMGLYPH; ++c)
                {
                    send_float (F(i)->glyph[c].space);
                    send_vector(F(i)->glyph[c].points);
                }
            }
            else error("Failure making sense of '%s'", filename);

            free(buffer);
        }
        else error("Failure loading '%s'", filename);
    }
}

void recv_set_font(void)
{
    int i, c;

    if ((i = vecadd(font)) >= 0)
    {
        F(i)->epsilon = recv_float();
        F(i)->outline = recv_index();

        recv_array(F(i)->kerns, NUMGLYPH * NUMGLYPH, sizeof (double));

        for (c = 0; c < NUMGLYPH; ++c)
        {
            F(i)->glyph[c].space  = recv_float();
            F(i)->glyph[c].points = recv_vector();
        }
    }
}

int get_font(void)
{
    return curr;
}

/*---------------------------------------------------------------------------*/

static void vertex(void *vertex_data)
{
    struct point *P = (struct point *) vertex_data;

    /* Generate texture coordinates for each vertex. */

    glTexCoord2d(P->p[0], P->p[1]);
    glVertex3d  (P->p[0], P->p[1], P->p[2]);
}

static void combine(GLdouble coords[3], void *vertex_data[3],
                    GLfloat  weight[4], void **out_data, void *polygon_data)
{
    vector_t points = (vector_t) polygon_data;

    /* Include the suggested combined vertex as a new vertex. */

    add_point(points, coords);
    *out_data = vecget(points, vecnum(points) - 1);
}

/*---------------------------------------------------------------------------*/

void init_font(int i)
{
    int j, k, l;

    if (F(i)->state == 0)
    {
        GLuint lists = glGenLists(NUMGLYPH);
        GLUtesselator *T;

        /* Set up a GLU tessalator to handle glyph outlines. */

        if ((T = gluNewTess()))
        {
            gluTessCallback(T, GLU_TESS_BEGIN,        (_GLUfuncptr) glBegin);
            gluTessCallback(T, GLU_TESS_COMBINE_DATA, (_GLUfuncptr) combine);
            gluTessCallback(T, GLU_TESS_VERTEX,       (_GLUfuncptr) vertex);
            gluTessCallback(T, GLU_TESS_END,          (_GLUfuncptr) glEnd);

            if (F(i)->outline)
                gluTessProperty(T, GLU_TESS_BOUNDARY_ONLY, GL_TRUE);

            /* Enumerate all glyphs. */

            for (j = MINGLYPH; j < MAXGLYPH; ++j)
            {
                struct glyph *glyph = &F(i)->glyph[j - MINGLYPH];

                if (glyph->points)
                {
                    glyph->list = lists + j - MINGLYPH;

                    /* Tessalate the glyph to a display list. */

                    glNewList(glyph->list, GL_COMPILE);
                    glNormal3f(0, 0, 1);

                    gluTessBeginPolygon(T, glyph->points);
                    {
                        for (k = 0, l = 0; k < vecnum(glyph->points); ++k)
                            if (l == k)
                            {
                                gluTessBeginContour(T);
                                gluTessVertex(T, vecget(glyph->points, k),
                                              vecget(glyph->points, k));
                            }
                            else if (veccmp(glyph->points, k, l) == 0)
                            {
                                gluTessEndContour(T);
                                l = k + 1;
                            }
                            else
                                gluTessVertex(T, vecget(glyph->points, k),
                                              vecget(glyph->points, k));
                    }
                    gluTessEndPolygon(T);
                    glEndList();
                }
            }
            gluDeleteTess(T);
        }
        F(i)->state = 1;
    }
}

void fini_font(int i)
{
    if (F(i)->state == 1)
    {
        struct glyph *glyph = &F(i)->glyph[MINGLYPH];

        glDeleteLists(glyph->list, NUMGLYPH);
        F(i)->state = 0;
    }
}

/*---------------------------------------------------------------------------*/

void draw_font(int i, const char *text)
{
    init_font(i);

    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    {
        int j, n = strlen(text);

        /* Glyph outlines are traversed clockwise. */

        glFrontFace(GL_CW);

        /* Enumerate each character of the given string. */

        for (j = 0; j < n; ++j)
        {
            struct glyph *glyph = F(i)->glyph + text[j] - MINGLYPH;
            GLdouble d  = glyph->space;

            /* Determine the kerning distance. */

            if (j < n - 1)
            {
                int r = (int) text[j]     - MINGLYPH;
                int c = (int) text[j + 1] - MINGLYPH;

                d += F(i)->kerns[r][c];
            }

            /* Invoke the glyph display, and advance the insertion point. */

            glCallList  (glyph->list);
            glTranslated(d, 0, 0);
        }
    }
    glPopAttrib();
    glPopMatrix();
}

void bbox_font(int i, const char *text, float bound[6])
{
    int j, n = strlen(text);

    GLdouble x = 0.0;

    bound[0] = bound[3] = 0;
    bound[1] = bound[4] = 0;
    bound[2] = bound[5] = 0;

    for (j = 0; j < n; ++j)
    {
        struct glyph *glyph = F(i)->glyph + text[j] - MINGLYPH;
        GLdouble d  = glyph->space;

        if (glyph->points)
        {
            int k, m = vecnum(glyph->points);

            /* Determine the kerning distance. */

            if (j < n - 1)
            {
                int r = (int) text[j]     - MINGLYPH;
                int c = (int) text[j + 1] - MINGLYPH;

                d += F(i)->kerns[r][c];
            }

            /* Include the current glyph in the bound. */

            for (k = 0; k < m; ++k)
            {
                GLdouble *p = ((struct point *) vecget(glyph->points, k))->p;

                bound[0] = MIN(bound[0], (float) (p[0] + x));
                bound[1] = MIN(bound[1], (float)  p[1]);
                bound[2] = MIN(bound[2], (float)  p[2]);
                bound[3] = MAX(bound[3], (float) (p[0] + x));
                bound[4] = MAX(bound[4], (float)  p[1]);
                bound[5] = MAX(bound[5], (float)  p[2]);
            }
        }
        /* Advance the insertion point. */

        x += d;
    }
}

/*===========================================================================*/

int startup_font(void)
{
    if ((font = vecnew(2, sizeof (struct font))))
    {
        if ((FT_Init_FreeType(&library)))
            error("FreeType2 init failure");

        curr = 0;
        return 1;
    }
    else
        return 0;
}
