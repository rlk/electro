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

#include "opengl.h"
#include "vector.h"
#include "buffer.h"
#include "event.h"
#include "brush.h"

/*---------------------------------------------------------------------------*/

struct brush
{
    char *file;
    char *name;
    char *frag;
    char *vert;

    int  count;
    int  state;
    int  flags;
    int  image;

    float d[4];
    float a[4];
    float s[4];
    float x[1];

    GLuint frag_obj;
    GLuint vert_obj;
};

static vector_t brush;

/*---------------------------------------------------------------------------*/

#define B(i) ((struct brush *) vecget(brush, i))

static int new_brush(void)
{
    int i, n = vecnum(brush);

    for (i = 0; i < n; ++i)
        if (B(i)->count == 0)
            return i;

    return vecadd(brush);
}

/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*===========================================================================*/

void init_brush(int i)
{
    struct brush *b = B(i);

    if (b->state == 0)
    {
        /* Initialize any vertex and fragment programs. */

        if (b->vert_text && GL_has_vertex_program)
            b->vert_prog = opengl_vert_prog(b->vert_text);
    
        if (b->frag_text && GL_has_fragment_program)
            b->frag_prog = opengl_frag_prog(b->frag_text);

        b->state = 1;
    }
}

void fini_brush(int i)
{
    struct brush *b = B(i);

    if (b->state == 1)
    {
        /* Finalize any vertex and fragment programs. */

        if (GL_has_vertex_program)
            if (glIsProgramARB(b->vert_prog))
                glDeleteProgramsARB(1, &b->vert_prog);

        if (GL_has_fragment_program)
            if (glIsProgramARB(b->frag_prog))
                glDeleteProgramsARB(1, &b->frag_prog);
                
        b->vert_prog = 0;
        b->frag_prog = 0;
        b->state     = 0;
    }
}

void draw_brush(int i, float a)
{
    struct brush *b = B(i);
    int transparent = (b->flags & FLAG_TRANSPARENT);

    /* Apply the texture. */

    draw_image(b->image);

    /* Enable vertex and fragment programs if specified. */

    if (b->frag_prog && GL_has_fragment_program)
    {
        glEnable(GL_FRAGMENT_PROGRAM_ARB);
        glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, b->frag_prog);
    }
    if (b->vert_prog && GL_has_vertex_program)
    {
        glEnable(GL_VERTEX_PROGRAM_ARB);
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB,   b->vert_prog);
    }

    /* Apply the material properties. */
    
    if (b->flags & FLAG_DIFF)
    {
        float d[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

        /* Modulate the diffuse color by the current value. */

        d[0] = b->d[0];
        d[1] = b->d[1];
        d[2] = b->d[2];
        d[3] = b->d[3] * alpha;

        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   d);

        transparent |= (d[3] < 1);
    }

    if (b->flags & FLAG_AMBT)
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   b->a);
    if (b->flags & FLAG_SPEC)
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  b->s);
    if (b->flags & FLAG_SHIN)
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, b->x);

    /* Return an indication that this brush is transparent. */

    return transparent;
}

void free_brush(int i)
{
    if (--I(i)->count == 0)
    {
        struct brush *b = B(i);

        fini_brush(i);

        if (b->file) free(b->file);
        if (b->name) free(b->name);
        if (b->frag) free(b->frag);
        if (b->vert) free(b->vert);

        memset(b, 0, sizeof (struct brush));
    }
}

/*---------------------------------------------------------------------------*/

void init_brushes(void)
{
    int i, n = vecnum(brush);

    for (i = 0; i < n; ++i)
        if (B(i)->count)
            init_brush(i);
}

void fini_brushes(void)
{
    int i, n = vecnum(brush);

    for (i = 0; i < n; ++i)
        if (B(i)->count)
            free_brush(i);
}

/*===========================================================================*/

int startup_brush(void)
{
    int i;

    if ((brush = vecnew(16, sizeof (struct brush))))
    {
        return 1;
    }
    return 0;
}
