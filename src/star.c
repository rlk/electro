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
#include <stdio.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include "opengl.h"
#include "buffer.h"
#include "image.h"
#include "star.h"

/*---------------------------------------------------------------------------*/
/*
static struct star *star_near_data  = NULL;
static int          star_near_count = 0;
static GLuint       star_near_vp    = 0;

static struct star *star_far_data   = NULL;
static int          star_far_count  = 0;
static GLuint       star_far_vp     = 0;

static GLuint       star_texture    = 0;
static GLuint       star_fp         = 0;
*/
/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#define FMODE_RB "rb"
#define FMODE_WB "wb"
#else
#define FMODE_RB "r"
#define FMODE_WB "w"
#endif

/*---------------------------------------------------------------------------*/

void star_color(char type, unsigned char c[3])
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

union swapper
{
    float f;
    long  l;
};

float htonf(float f)
{
    union swapper s;

    s.f = f;
    s.l = htonl(s.l);

    return s.f;
}

float ntohf(float f)
{
    union swapper s;

    s.f = f;
    s.l = ntohl(s.l);

    return s.f;
}

/*---------------------------------------------------------------------------*/

int star_write_bin(FILE *fp, struct star *s)
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

int star_parse_bin(FILE *fp, struct star *s)
{
    /* Read a single star record from the given file. */

    if (fread(s, STAR_BIN_RECLEN, 1, fp) > 0)
    {
        /* Ensure all values are represented in host byte order. */

        s->pos[0] = ntohf(s->pos[0]);
        s->pos[1] = ntohf(s->pos[1]);
        s->pos[2] = ntohf(s->pos[2]);
        s->mag    = ntohf(s->mag);

        return +1;
    }
    return -1;
}

/*---------------------------------------------------------------------------*/

int star_parse_hip(FILE *fp, struct star *s)
{
    char buf[STAR_HIP_RECLEN];

    /* Constants for use in computing galactic coordinates. */

    const double c1 = PI * 282.25 / 180.0;
    const double c2 = PI *  62.6  / 180.0;
    const double c3 = PI *  33.0  / 180.0;

    /* Read a single line from the given file. */

    if (fgets(buf, STAR_HIP_RECLEN, fp))
    {
        double ra;                  /* right ascension    */
        double de;                  /* declination        */
        double mag;                 /* apparent magnitude */
        double plx;                 /* parallax           */

        double b, l, n1, n2, n3;

        /* Attempt to parse necessary data from the line. */

        if (sscanf(buf + 51, "%lf", &ra)  == 1 &&
            sscanf(buf + 64, "%lf", &de)  == 1 &&
            sscanf(buf + 41, "%lf", &mag) == 1 &&
            sscanf(buf + 79, "%lf", &plx) == 1)
        {
            /* Compute equatorial position in parsecs and radians. */

            plx =  1000.0 / fabs(plx);
            ra  = PI * ra / 180.0;
            de  = PI * de / 180.0;

            if (isnormal(plx))
            {
                /* Compute the position in galactic coordinates. */

                n1 =                     cos(de) * cos(ra - c1);
                n2 = sin(de) * sin(c2) + cos(de) * sin(ra - c1) * cos(c2);
                n3 = sin(de) * cos(c2) - cos(de) * sin(ra - c1) * sin(c2);

                l = -atan2(n1, n2) + c3;
                b =  asin(n3);

                s->pos[0] = (float) (sin(l) * cos(b) * plx);
                s->pos[1] = (float) (         sin(b) * plx + 15.5);
                s->pos[2] = (float) (cos(l) * cos(b) * plx + 9200);

                /* Compute the absolute magnitude and color. */

                s->mag =  (float) (mag - 5.0 * log(plx / 10.0) / log(10.0));

                star_color(buf[435], s->col);
            
                return 1;
            }
        }
    }
    return 0;
}

