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
#include "font.h"

/*---------------------------------------------------------------------------*/

#define MINGLYPH  32
#define MAXGLYPH 128

struct point
{
    GLdouble p[3];
};

struct glyph
{
    vector_t points;
    float    space;
    GLuint   list;
};

struct font
{
    char *filename;

    struct glyph glyph[MAXGLYPH - MINGLYPH];
    int          state;
};

/*---------------------------------------------------------------------------*/

static vector_t font;
static int      curr;

static FT_Library library;

/*---------------------------------------------------------------------------*/

#define F(i) ((struct font *) vecget(font, i))

/*===========================================================================*/

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

/*
static void get_point(vector_t points, GLdouble p[3])
{
    struct point *P;

    if ((P = (struct point *) vecget(points, vecnum(points) - 1)))
    {
        p[0] = P->p[0];
        p[1] = P->p[1];
        p[2] = P->p[2];
    }
}
*/

/*---------------------------------------------------------------------------*/
/*
void linear(GLdouble p[3], const GLdouble a[3],
                           const GLdouble b[3], GLdouble u)

{
    GLdouble v = (1 - u);

    p[0] = a[0] * v +
           b[0] * u;
    p[1] = a[1] * v +
           b[1] * u;
    p[2] = a[2] * v +
           b[2] * u;
}

void conic(GLdouble p[3], const GLdouble a[3],
                          const GLdouble b[3],
                          const GLdouble c[3], GLdouble u)
{
    GLdouble v = (1.0 - u);

    p[0] = a[0] * v * v +
       2 * b[0] * u * v +
           c[0] * u * u;
    p[1] = a[1] * v * v +
       2 * b[1] * u * v +
           c[1] * u * u;
    p[2] = a[2] * v * v +
       2 * b[2] * u * v +
           c[2] * u * u;
}

void cubic(GLdouble p[3], const GLdouble a[3],
                          const GLdouble b[3],
                          const GLdouble c[3],
                          const GLdouble d[3], GLdouble u)
{
    GLdouble v = (1 - u);

    p[0] = a[0] * v * v * v +
       3 * b[0] * u * v * v +
       3 * c[0] * u * u * v +
           d[0] * u * u * u;
    p[1] = a[1] * v * v * v +
       3 * b[1] * u * v * v +
       3 * c[1] * u * u * v +
           d[1] * u * u * u;
    p[2] = a[2] * v * v * v +
       3 * b[2] * u * v * v +
       3 * c[2] * u * u * v +
           d[2] * u * u * u;
}
*/
/*---------------------------------------------------------------------------*/

static GLdouble epsilon = 0.001;
/*
static void adapt_conic(const GLdouble la[3],
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
        adapt_conic(la, cp, ca, cb, cc, u0, uu, points);
        adapt_conic(cp, lc, ca, cb, cc, uu, u1, points);
    }
    else
        add_point(points, lc);
}
*/

/*---------------------------------------------------------------------------*/
/*
static int move_to(FT_Vector *to, vector_t points)
{
    GLdouble p[3];

    p[0] = (GLdouble) to->x / 64;
    p[1] = (GLdouble) to->y / 64;
    p[2] = 0;

    add_point(points, p);

    return 0;
}

static int line_to(FT_Vector *to, vector_t points)
{
    GLdouble p[3];

    p[0] = (GLdouble) to->x / 64;
    p[1] = (GLdouble) to->y / 64;
    p[2] = 0;

    add_point(points, p);

    return 0;
}

static int conic_to(FT_Vector *control, FT_Vector *to, vector_t points)
{
    GLdouble a[3], b[3], c[3];

    get_point(points, a);

    b[0] = (GLdouble) control->x / 64;
    b[1] = (GLdouble) control->y / 64;
    b[2] = 0;

    c[0] = (GLdouble) to->x      / 64;
    c[1] = (GLdouble) to->y      / 64;
    c[2] = 0;

    adapt_conic(a, c, a, b, c, 0, 1, points);

    return 0;
}

static int cubic_to(FT_Vector *control1,
                    FT_Vector *control2, FT_Vector *to, vector_t points)
{
    GLdouble p[3], a[3], b[3], c[3], d[3];

    get_point(points, a);

    b[0] = (GLdouble) control1->x / 64;
    b[1] = (GLdouble) control1->y / 64;
    b[2] = 0;

    c[0] = (GLdouble) control2->x / 64;
    c[1] = (GLdouble) control2->y / 64;
    c[2] = 0;

    d[0] = (GLdouble) to->x       / 64;
    d[1] = (GLdouble) to->y       / 64;
    d[2] = 0;

    cubic(p, a, b, c, d, 0.5);

    add_point(points, p);
    add_point(points, d);

    printf("cubic!\n");

    return 0;
}
*/
/*---------------------------------------------------------------------------*/
/*
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
    glyph->space  = (GLdouble) slot->advance.x / 64;

    FT_Outline_Decompose(&slot->outline, &funcs, glyph->points);
}
*/

FT_Vector midpt(FT_Vector a, FT_Vector b)
{
    FT_Vector p;

    p.x = (a.x + b.x) / 2;
    p.y = (a.y + b.y) / 2;

    return p;
}

void addpt(vector_t points, FT_Vector p)
{
    struct point *P;

    if ((P = (struct point *) vecget(points, vecadd(points))))
    {
        P->p[0] = (GLdouble) p.x / 64;
        P->p[1] = (GLdouble) p.y / 64;
        P->p[2] = 0;
    }
}

