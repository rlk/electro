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

#define GLSL 1

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

GLuint make_texture(void)
{
    int c, w = 256;
    int r, h = 256;

    GLubyte *b = NULL;
    GLuint   o = 0;

    if ((b = (GLubyte *) malloc(w * h)))
    {
        const double k = -(2.0 * exp(1)) * (2.0 * exp(1));

        /* Fill the buffer with an exponential gradient. */

        for (r = 0; r < h; r++)
            for (c = 0; c < w; c++)
            {
                double x = (double) c / (double) w - 0.5;
                double y = (double) r / (double) h - 0.5;
                double z = sqrt(x * x + y * y);
                                

                b[r * w + c] = (GLubyte) floor(exp(k * z * z) * 255.0);
            }

        /* Create a GL texture object. */

        glGenTextures(1, &o);
        glBindTexture(GL_TEXTURE_2D, o);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE, w, h,
                          GL_LUMINANCE, GL_UNSIGNED_BYTE, b);

        free(b);
    }

    return o;
}

void get_color(char type, unsigned char c[3])
{
    c[0] = c[1] = c[2] = 0xFF;

    switch (type)
    {
    case 'O': c[0] = 0x6F; c[1] = 0x6F; c[2] = 0xFF; break;
    case 'B': c[0] = 0x8F; c[1] = 0x8F; c[2] = 0xFF; break;
    case 'A': c[0] = 0xBF; c[1] = 0xBF; c[2] = 0xFF; break;
    case 'F': c[0] = 0xDF; c[1] = 0xDF; c[2] = 0xFF; break;
    case 'G': c[0] = 0xFF; c[1] = 0xFF; c[2] = 0xBF; break;
    case 'K': c[0] = 0xFF; c[1] = 0xAF; c[2] = 0x8F; break;
    case 'M': c[0] = 0xFF; c[1] = 0x6F; c[2] = 0x6F; break;
    }
}

void read_star(FILE *fp)
{
    char buf[512];

    if (fgets(buf, 512, fp))
    {
        double ra  = 0;
        double de  = 0;
        double mag = 0;
        double plx = 0;

        double n1, c1 = M_PI * 282.25 / 180.0;
        double n2, c2 = M_PI *  62.6  / 180.0;
        double n3, c3 = M_PI *  33.0  / 180.0;
        double b, l;

        unsigned char c[3];

        if (sscanf(buf + 51, "%lf", &ra)  == 1 &&
            sscanf(buf + 64, "%lf", &de)  == 1 &&
            sscanf(buf + 41, "%lf", &mag) == 1 &&
            sscanf(buf + 79, "%lf", &plx) == 1 && fabs(plx) > 0)
        {
            /* Compute equatorial position in parsecs and radians. */

            plx = 1000.0 / fabs(plx);
            ra  = M_PI * ra /  180.0;
            de  = M_PI * de /  180.0;

            /* Compute the position in galactic coordinates. */

            n1 =                     cos(de) * cos(ra - c1);
            n2 = sin(de) * sin(c2) + cos(de) * sin(ra - c1) * cos(c2);
            n3 = sin(de) * cos(c2) - cos(de) * sin(ra - c1) * sin(c2);

            l = -atan2(n1, n2) + c3;
            b =  asin(n3);

            S[num_stars].position[0] =  sin(l) * cos(b) * plx;
            S[num_stars].position[1] =           sin(b) * plx + 15.5;
            S[num_stars].position[2] =  cos(l) * cos(b) * plx + 9200;

            /* Compute the absolute magnitude. */

            S[num_stars].magnitude =  (float) (mag - 5.0 * log_10(plx / 10.0));

            /* Compute the color. */
            
            get_color(buf[435], S[num_stars].color);

            num_stars++;
        }
    }
}

void star_init(void)
{
    star_texture = make_texture();

    if ((S = (struct star *) calloc(sizeof (struct star), max_stars)))
    {
        FILE *fp;

        S[num_stars].position[0] =    0.0;
        S[num_stars].position[1] =   15.5;
        S[num_stars].position[2] = 9200.0;
        S[num_stars].magnitude   =    5.0;

        get_color('G', S[num_stars].color);

        num_stars++;

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

const char *read_file(const char *filename)
{
    char  *ptr = NULL;
    char   buf[MAXBUF];
    size_t len;

    FILE  *fp;

    if ((fp = fopen(filename, "r")))
    {
        if ((len = fread(buf, 1, MAXBUF, fp)) > 0)
        {
            if ((ptr = (char *) calloc(1, len + 1)))
                strncpy(ptr, buf, len);
        }
        fclose(fp);
    }

    return ptr;
}

void galaxy_init(void)
{
    FILE *fp;
    const char *vert_program;
    const char *frag_program;

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
    glEnable(GL_VERTEX_PROGRAM_ARB);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);

    glEnable(GL_POINT_SPRITE_ARB);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_ONE, GL_ONE);
/*    glBlendFunc(GL_SRC_ALPHA, GL_ONE);*/

    glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

    vert_program = read_file("star.vp");
    frag_program = read_file("star.fp");

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

    star_init();
    tree_init(2);
}

/*---------------------------------------------------------------------------*/

void galaxy_draw(const double p[3])
{
    GLsizei stride = sizeof (struct star);
    double  viewpoint[3];
    double  magnifier[1];
    int i;

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
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_TEXTURE_2D);
    {
        double r = 15000;

        glBegin(GL_LINE_LOOP);
        {
            for (i = 0; i < 360; i++)
                glVertex3d(r * sin(M_PI * i / 180.0), 0,
                           r * cos(M_PI * i / 180.0));
        }
        glEnd();
    }
    glEnable(GL_TEXTURE_2D);
}

/*---------------------------------------------------------------------------*/