#ifdef SNIP
int star_parse_tyc(FILE *fp, struct star *s)
{
    char buf[STAR_TYC_RECLEN];

    /* Read a single line from the given file. */

    if (fgets(buf, STAR_TYC_RECLEN, fp))
    {
        double ra, de;
        double bt, vt;
        double mag;
        double plx;

        double n1, c1 = PI * 282.25 / 180.0;
        double n2, c2 = PI *  62.6  / 180.0;
        double n3, c3 = PI *  33.0  / 180.0;
        double b, l;
        
        int hip;

        /* Attempt to parse necessary data from the line. */

        if (sscanf(buf + 142, "%d",  &hip) == 0 &&
            sscanf(buf + 152, "%lf", &ra)  == 1 &&
            sscanf(buf + 165, "%lf", &de)  == 1 &&
            sscanf(buf + 110, "%lf", &bt)  == 1 &&
            sscanf(buf + 123, "%lf", &vt)  == 1)
        {
            /* Compute equatorial position in parsecs and radians. */
            
            mag = vt - 0.090 * (bt - vt);
            plx = 100000.0;
            ra  = PI * ra / 180.0;
            de  = PI * de / 180.0;

            /* Compute the position in galactic coordinates. */

            n1 =                     cos(de) * cos(ra - c1);
            n2 = sin(de) * sin(c2) + cos(de) * sin(ra - c1) * cos(c2);
            n3 = sin(de) * cos(c2) - cos(de) * sin(ra - c1) * sin(c2);

            l = -atan2(n1, n2) + c3;
            b =  asin(n3);

            s->pos[0] = (GLfloat) (sin(l) * cos(b) * plx);
            s->pos[1] = (GLfloat) (         sin(b) * plx + 15.5);
            s->pos[2] = (GLfloat) (cos(l) * cos(b) * plx + 9200);
            s->mag    = mag;

            star_color('K', s->col);

            return 1;
        }
    }
    return 0;
}
#endif
/*---------------------------------------------------------------------------*/
#ifdef SNIP
int star_write(const char *filename)
{
    FILE *fp;
    int n = 0;

    if ((fp = fopen(filename, FMODE_WB)))
    {
        for (n = 0; n < star_count; n++)
            star_write_bin(fp, star_data + n);

        fclose(fp);
    }
    else perror("star_write_catalog: fopen()");

    return n;
}

int star_read_near_bin(const char *filename)
{
    struct stat buf;

    /* Count the number of stars in the catalog. */

    if (stat(filename, &buf) == 0)
    {
        size_t n = buf.st_size / STAR_BIN_RECLEN;
        size_t s = sizeof (struct star);
        FILE *fp;

        /* Open the catalog, allocate and fill a buffer of stars. */

        if ((fp = fopen(filename, FMODE_RB)))
        {
            star_near_data = (struct star *) realloc(star_near_data, s * n);

            if (star_near_data)
            {
                int c;

                star_near_count = 0;

                /* Parse all catalog records. */
               
                while ((c = star_parse_bin(fp, star_near_data +
                                               star_near_count)) >= 0)
                    star_near_count += c;
            }
            fclose(fp);
        }
        else perror("star_read_near_bin: fopen()");
    }
    else perror("star_read_near_bin: stat()");

    return star_near_count;
}

int star_read_far_bin(const char *filename)
{
    struct stat buf;

    /* Count the number of stars in the catalog. */

    if (stat(filename, &buf) == 0)
    {
        size_t n = buf.st_size / STAR_BIN_RECLEN;
        size_t s = sizeof (struct star);
        FILE *fp;

        /* Open the catalog, allocate and fill a buffer of stars. */

        if ((fp = fopen(filename, FMODE_RB)))
        {
            star_far_data = (struct star *) realloc(star_far_data, s * n);

            if (star_far_data)
            {
                int c;

                star_far_count = 0;

                /* Parse all catalog records. */
               
                while ((c = star_parse_bin(fp, star_far_data +
                                               star_far_count)) >= 0)
                    star_far_count += c;
            }
            fclose(fp);
        }
        else perror("star_read_far_bin: fopen()");
    }
    else perror("star_read_far_bin: stat()");

    return star_far_count;
}

int star_read_near_sol(void)
{
    size_t n = star_near_count + 1;
    size_t s = sizeof (struct star);
    
    if ((star_near_data = (struct star *) realloc(star_near_data, s * n)))
    {
        /* The sun is not in the catalog.  Add it manually. */

        star_near_data[star_near_count].pos[0] =    0.0;
        star_near_data[star_near_count].pos[1] =   15.5;
        star_near_data[star_near_count].pos[2] = 9200.0;
        star_near_data[star_near_count].mag    =    5.0;

        star_color('G', star_near_data[star_near_count].col);

        star_near_count++;

        return 1;
    }
    return 0;
}

int star_read_near_hip(const char *filename)
{
    struct stat buf;

    star_read_near_sol();

    /* Count the number of stars in the catalog. */

    if (stat(filename, &buf) == 0)
    {
        size_t n = star_near_count + buf.st_size / STAR_HIP_RECLEN;
        size_t s = sizeof (struct star);
        FILE *fp = NULL;

        /* Open the catalog, allocate and fill a buffer of stars. */

        if ((fp = fopen(filename, "r")))
        {
            star_near_data = (struct star *) realloc(star_near_data, s * n);

            if (star_near_data)
            {
                int c;

                /* Parse all catalog records. */
               
                while ((c = star_parse_hip(fp, star_near_data +
                                               star_near_count)) >= 0)
                    star_near_count += c;
            }
            fclose(fp);
        }
        else perror("star_read_hip: fopen()");
    }
    else perror("star_read_hip: stat()");

    return star_near_count;
}

