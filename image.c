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

#include <stdlib.h>
#include <stdio.h>
#include <png.h>

#include "opengl.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

static GLubyte *image_punt(const char *message)
{
    fprintf(stderr, "Image error: %s\n", message);
    return NULL;
}

/*---------------------------------------------------------------------------*/

GLuint image_make_tex(const GLubyte *p, int w, int h, int b)
{
    GLenum f = GL_RGB;
    GLuint o;

    /* Determine GL texture format from byte count. */

    switch (b)
    {
    case 1: f = GL_LUMINANCE;        break;
    case 2: f = GL_LUMINANCE_ALPHA;  break;
    case 3: f = GL_RGB;              break;
    case 4: f = GL_RGBA;             break;
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

    gluBuild2DMipmaps(GL_TEXTURE_2D, f, w, h, f, GL_UNSIGNED_BYTE, p);

    return o;
}

/*---------------------------------------------------------------------------*/

GLubyte *image_load_png(const char *filename, int *width,
                                              int *height,
                                              int *bytes)
{
    FILE   *fp = NULL;
    GLubyte *p = NULL;

    png_structp readp = NULL;
    png_infop   infop = NULL;
    png_bytep  *bytep = NULL;

    /* Initialize all PNG import data structures. */

    if (!(fp = fopen(filename, FMODE_RB)))
        return image_punt("Failure opening PNG file");

    if (!(readp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        return image_punt("Failure creating PNG read struct");
        
    if (!(infop = png_create_info_struct(readp)))
        return image_punt("Failure creating PNG info struct");

    /* Enable the default PNG error handler. */

    if (setjmp(png_jmpbuf(readp)) == 0)
    {
        int w, h, b, c, r, i;

        /* Read the PNG header. */

        png_init_io(readp, fp);
        png_read_png(readp, infop,
                     PNG_TRANSFORM_STRIP_16 |
                     PNG_TRANSFORM_PACKING, NULL);
        
        /* Extract and check image properties. */

        w = (int) png_get_image_width (readp, infop);
        h = (int) png_get_image_height(readp, infop);

        switch (png_get_color_type(readp, infop))
        {
        case PNG_COLOR_TYPE_GRAY:       b = 1; break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: b = 2; break;
        case PNG_COLOR_TYPE_RGB:        b = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  b = 4; break;

        default: return image_punt("Unsupported PNG color type");
        }

        /* Read the pixel data. */

        if (!(bytep = png_get_rows(readp, infop)))
            return image_punt("Failure reading PNG pixel data");

        /* Allocate the final pixel buffer and copy pixels there. */

        p = (GLubyte *) malloc(w * h * b);

        for (r = 0; r < h; r++)
            for (c = 0; c < w; c++)
                for (i = 0; i < b; i++)
                    p[r*w*b + c*b + i] = (GLubyte) bytep[h - r - 1][c*b + i];

        *width  = w;
        *height = h;
        *bytes  = b;
    }
    else return image_punt("PNG read error");

    /* Free all resources. */

    png_destroy_read_struct(&readp, &infop, NULL);
    fclose(fp);

    return p;
}

/*---------------------------------------------------------------------------*/
