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
#include FT_IMAGE_H

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "opengl.h"
#include "vector.h"
#include "buffer.h"
#include "matrix.h"
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
    float    space;
    GLuint   fill;
    GLuint   line;
};

struct font
{
    char *filename;
    float epsilon;
    float outline;

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

static float epsilon = 0.001f;

static void add_conic(vector_t points,
                      const GLdouble la[3],
                      const GLdouble lc[3],
                      const GLdouble ca[3],
                      const GLdouble cb[3],
                      const GLdouble cc[3], GLdouble u0, GLdouble u1)
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
        add_conic(points, la, cp, ca, cb, cc, u0, uu);
        add_conic(points, cp, lc, ca, cb, cc, uu, u1);
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
    add_conic(points, a, c, a, b, c, 0, 1);

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
    glyph->space  = (float) TO_DBL(slot->advance.x);

    FT_Outline_Decompose(&slot->outline, &funcs, glyph->points);
}

/*===========================================================================*/

void send_set_font(const char *filename, float e, float o)
{
    int r, c, i, n = vecnum(font);

    epsilon = e;

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
                send_float(F(i)->outline);
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
        F(i)->outline = recv_float();

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
/* Vertex list for points generated during tesselator combination            */
/*
struct combo
{
    GLdouble      p[3];
    struct combo *next;
};

static struct combo *combo = NULL;

static void combine(GLdouble coords[3], void  *vertex_data[4],
                    GLfloat  weight[4], void **output_data)
{
    struct combo *c;

    if ((c = (struct combo *) calloc(1, sizeof (struct combo))))
    {
        c->p[0] = coords[0];
        c->p[1] = coords[1];
        c->p[2] = coords[2];
        c->next = combo;
        combo   = c;

        *output_data = c->p;
    }
}
*/
/*---------------------------------------------------------------------------*/

static void tess_stroke(vector_t points, int first, int last, GLUtesselator *T)
{
    int i;

    gluTessBeginContour(T);
    {
        for (i = first; i <= last; ++i)
            gluTessVertex(T, vecget(points, i),
                             vecget(points, i));
    }
    gluTessEndContour(T);
}

static void tess_glyph(struct glyph *glyph, GLUtesselator *T)
{
    int b, i, n = vecnum(glyph->points);

    /* Tesselate the glyph point list. */

    gluTessBeginPolygon(T, NULL);
    {
        for (b = 0, i = 1; i < n; ++i)
            if (veccmp(glyph->points, b, i) == 0)
            {
                tess_stroke(glyph->points, b, i, T);
                i = b = i + 1;
            }
    }
    gluTessEndPolygon(T);

    /* Release all combined vertices generated during tesselation. */
    /*
    while (combo)
    {
        struct combo *temp = combo;
        combo = combo->next;
        free(temp);
    }
    */
}

/*---------------------------------------------------------------------------*/

static void line_stroke(vector_t points, int first, int last, GLdouble k)
{
    int i, n = last - first;

    GLdouble pn[2];
    GLdouble qn[2];
    GLdouble rn[2];
    GLdouble m, l;

    if (k > 0)
    {
        glBegin(GL_QUAD_STRIP);
        {
            for (i = 0; i <= n; i++)
            {
                const int a = first + (i + n - 1) % n;
                const int b = first +  i;
                const int c = first + (i     + 1) % n;

                const GLdouble *p = ((struct point *) vecget(points, a))->p;
                const GLdouble *q = ((struct point *) vecget(points, b))->p;
                const GLdouble *r = ((struct point *) vecget(points, c))->p;

                pn[0] = -(q[1] - p[1]);
                pn[1] =  (q[0] - p[0]);
                l = sqrt(pn[0] * pn[0] + pn[1] * pn[1]);
                pn[0] /= l;
                pn[1] /= l;

                rn[0] = -(r[1] - q[1]);
                rn[1] =  (r[0] - q[0]);
                l = sqrt(rn[0] * rn[0] + rn[1] * rn[1]);
                rn[0] /= l;
                rn[1] /= l;

                qn[0] = pn[0] + rn[0];
                qn[1] = pn[1] + rn[1];

                m = k / (qn[0] * pn[0] + qn[1] * pn[1]);

                glVertex3d(q[0] - qn[0] * 0, q[1] - qn[1] * 0, q[2]);
                glVertex3d(q[0] + qn[0] * m, q[1] + qn[1] * m, q[2]);
            }
        }
        glEnd();
    }
}

static void line_glyph(struct glyph *glyph, GLdouble O)
{
    int b, i, n = vecnum(glyph->points);

    for (b = 0, i = 1; i < n; ++i)
        if (veccmp(glyph->points, b, i) == 0)
        {
            line_stroke(glyph->points, b, i, O);
            i = b = i + 1;
        }
}

/*---------------------------------------------------------------------------*/

void init_font(int i, GLdouble O)
{
    if (F(i)->state == 0)
    {
        GLuint fills = glGenLists(NUMGLYPH);
        GLuint lines = glGenLists(NUMGLYPH);

        GLUtesselator *T;

        /* Set up a GLU tessalator to handle glyphs. */

        if ((T = gluNewTess()))
        {
            int j;

            gluTessCallback(T, GLU_TESS_BEGIN,   (_GLUfuncptr) glBegin);
/*          gluTessCallback(T, GLU_TESS_COMBINE, (_GLUfuncptr) combine); */
            gluTessCallback(T, GLU_TESS_VERTEX,  (_GLUfuncptr) glVertex3dv);
            gluTessCallback(T, GLU_TESS_END,     (_GLUfuncptr) glEnd);

            /* Enumerate all glyphs. */

            for (j = MINGLYPH; j < MAXGLYPH; ++j)
            {
                struct glyph *glyph = F(i)->glyph + j - MINGLYPH;

                if (glyph->points)
                {
                    glyph->fill = fills + j - MINGLYPH;
                    glyph->line = lines + j - MINGLYPH;

                    /* Tessalate the outline to a display list. */

                    glNewList(glyph->line, GL_COMPILE);
                    glNormal3f(0, 0, 1);
                    line_glyph(glyph, O);
                    glEndList();

                    /* Tessalate the glyph to a display list. */

                    glNewList(glyph->fill, GL_COMPILE);
                    glNormal3f(0, 0, 1);
                    tess_glyph(glyph, T);
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
        struct glyph *glyph = F(i)->glyph;

        glDeleteLists(glyph->fill, NUMGLYPH);
        glDeleteLists(glyph->line, NUMGLYPH);

        F(i)->state = 0;
    }
}

void fini_fonts(void)
{
    int i;

    for (i = 0; i < vecnum(font); ++i)
        fini_font(i);
}

/*---------------------------------------------------------------------------*/

void draw_font(int i, const char *text, int line)
{
    init_font(i, F(i)->outline);

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
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

            if (line)
                glCallList(glyph->line);
            else
                glCallList(glyph->fill);

            glMatrixMode(GL_TEXTURE);
            glTranslated(d, 0, 0);
            glMatrixMode(GL_MODELVIEW);
            glTranslated(d, 0, 0);
        }
    }
    glPopAttrib();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void aabb_font(int i, const char *text, float aabb[6])
{
    int j, n = strlen(text);

    GLdouble x = 0.0;

    aabb[0] = aabb[3] = 0;
    aabb[1] = aabb[4] = 0;
    aabb[2] = aabb[5] = 0;

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

                aabb[0] = MIN(aabb[0], (float) (p[0] + x));
                aabb[1] = MIN(aabb[1], (float)  p[1]);
                aabb[2] = MIN(aabb[2], (float)  p[2]);
                aabb[3] = MAX(aabb[3], (float) (p[0] + x));
                aabb[4] = MAX(aabb[4], (float)  p[1]);
                aabb[5] = MAX(aabb[5], (float)  p[2]);
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
