#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "opengl.h"
#include "galaxy.h"
#include "viewer.h"
#include "png.h"

/*---------------------------------------------------------------------------*/

static GLuint star_texture;

static int max_stars = 118218;
static int num_stars = 0;
static int max_nodes = 0;
static int num_nodes = 0;

static struct star *S;
static struct node *N;

/*---------------------------------------------------------------------------*/

double log_10(double n)
{
    return log(n) / log(10);
}

double log_2(double n)
{
    return log(n) / log(2);
}

/*---------------------------------------------------------------------------*/

void read_star(FILE *fp)
{
    char buf[512];

    if (fgets(buf, 512, fp))
    {
        float ra  = 0;
        float de  = 0;
        float mag = 0;
        float plx = 0;

        if (sscanf(buf + 51, "%f", &ra)  == 1 &&
            sscanf(buf + 64, "%f", &de)  == 1 &&
            sscanf(buf + 41, "%f", &mag) == 1 &&
            sscanf(buf + 79, "%f", &plx) == 1 && fabs(plx) > 0)
        {
            plx =    1000.0 / fabs(plx);
            ra  = M_PI * ra / 180.0;
            de  = M_PI * de / 180.0;

            S[num_stars].magnitude   =  mag - 5.0 * log_10(plx / 10.0);

            S[num_stars].position[0] =  sin(ra) * cos(de) * plx;
            S[num_stars].position[1] =            sin(de) * plx;
            S[num_stars].position[2] =  cos(ra) * cos(de) * plx;

            S[num_stars].color[0] = 0xff;
            S[num_stars].color[1] = 0xff;
            S[num_stars].color[2] = 0xff;

            num_stars++;
        }
    }
}

void star_init(void)
{
    star_texture = png_load("blob.png");

    if ((S = (struct star *) calloc(sizeof (struct star), max_stars)))
    {
        FILE *fp;

        if ((fp = fopen("hip_main.dat", "r")))
        {
            int i;

            for (i = 0; i < max_stars; i++)
                read_star(fp);
       
            fclose(fp);
        }
    }
}

/*---------------------------------------------------------------------------*/

static int tree_init_count(int d)
{
    if (d == 0)
        return 8;
    else
        return 8 * tree_init_count(d - 1);
}

int tree_init(int d)
{
    int n = tree_init_count(d);
}

/*---------------------------------------------------------------------------*/

void galaxy_init(void)
{
    FILE  *fp;

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
    glEnable(GL_VERTEX_PROGRAM_ARB);

    glEnable(GL_POINT_SPRITE_ARB);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

    if ((fp = fopen("star.vp", "r")))
    {
        char   buf[VPMAXLEN];
        size_t len;

        if ((len = fread(buf, 1, VPMAXLEN, fp)) > 0)
            glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                               GL_PROGRAM_FORMAT_ASCII_ARB, len, buf);

        if (glGetError() != GL_NO_ERROR)
            printf("%s", glGetString(GL_PROGRAM_ERROR_STRING_ARB));

        fclose(fp);
    }

    star_init();
    tree_init(2);
}

/*---------------------------------------------------------------------------*/

void galaxy_draw(const double p[3])
{
    GLsizei stride = sizeof (struct star);
    double  viewpoint[3];
    double  magnifier[1];

    viewer_get_pos(viewpoint);
    viewer_get_mag(magnifier);

    glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, 0, viewpoint);
    glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, 1, magnifier);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableVertexAttribArrayARB(6);

    glVertexPointer(3, GL_FLOAT, stride, &S[0].position);
    glColorPointer (3, GL_UNSIGNED_BYTE, stride, &S[0].color);
    glVertexAttribPointerARB(6, 1, GL_FLOAT, 0, stride, &S[0].magnitude);

    glDrawArrays(GL_POINTS, 0, num_stars);

    glDisableVertexAttribArrayARB(6);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

/*---------------------------------------------------------------------------*/
