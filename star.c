#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>

#include "opengl.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

static GLuint       star_texture = 0;
static int          star_count;
static struct star *star_data;

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

GLuint star_make_texture(void)
{
    int c, w = 256;
    int r, h = 256;

    GLubyte *p = NULL;
    GLuint   o = 0;

    if ((p = (GLubyte *) malloc(w * h)))
    {
        const double k = -(2.0 * exp(1)) * (2.0 * exp(1));

        /* Fill the buffer with an exponential gradient. */

        for (r = 0; r < h; r++)
            for (c = 0; c < w; c++)
            {
                double x = (double) c / (double) w - 0.5;
                double y = (double) r / (double) h - 0.5;
                double z = sqrt(x * x + y * y);
                                

                p[r * w + c] = (GLubyte) floor(exp(k * z * z) * 255.0);
            }

        /* Create a texture object, and release the image buffer. */

        o = image_make_tex(p, w, h, 1);

        free(p);
    }

    return o;
}

/*---------------------------------------------------------------------------*/

void star_calc_color(char type, unsigned char c[3])
{
    c[0] = c[1] = c[2] = 0xFF;

    switch (type)
    {
    case 'O': c[0] = 0xBF; c[1] = 0xBF; c[2] = 0xFF; break;
    case 'B': c[0] = 0xCF; c[1] = 0xCF; c[2] = 0xFF; break;
    case 'A': c[0] = 0xDF; c[1] = 0xDF; c[2] = 0xFF; break;
    case 'F': c[0] = 0xEF; c[1] = 0xEF; c[2] = 0xFF; break;
    case 'G': c[0] = 0xFF; c[1] = 0xFF; c[2] = 0xDF; break;
    case 'K': c[0] = 0xFF; c[1] = 0xDF; c[2] = 0xBF; break;
    case 'M': c[0] = 0xFF; c[1] = 0xBF; c[2] = 0x8F; break;
    }
    /*
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
    */
}

/*---------------------------------------------------------------------------*/

int star_parse_bin(FILE *fp, struct star *s)
{
    
}

int star_parse_txt(FILE *fp, struct star *s)
{
    char buf[STAR_TXT_RECLEN + 1];

    /* Read a single line from the given file. */

    if (fgets(buf, STAR_TXT_RECLEN + 1, fp))
    {
        double ra  = 0;
        double de  = 0;
        double mag = 0;
        double plx = 0;

        double n1, c1 = M_PI * 282.25 / 180.0;
        double n2, c2 = M_PI *  62.6  / 180.0;
        double n3, c3 = M_PI *  33.0  / 180.0;
        double b, l;

        /* Attempt to parse necessary data from the line. */

        if (sscanf(buf + 51, "%lf", &ra)  == 1 &&
            sscanf(buf + 64, "%lf", &de)  == 1 &&
            sscanf(buf + 41, "%lf", &mag) == 1 &&
            sscanf(buf + 79, "%lf", &plx) == 1)
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

            s->position[0] = (float) (sin(l) * cos(b) * plx);
            s->position[1] = (float) (         sin(b) * plx + 15.5);
            s->position[2] = (float) (cos(l) * cos(b) * plx + 9200);

            /* Compute the absolute magnitude. */

            s->magnitude =  (float) (mag - 5.0 * log_10(plx / 10.0));

            /* Compute the color. */
            
            star_calc_color(buf[435], s->color);
            
            return +1;
        }
        return 0;
    }
    return -1;
}

/*---------------------------------------------------------------------------*/

int star_read_catalog_txt(const char *filename)
{
    struct stat buf;

    /* Count the number of stars in the catalog. */

    if (stat(filename, &buf) == 0)
    {
        size_t n = 1 + buf.st_size / STAR_TXT_RECLEN;
        FILE *fp = NULL;

        /* Open the catalog, allocate and fill a buffer of stars. */

        if ((fp = fopen(filename, "r")))
        {
            if ((star_data = (struct star *) calloc(sizeof (struct star), n)))
            {
                int c;

                /* The sun is not in the catalog.  Add it manually. */

                star_data[0].position[0] =    0.0;
                star_data[0].position[1] =   15.5;
                star_data[0].position[2] = 9200.0;
                star_data[0].magnitude   =    5.0;

                star_calc_color('G', star_data[0].color);

                star_count = 1;

                /* Parse all catalog records. */
               
                while ((c = star_parse_txt(fp, star_data + star_count)) >= 0)
                    star_count += c;
       
                fclose(fp);
            }
        }
        else perror("star_read_catalog_txt: fopen()");
    }
    else perror("star_read_catalog_txt: stat()");
}

/*---------------------------------------------------------------------------*/

void star_draw(void)
{
    GLsizei stride = sizeof (struct star);

    if (star_texture == 0)
        star_texture = star_make_texture();

    glEnable(GL_VERTEX_PROGRAM_ARB);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableVertexAttribArrayARB(6);

        glVertexPointer(3, GL_FLOAT, stride, &star_data[0].position);
        glColorPointer (3, GL_UNSIGNED_BYTE, stride, &star_data[0].color);
        glVertexAttribPointerARB(6, 1, GL_FLOAT, 0, stride, &star_data[0].magnitude);

        glDrawArrays(GL_POINTS, 0, star_count);

        glDisableVertexAttribArrayARB(6);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glDisable(GL_VERTEX_PROGRAM_ARB);
}

/*---------------------------------------------------------------------------*/
