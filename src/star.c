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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "utility.h"
#include "opengl.h"
#include "matrix.h"
#include "image.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

static int star_cmp0(const void *p1,
                     const void *p2)
{
    return (((const struct star *) p1)->pos[0] <
            ((const struct star *) p2)->pos[0]);
}

static int star_cmp1(const void *p1,
                     const void *p2)
{
    return (((const struct star *) p1)->pos[1] <
            ((const struct star *) p2)->pos[1]);
}

static int star_cmp2(const void *p1,
                     const void *p2)
{
    return (((const struct star *) p1)->pos[2] <
            ((const struct star *) p2)->pos[2]);
}

int (*star_cmp[3])(const void *, const void *) = {
    star_cmp0,
    star_cmp1,
    star_cmp2
};

/*---------------------------------------------------------------------------*/

static void star_color(char type, unsigned char c[3])
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
}

/*---------------------------------------------------------------------------*/

int star_write_bin(struct star *s, FILE *fp)
{
    struct star t = *s;

    /* Ensure all values are represented in network byte order. */

    t.pos[0] = htonf(t.pos[0]);
    t.pos[1] = htonf(t.pos[1]);
    t.pos[2] = htonf(t.pos[2]);
    t.mag    = htonf(t.mag);

    /* Write the record to the given file. */

    if (fwrite(&t, STAR_BIN_RECLEN, 1, fp) > 0)
        return 1;
    else
        return 0;
}

int star_parse_bin(struct star *s, FILE *fp)
{
    /* Read a single star record from the given file. */

    if (fread(s, STAR_BIN_RECLEN, 1, fp) > 0)
    {
        /* Ensure all values are represented in host byte order. */

        s->pos[0] = ntohf(s->pos[0]);
        s->pos[1] = ntohf(s->pos[1]);
        s->pos[2] = ntohf(s->pos[2]);
        s->mag    = ntohf(s->mag);

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

int star_parse_hip(struct star *s, FILE *fp)
{
    char buf[STAR_HIP_RECLEN + 1];

    /* Constants for use in computing galactic coordinates. */

    const double c1 = PI * 282.25 / 180.0;
    const double c2 = PI *  62.6  / 180.0;
    const double c3 = PI *  33.0  / 180.0;

    /* Read a single line from the given file. */

    if (fgets(buf, STAR_HIP_RECLEN + 1, fp))
    {
        double ra;
        double de;
        double mag;
        double plx;

        /* Attempt to parse necessary data from the line. */

        if (sscanf(buf + 51, "%lf", &ra)  == 1 &&
            sscanf(buf + 64, "%lf", &de)  == 1 &&
            sscanf(buf + 41, "%lf", &mag) == 1 &&
            sscanf(buf + 79, "%lf", &plx) == 1 && plx > 0.0)
        {
            double b, l, n1, n2, n3;

            /* Compute equatorial position in parsecs and radians. */

            plx = 1000.0 / fabs(plx);
            ra  = M_RAD(ra);
            de  = M_RAD(de);

            /* Compute the position in galactic coordinates. */

            n1 =                     cos(de) * cos(ra - c1);
            n2 = sin(de) * sin(c2) + cos(de) * sin(ra - c1) * cos(c2);
            n3 = sin(de) * cos(c2) - cos(de) * sin(ra - c1) * sin(c2);

            l = -atan2(n1, n2) + c3;
            b =  asin(n3);

            s->pos[0] = (float) (sin(l) * cos(b) * plx);
            s->pos[1] = (float) (         sin(b) * plx);
            s->pos[2] = (float) (cos(l) * cos(b) * plx);

            /* Compute the absolute magnitude and color. */

            s->mag =  (float) (mag - 5.0 * log(plx / 10.0) / log(10.0));

            star_color(buf[435], s->col);
            
            return 1;
        }
    }
    return 0;
}

int star_parse_tyc(struct star *s, FILE *fp)
{
    char buf[STAR_TYC_RECLEN + 1];

    /* Constants for use in computing galactic coordinates. */

    const double c1 = PI * 282.25 / 180.0;
    const double c2 = PI *  62.6  / 180.0;
    const double c3 = PI *  33.0  / 180.0;

    /* Read a single line from the given file. */

    if (fgets(buf, STAR_TYC_RECLEN + 1, fp))
    {
        double ra, de;
        double bt, vt;
        double mag;
        double plx;
        int    hip;

        /* Attempt to parse necessary data from the line. */

        if (sscanf(buf + 142, "%d",  &hip) == 0 &&
            sscanf(buf + 152, "%lf", &ra)  == 1 &&
            sscanf(buf + 165, "%lf", &de)  == 1 &&
            sscanf(buf + 110, "%lf", &bt)  == 1 &&
            sscanf(buf + 123, "%lf", &vt)  == 1)
        {
            double b, l, n1, n2, n3;
        
            /* Compute equatorial position in parsecs and radians. */
            
            mag = vt - 0.090 * (bt - vt);
            ra  = M_RAD(ra);
            de  = M_RAD(de);
            plx = 10.0;

            /* Compute the position in galactic coordinates. */

            n1 =                     cos(de) * cos(ra - c1);
            n2 = sin(de) * sin(c2) + cos(de) * sin(ra - c1) * cos(c2);
            n3 = sin(de) * cos(c2) - cos(de) * sin(ra - c1) * sin(c2);

            l = -atan2(n1, n2) + c3;
            b =  asin(n3);

            s->pos[0] = (GLfloat) (sin(l) * cos(b) * plx);
            s->pos[1] = (GLfloat) (         sin(b) * plx);
            s->pos[2] = (GLfloat) (cos(l) * cos(b) * plx);
            s->mag    =   (float) mag;

            star_color('K', s->col);

            return 1;
        }
    }
    return 0;
}

int star_gimme_sol(struct star *s)
{
    /* The sun is not in the catalog.  Add it manually. */

    s->pos[0] =    0.0;
    s->pos[1] =   15.5;
    s->pos[2] = 9200.0;
    s->mag    =    5.0;

    star_color('G', s->col);

    return 1;
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

        o = make_texture(p, w, h, 1);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        free(p);
    }

    return o;
}

/*---------------------------------------------------------------------------*/

GLuint star_frag_program(void)
{
    const char *star_fp =
        "!!ARBfp1.0                              \n"

        "ATTRIB icol = fragment.color;           \n"
        "ATTRIB texc = fragment.texcoord;        \n"

        "PARAM  half = { 0.5, 0.5, 0.0, 1.0 };   \n"

        "TEMP   ncol;                            \n"
        "TEMP   dist;                            \n"
        "TEMP   temp;                            \n"

        "OUTPUT ocol = result.color;             \n"

        /* We must subtract and re-add (0.5, 0.5) on texture coordinates.*/

        "SUB    dist, texc, half;                \n"

        /* Scale the texture inversely to the value of each channel. */

        "RCP    ncol.r, icol.r;                  \n"
        "RCP    ncol.g, icol.g;                  \n"
        "RCP    ncol.b, icol.b;                  \n"

        /* Perform the texture lookup separately for each color component.*/

        "MAD    temp, dist, ncol.r, half;        \n"
        "TEX    ocol.r, temp, texture[0], 2D;    \n"

        "MAD    temp, dist, ncol.g, half;        \n"
        "TEX    ocol.g, temp, texture[0], 2D;    \n"

        "MAD    temp, dist, ncol.b, half;        \n"
        "TEX    ocol.b, temp, texture[0], 2D;    \n"

        /* Assume additive blending, and set alpha to 1. */

        "MOV    ocol.a, 1;                       \n"

        "END                                     \n";

    GLuint program = 0;

    if (GL_has_program)
    {
        size_t len = strlen(star_fp);

        glGenProgramsARB(1, &program);

        glBindProgramARB  (GL_FRAGMENT_PROGRAM_ARB, program);
        glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
                           GL_PROGRAM_FORMAT_ASCII_ARB, len, star_fp);
    }
    return program;
}

