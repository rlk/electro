#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "opengl.h"
#include "galaxy.h"
#include "viewer.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

static int max_nodes = 0;
static int num_nodes = 0;

static struct node *N;

/*---------------------------------------------------------------------------*/

void galaxy_init(void)
{
    FILE *fp;
    const char *vert_program;
    const char *frag_program;

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);

    glEnable(GL_POINT_SPRITE_ARB);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_ONE, GL_ONE);

    glPointSize(4);

    glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

    vert_program = gl_read_text("star.vp");
    frag_program = gl_read_text("star.fp");

    glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB,
                       strlen(vert_program), vert_program);

    if (glGetError() != GL_NO_ERROR)
        printf("vert_program: %s", glGetString(GL_PROGRAM_ERROR_STRING_ARB));

    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB,
                       strlen(frag_program), frag_program);

    if (glGetError() != GL_NO_ERROR)
        printf("frag_program: %s", glGetString(GL_PROGRAM_ERROR_STRING_ARB));

    star_read_catalog_txt("hip_main.dat");
}

/*---------------------------------------------------------------------------*/

void circum_draw(void)
{
    int i;

    glDisable(GL_TEXTURE_2D);
    {
        glBegin(GL_LINE_LOOP);
        {
            for (i = 0; i < 360; i++)
                glVertex3d(RADIUS * sin(M_PI * i / 180.0), 0,
                           RADIUS * cos(M_PI * i / 180.0));
        }
        glEnd();
    }
    glEnable(GL_TEXTURE_2D);
}

void galaxy_draw(const double p[3])
{
    double viewpoint[3];
    double magnifier[1];

    viewer_get_pos(viewpoint);
    viewer_get_mag(magnifier);

    glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, 0, viewpoint);
    glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, 1, magnifier);

    circum_draw();
    star_draw();

    /*
    glPointSize(50);
    glBegin(GL_POINTS);
    {
        glColor3f(0.1, 0.1, 0.1);
        node_draw(0, viewer_depth());
    }
    glEnd();
    */
}

/*---------------------------------------------------------------------------*/