FT_Vector conic(FT_Vector a, FT_Vector b, FT_Vector c, float u)
{
    float v = 1.0f - u; 
    FT_Vector p;

    p.x = (a.x * v * v +
       2 * b.x * u * v +
           c.x * u * u);
    p.y = (a.y * v * v +
       2 * b.y * u * v +
           c.y * u * u);

    return p;
}

static void read_glyph(struct glyph *glyph, FT_GlyphSlot slot)
{
    FT_Outline *o = &slot->outline;

    short pc = 0;
    short pi = 0;
    short ci = 0;

    glyph->points = vecnew(16, sizeof (struct point));
    glyph->space  = (GLdouble) slot->advance.x / 64;

    if (o->n_points > 0)
    {
        FT_Vector init_p = o->points[0];
        FT_Vector last_p;
        FT_Vector virt_p;

        while (pi < o->n_points && ci < o->n_contours)
        {
            if (o->tags[pi] & 0x1)
            {
                addpt(glyph->points, o->points[pi]);
                last_p = o->points[pi];

                if (pc == 0)
                    init_p = last_p;
            }
            else if (o->tags[pi + 1] & 0x1)
            {
                addpt(glyph->points, conic(last_p, o->points[pi], o->points[pi + 1], 0.25f));
                addpt(glyph->points, conic(last_p, o->points[pi], o->points[pi + 1], 0.50f));
                addpt(glyph->points, conic(last_p, o->points[pi], o->points[pi + 1], 0.75f));

                last_p = o->points[pi];
            }
            else
            {
                virt_p = midpt(o->points[pi], o->points[pi + 1]);

                addpt(glyph->points, conic(last_p, o->points[pi], virt_p, 0.25f));
                addpt(glyph->points, conic(last_p, o->points[pi], virt_p, 0.50f));
                addpt(glyph->points, conic(last_p, o->points[pi], virt_p, 0.75f));
                addpt(glyph->points, virt_p);

                last_p = virt_p;
            }

            pc++;

            if (pi == o->contours[ci])
            {
                addpt(glyph->points, init_p);
                ci++;
                pc = 0;
            }

            pi++;
        }
    }
}

/*===========================================================================*/

void set_font(const char *filename, float e)
{
    int c, i, n = vecnum(font);

    epsilon = (GLdouble) e;

    /* If the requested font is already loaded, select it and return. */

    for (i = 0; i < n; ++i)
        if (strcmp(F(i)->filename, filename) == 0)
        {
            curr = i;
            return;
        }

    /* Otherwise, load the new font. */

    if ((i = vecadd(font)) >= 0)
    {
        FT_Face face;
        void *buffer;
        size_t size;

        /* Find and load the font file. */

        if ((buffer = load_file(filename, FMODE_RB, &size)))
        {
            if ((FT_New_Memory_Face(library, buffer, size, 0, &face)) == 0)
            {
                FT_Set_Pixel_Sizes(face, 0, 1);

                /* Extract all necessary glyphs. */

                for (c = MINGLYPH; c < MAXGLYPH; ++c)
                    if (FT_Load_Glyph(face, FT_Get_Char_Index(face, c), 0) ==0)
                        read_glyph(&F(i)->glyph[c - MINGLYPH], face->glyph);

                /* Select the successfully loaded font at current. */

                curr = i;
                F(i)->filename = memdup(filename, strlen(filename) + 1, 1);
            }
            else error("Failure making sense of '%s'", filename);

            free(buffer);
        }
        else error("Failure loading '%s'", filename);
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

void init_font(int i)
{
    int j, k, l;

    if (F(i)->state == 0)
    {
        GLuint lists = glGenLists(MAXGLYPH - MINGLYPH);
        GLUtesselator *T;

        /* Set up a GLU tessalator to handle glyph outlines. */

        if ((T = gluNewTess()))
        {
            gluTessCallback(T, GLU_TESS_BEGIN,        (_GLUfuncptr) glBegin);
            gluTessCallback(T, GLU_TESS_COMBINE_DATA, (_GLUfuncptr) combine);
            gluTessCallback(T, GLU_TESS_VERTEX,       (_GLUfuncptr) vertex);
            gluTessCallback(T, GLU_TESS_END,          (_GLUfuncptr) glEnd);

            gluTessProperty(T, GLU_TESS_BOUNDARY_ONLY, GL_TRUE);

            /* Enumerate all glyphs. */

            for (j = MINGLYPH; j < MAXGLYPH; ++j)
            {
                struct glyph *glyph = &F(i)->glyph[j - MINGLYPH];
                vector_t     points = glyph->points;

                glyph->list = lists + j - MINGLYPH;

                /* Tessalate the glyph to a display list. */

                glNewList(glyph->list, GL_COMPILE);
                glNormal3f(0, 0, 1);

                gluTessBeginPolygon(T, points);
                {
                    for (k = 0, l = 0; k < vecnum(points); ++k)
                        if (l == k)
                        {
                            gluTessBeginContour(T);
                            gluTessVertex(T, vecget(points, k),
                                             vecget(points, k));
                        }
                        else if (veccmp(glyph->points, k, l) == 0)
                        {
                            gluTessEndContour(T);
                            l = k + 1;
                        }
                        else
                            gluTessVertex(T, vecget(points, k),
                                             vecget(points, k));
                }
                gluTessEndPolygon(T);
                glEndList();
            }
            gluDeleteTess(T);
        }
        F(i)->state = 1;
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
            struct glyph *glyph = &F(i)->glyph[text[j] - MINGLYPH];

            /* Invoke the glyph display, and advance the insertion point. */

            glCallList  (glyph->list);
            glTranslatef(glyph->space, 0, 0);
        }
    }
    glPopAttrib();
    glPopMatrix();
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