int star_read_far_tyc(const char *filename)
{
    struct stat buf;

    /* Count the number of stars in the catalog. */

    if (stat(filename, &buf) == 0)
    {
        size_t n = star_far_count + buf.st_size / STAR_TYC_RECLEN;
        size_t s = sizeof(struct star);
        FILE *fp = NULL;

        /* Open the catalog, allocate and fill a buffer of stars. */

        if ((fp = fopen(filename, "r")))
        {
            star_far_data = (struct star *) realloc(star_far_data, s * n);

            if (star_far_data)
            {
                int c;

                /* Parse all catalog records. */
               
                while ((c = star_parse_tyc(fp, star_far_data +
                                               star_far_count)) >= 0)
                    star_far_count += c;
            }
            fclose(fp);
        }
        else perror("star_read_far_tyc: fopen()");
    }
    else perror("star_read_far_tyc: stat()");

    return star_far_count;
}
#endif

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
        "!!ARBfp1.0                                                        \n"

        "ATTRIB icol = fragment.color;                                     \n"
        "ATTRIB texc = fragment.texcoord;                                  \n"

        "PARAM  half = { 0.5, 0.5, 0.0, 1.0 };                             \n"

        "TEMP   ncol;                                                      \n"
        "TEMP   dist;                                                      \n"
        "TEMP   temp;                                                      \n"

        "OUTPUT ocol = result.color;                                       \n"

        /* We must subtract and re-add (0.5, 0.5) on texture coordinates.*/

        "SUB    dist, texc, half;                                          \n"

        /* Scale the texture inversely to the value of each channel. */

        "RCP    ncol.r, icol.r;                                            \n"
        "RCP    ncol.g, icol.g;                                            \n"
        "RCP    ncol.b, icol.b;                                            \n"

        /* Perform the texture lookup separately for each color component.*/

        "MAD    temp, dist, ncol.r, half;                                  \n"
        "TEX    ocol.r, temp, texture[0], 2D;                              \n"

        "MAD    temp, dist, ncol.g, half;                                  \n"
        "TEX    ocol.g, temp, texture[0], 2D;                              \n"

        "MAD    temp, dist, ncol.b, half;                                  \n"
        "TEX    ocol.b, temp, texture[0], 2D;                              \n"

        /* Assume additive blending, and set alpha to 1. */

        "MOV    ocol.a, 1;                                                 \n"

        "END                                                               \n";

    GLuint program;

    glGenProgramsARB(1, &program);

    glBindProgramARB  (GL_FRAGMENT_PROGRAM_ARB, program);
    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB, strlen(star_fp), star_fp);

    return program;
}

GLuint star_vert_program(void)
{
    const char *star_vp =
        "!!ARBvp1.0                                                         \n"

        "ATTRIB ipos   = vertex.position;                                  \n"
        "ATTRIB icol   = vertex.color;                                     \n"
        "ATTRIB imag   = vertex.attrib[6];                                 \n"

        "PARAM  const  = { 0.01, -0.2, 10.0, 0.7525749 };                  \n"
        "PARAM  view   = program.env[0];                                   \n"
        "PARAM  mult   = program.env[1];                                   \n"
        "PARAM  mvp[4] = { state.matrix.mvp };                             \n"

        "TEMP   dist;                                                      \n"
        "TEMP   amag;                                                      \n"
        "TEMP   luma;                                                      \n"
        "TEMP   temp;                                                      \n"

        "OUTPUT osiz   = result.pointsize;                                 \n"
        "OUTPUT opos   = result.position;                                  \n"
        "OUTPUT ocol   = result.color;                                     \n"

        /* Transform the star position. */

        "DP4    opos.x, mvp[0], ipos;                                      \n"
        "DP4    opos.y, mvp[1], ipos;                                      \n"
        "DP4    opos.z, mvp[2], ipos;                                      \n"
        "DP4    opos.w, mvp[3], ipos;                                      \n"

        /*  Compute the distance (squared) from the viewpoint to the star. */

        "SUB    temp, ipos, view;                                          \n"
        "DP3    dist, temp, temp;                                          \n"

        /* Compute the apparent magnitude. */

        "MUL    temp, dist, const.x;                                       \n"
        "LG2    temp, temp.x;                                              \n"
        "MUL    temp, temp, const.w;                                       \n"
        "ADD    amag, imag, temp;                                          \n"

        /* Compute the luminosity and sprite scale. */

        "MUL    temp, amag, const.y;                                       \n"
        "POW    luma, const.z, temp.x;                                     \n"
        "MUL    osiz, luma, mult;                                          \n"
        
        "MOV    ocol, icol;                                                \n"
        "END                                                               \n";

    GLuint program;

    glGenProgramsARB(1, &program);

    glBindProgramARB  (GL_VERTEX_PROGRAM_ARB, program);
    glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB, strlen(star_vp), star_vp);

    return program;
}