GLuint star_vert_program(void)
{
    const char *star_vp =
        "!!ARBvp1.0                              \n"

        "ATTRIB ipos   = vertex.position;        \n"
        "ATTRIB icol   = vertex.color;           \n"
        "ATTRIB imag   = vertex.attrib[6];       \n"

        "PARAM  const  = { 0.01, -0.2, 10.0, 0.7525749 };  \n"
        "PARAM  view   = program.env[0];                   \n"
        "PARAM  mult   = program.env[1];                   \n"
        "PARAM  mvp[4] = { state.matrix.mvp };             \n"

        "TEMP   dist;                            \n"
        "TEMP   amag;                            \n"
        "TEMP   luma;                            \n"
        "TEMP   temp;                            \n"

        "OUTPUT osiz   = result.pointsize;       \n"
        "OUTPUT opos   = result.position;        \n"
        "OUTPUT ocol   = result.color;           \n"

        /* Transform the star position. */

        "DP4    opos.x, mvp[0], ipos;            \n"
        "DP4    opos.y, mvp[1], ipos;            \n"
        "DP4    opos.z, mvp[2], ipos;            \n"
        "DP4    opos.w, mvp[3], ipos;            \n"

        /*  Compute the distance (squared) from the viewpoint to the star. */

        "SUB    temp, ipos, view;                \n"
        "DP3    dist, temp, temp;                \n"

        /* Compute the apparent magnitude. */

        "MUL    temp, dist, const.x;             \n"
        "LG2    temp, temp.x;                    \n"
        "MUL    temp, temp, const.w;             \n"
        "ADD    amag, imag, temp;                \n"

        /* Compute the luminosity and sprite scale. */

        "MUL    temp, amag, const.y;             \n"
        "POW    luma, const.z, temp.x;           \n"
        "MUL    osiz, luma, mult;                \n"
        
        "MOV    ocol, icol;                      \n"
        "END                                     \n";

    GLuint program;

    if (GL_has_program)
    {
        size_t len = strlen(star_vp);

        glGenProgramsARB(1, &program);

        glBindProgramARB  (GL_VERTEX_PROGRAM_ARB, program);
        glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                           GL_PROGRAM_FORMAT_ASCII_ARB, len, star_vp);
    }
    return program;
}

/*---------------------------------------------------------------------------*/

float star_pick(const struct star *s, const float p[3], const float v[3])
{
    float w[3];
    float d;

    w[0] = s->pos[0] - p[0];
    w[1] = s->pos[1] - p[1];
    w[2] = s->pos[2] - p[2];

    d = (float) sqrt(w[0] * w[0] + w[1] * w[1] + w[2] * w[2]);

    w[0] /= d;
    w[1] /= d;
    w[2] /= d;

    return (v[0] * w[0] + v[1] * w[1] + v[2] * w[2]);
}

/*---------------------------------------------------------------------------*/
