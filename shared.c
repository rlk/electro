/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    TOTAL PERSPECTIVE VORTEX is free software;  you can redistribute it    */
/*    and/or modify it under the terms of the  GNU General Public License    */
/*    as published by the  Free Software Foundation;  either version 2 of    */
/*    the License, or (at your option) any later version.                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mpi.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#endif

#include "image.h"
#include "status.h"
#include "shared.h"

/*---------------------------------------------------------------------------*/

void mpi_error(int err)
{
    char buf[256];
    int len = 256;

    MPI_Error_string(err, buf, &len);
    fprintf(stderr, "MPI Error: %s\n", buf);
}

/*---------------------------------------------------------------------------*/

static struct viewport *Vi;
static struct viewport *Vo;
static int              V_max = 0;
static int              V_num = 0;

void viewport_init(int n)
{
    Vi = (struct viewport *) calloc(n, sizeof (struct viewport));
    Vo = (struct viewport *) calloc(n, sizeof (struct viewport));

    V_max = n;
    V_num = 0;
}

void viewport_tile(const char *name, float X, float Y,
                                     float x, float y, float w, float h)
{
    if (V_num < V_max)
    {
        strncpy(Vi[V_num].name, name, NAMELEN);

        Vi[V_num].X = X;
        Vi[V_num].Y = Y;
        Vi[V_num].x = x;
        Vi[V_num].y = y;
        Vi[V_num].w = w;
        Vi[V_num].h = h;

        V_num++;
    }
}

void viewport_sync(int i, int n)
{
    size_t sz = sizeof (struct viewport);
    int err;

    struct viewport Vt;

    /* Get this client's host name.  Set a default viewport. */

    gethostname(Vt.name, NAMELEN);

    Vt.X =    0.0f;
    Vt.Y =    0.0f;
    Vt.x = -400.0f;
    Vt.y = -300.0f;
    Vt.w =  800.0f;
    Vt.h =  600.0f;

    /* Gather all host names at the root. */

    if ((err = MPI_Gather(&Vt, sz, MPI_BYTE,
                           Vo, sz, MPI_BYTE, 0, MPI_COMM_WORLD))
            != MPI_SUCCESS)
        mpi_error(err);

    /* If this is the root, assign viewports by matching host names. */

    if (i == 0)
    {
        int j;
        int k;

        for (j = 1; j < n; j++)
            for (k = 0; k < V_num; k++)
                if (strcmp(Vo[j].name, Vi[k].name) == 0)
                {
                    /* A name matches.  Copy the viewport definition. */

                    Vo[j].X = Vi[k].X;
                    Vo[j].Y = Vi[k].Y;
                    Vo[j].x = Vi[k].x;
                    Vo[j].y = Vi[k].y;
                    Vo[j].w = Vi[k].w;
                    Vo[j].h = Vi[k].h;

                    /* Destroy the name to ensure it matches at most once. */

                    strcpy(Vi[k].name, "");

                    break;
                }
    }

    /* Scatter the assignments to all clients. */

    if ((err = MPI_Scatter(Vo, sz, MPI_BYTE,
                          &Vt, sz, MPI_BYTE, 0, MPI_COMM_WORLD))
            != MPI_SUCCESS)
        mpi_error(err);

    /* Apply this client's viewport. */

    if (i) status_set_viewport(Vt.X, Vt.Y, Vt.x, Vt.y, Vt.w, Vt.h);
}

/*---------------------------------------------------------------------------*/

GLuint mpi_load_program(int id, const char *filename, GLenum target)
{
    struct stat buf;

    char *ptr = NULL;
    int   len = 0;
    int   err;

    GLuint program = 0;

    /* If this host is root, determine the file size. */

    if ((id == 0) && (stat(filename, &buf) == 0))
        len = buf.st_size + 1;
        
    /* Broadcast the size and allocate that much memory. */

    if ((err = MPI_Bcast(&len, 1, MPI_INTEGER, 0, MPI_COMM_WORLD)))
        mpi_error(err);

    if ((ptr = (char *) calloc(1, len)))
    {
        FILE *fp;

        /* If this host is root, read the contents of the file. */

        if ((id == 0) && (fp = fopen(filename, "r")))
        {
            fread(ptr, 1, len, fp);
            fclose(fp);
        }

        /* Broadcast the contents of the file. */

        if ((err = MPI_Bcast(ptr, len, MPI_CHAR, 0, MPI_COMM_WORLD)))
            mpi_error(err);

        /* Generate and initialize a program object. */

        glGenProgramsARB(1, &program);
        glBindProgramARB(target, program);
        glProgramStringARB(target, GL_PROGRAM_FORMAT_ASCII_ARB, len - 1, ptr);

        /* If this host is root, report any error in the program text. */

        if ((id == 0) && (glGetError() != GL_NO_ERROR))
            fprintf(stderr, "%s", glGetString(GL_PROGRAM_ERROR_STRING_ARB));

        free(ptr);
    }
    return program;
}

GLuint mpi_load_texture(int id, const char *filename)
{
    GLubyte *p = NULL;
    int      w = 0;
    int      h = 0;
    int      b = 0;

    GLuint texture = 0;
    int err;

    /* If this host is root, load the image file. */

    if (id == 0) p = (GLubyte *) image_load_png(filename, &w, &h, &b);

    /* Broadcast the image attributes. */

    if ((err = MPI_Bcast(&w, 1, MPI_INT, 0, MPI_COMM_WORLD)) ||
        (err = MPI_Bcast(&h, 1, MPI_INT, 0, MPI_COMM_WORLD)) ||
        (err = MPI_Bcast(&b, 1, MPI_INT, 0, MPI_COMM_WORLD)))
        mpi_error(err);

    /* If this host is not root, allocate a pixel buffer. */

    if (id != 0) p = (GLubyte *) malloc(w * h * b);

    /* Broadcast the pixel data. */

    if ((err = MPI_Bcast(p, w * h * b, MPI_BYTE, 0, MPI_COMM_WORLD)))
        mpi_error(err);

    /* Create a texture object. */

    texture = image_make_tex(p, w, h, b);

    free(p);

    return texture;
}

/*---------------------------------------------------------------------------*/