#ifdef SNIP
static GLuint load_star_far_vp(void)
{
    const char *star_vp =
        "!!ARBvp1.0                                                         \n"

        "ATTRIB ipos   = vertex.position;                                  \n"
        "ATTRIB icol   = vertex.color;                                     \n"
        "ATTRIB imag   = vertex.attrib[6];                                 \n"

        "PARAM  const  = { 0.0, -0.2, 10.0, 0.0 };                         \n"
        "PARAM  view   = program.env[0];                                   \n"
        "PARAM  mult   = program.env[1];                                   \n"
        "PARAM  mvp[4] = { state.matrix.mvp };                             \n"

        "TEMP   dist;                                                      \n"
        "TEMP   amag;                                                      \n"
        "TEMP   luma;                                                      \n"
        "TEMP   temp;                                                      \n"

        "OUTPUT osiz   = result.pointsize;                                 \n"
        "OUTPUT opos   = result.position;                                  \n"
        "OUTPUT ocol   = result.color;                                     \n"

        /* Transform the star position. */

        "DP4    opos.x, mvp[0], ipos;                                      \n"
        "DP4    opos.y, mvp[1], ipos;                                      \n"
        "DP4    opos.z, mvp[2], ipos;                                      \n"
        "DP4    opos.w, mvp[3], ipos;                                      \n"

        /* Compute the luminosity and sprite scale. */

        "MUL    temp, imag, const.y;                                       \n"
        "POW    luma, const.z, temp.x;                                     \n"
        "MUL    osiz, luma, mult;                                          \n"
        
        "MOV    ocol, icol;                                                \n"
        "END                                                               \n";

    GLuint program;

    glGenProgramsARB(1, &program);

    glBindProgramARB  (GL_VERTEX_PROGRAM_ARB, program);
    glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB, strlen(star_vp), star_vp);

    return program;
}
#endif
/*---------------------------------------------------------------------------*/
#ifdef SNIP
void star_send_create(void)
{
    pack_index(star_near_count);
    pack_alloc(star_near_count * sizeof (struct star), star_near_data);

    pack_index(star_far_count);
    pack_alloc(star_far_count  * sizeof (struct star), star_far_data);

    star_texture = star_make_texture();
    star_fp      = load_star_fp();
    star_near_vp = load_star_near_vp();
    star_far_vp  = load_star_far_vp();
}

void star_recv_create(void)
{
    star_near_count = unpack_index();
    star_near_data  = unpack_alloc(star_near_count * sizeof (struct star));

    star_far_count  = unpack_index();
    star_far_data   = unpack_alloc(star_far_count  * sizeof (struct star));

    star_texture = star_make_texture();
    star_fp      = load_star_fp();
    star_near_vp = load_star_near_vp();
    star_far_vp  = load_star_far_vp();
}

/*---------------------------------------------------------------------------*/

void star_draw(void)
{
    GLsizei s = sizeof (struct star);

    glBindTexture(GL_TEXTURE_2D, star_texture);

    glEnable(GL_VERTEX_PROGRAM_ARB);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableVertexAttribArrayARB(6);

        glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, star_fp);

        glBindProgramARB(GL_VERTEX_PROGRAM_ARB,   star_far_vp);
        glVertexPointer(3, GL_FLOAT,                s, &star_far_data[0].pos);
        glColorPointer (3, GL_UNSIGNED_BYTE,        s, &star_far_data[0].col);
        glVertexAttribPointerARB(6, 1, GL_FLOAT, 0, s, &star_far_data[0].mag);
        glDrawArrays(GL_POINTS, 0, star_far_count);

        glBindProgramARB(GL_VERTEX_PROGRAM_ARB,   star_near_vp);
        glVertexPointer(3, GL_FLOAT,                s, &star_near_data[0].pos);
        glColorPointer (3, GL_UNSIGNED_BYTE,        s, &star_near_data[0].col);
        glVertexAttribPointerARB(6, 1, GL_FLOAT, 0, s, &star_near_data[0].mag);
        glDrawArrays(GL_POINTS, 0, star_near_count);

        glDisableVertexAttribArrayARB(6);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glDisable(GL_VERTEX_PROGRAM_ARB);
}

/*---------------------------------------------------------------------------*/

void star_delete(void)
{
    free(star_near_data);
    free(star_far_data);

    star_near_data  = NULL;
    star_near_count = 0;
    star_far_data   = NULL;
    star_far_count  = 0;
}
#endif
/*---------------------------------------------------------------------------*/
