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
    const char *prog =
        "!!ARBvp1.0                            \n"
        "ATTRIB ipos   = vertex.position;      \n"
        "ATTRIB icol   = vertex.color;         \n"
        "ATTRIB imag   = vertex.attrib[6];     \n"

        "PARAM  view   = program.env[0];       \n"
        "PARAM  mvp[4] = { state.matrix.mvp }; \n"
        "PARAM  const  = { 0.01, -0.2, 10.0, 50.0 }; \n"
        "PARAM  foo  = { -1 }; \n"

        "TEMP   dist;                          \n"
        "TEMP   amag;                          \n"
        "TEMP   luma;                          \n"
        "TEMP   tmp1;                          \n"
        "TEMP   tmp2;                          \n"
        "TEMP   tmp3;                          \n"
        "TEMP   tmp4;                          \n"
        "TEMP   tmp5;                          \n"

        "OUTPUT osiz   = result.pointsize;     \n"
        "OUTPUT opos   = result.position;      \n"
        "OUTPUT ocol   = result.color;         \n"

        /* Transform the star position. */

        "DP4    opos.x, mvp[0], ipos;          \n"
        "DP4    opos.y, mvp[1], ipos;          \n"
        "DP4    opos.z, mvp[2], ipos;          \n"
        "DP4    opos.w, mvp[3], ipos;          \n"

        /* Compute the distance (squared) from the viewpoint to the star. */

        "SUB    tmp1, ipos, view;              \n"
        "DP3    dist, tmp1, tmp1;              \n"

        /* Compute the apparent magnitude. */

        "MUL    tmp2, dist, const.x;           \n"
        "LG2    tmp3, tmp2.x;                  \n"
        "MUL    tmp4, tmp3, 0.7525749;         \n"
        "ADD    amag, imag, tmp4;              \n"

        /* Compute the luminosity and sprite scale. */

        "MUL    tmp5, amag, const.y;           \n"
        "POW    luma, const.z, tmp5.x;         \n"
        "MUL    osiz, luma, const.w;           \n"

        "MOV    ocol, icol;                    \n"
        "END                                   \n";

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
    glEnable(GL_VERTEX_PROGRAM_ARB);

    glEnable(GL_POINT_SPRITE_ARB);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

    glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB, strlen(prog), prog);

    if (glGetError() != GL_NO_ERROR)
        printf("%s", glGetString(GL_PROGRAM_ERROR_STRING_ARB));

    star_init();
    tree_init(2);
}

/*---------------------------------------------------------------------------*/

void galaxy_draw(const double p[3])
{
    GLsizei stride = sizeof (struct star);
    double  viewpoint[3];

    viewer_get_pos(viewpoint);

    glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, 0, viewpoint);

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
