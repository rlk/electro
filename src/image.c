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
#include <string.h>
#include <png.h>

#include "utility.h"
#include "opengl.h"
#include "buffer.h"
#include "shared.h"
#include "entity.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

static void *image_punt(const char *message)
{
    fprintf(stderr, "Image error: %s\n", message);
    return NULL;
}

/*---------------------------------------------------------------------------*/

static int power_of_two(int n)
{
    int i = 1;

    while (i < n)
        i *= 2;

    return i;
}

/*---------------------------------------------------------------------------*/

GLuint image_make_tex(const void *p, int w, int h, int b)
{
    GLenum f = GL_RGB;
    GLuint o = 0;

    int W = power_of_two(w);
    int H = power_of_two(h);

    /* Create a GL texture object. */

    glGenTextures(1, &o);
    glBindTexture(GL_TEXTURE_2D, o);

    /* Determine the GL texture format from the byte count. */

    switch (b)
    {
    case 1: f = GL_LUMINANCE;       break;
    case 2: f = GL_LUMINANCE_ALPHA; break;
    case 3: f = GL_RGB;             break;
    case 4: f = GL_RGBA;            break;
    }

    /* Ensure that the image is power-of-two in size.  Generate mipmaps. */

    if (W != w || H != h)
    {
        void *P;

        if ((P = malloc(W * H * b)))
        {
            gluScaleImage(f, w, h, GL_UNSIGNED_BYTE, p,
                             W, H, GL_UNSIGNED_BYTE, P);
            gluBuild2DMipmaps(GL_TEXTURE_2D, b, W, H, f, GL_UNSIGNED_BYTE, P);

            free(P);
        }
    }
    else
        gluBuild2DMipmaps(GL_TEXTURE_2D, b, w, h, f, GL_UNSIGNED_BYTE, p);

    /* Enable mipmapping on it. */

    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);

    return o;
}

/*---------------------------------------------------------------------------*/

void *image_load_png(const char *filename, int *width,
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

#define IMAXINIT 256

static struct image *I;
static int           I_max;

static int image_exists(int id)
{
    return (I && 0 <= id && id < I_max && I[id].filename);
}

/*---------------------------------------------------------------------------*/

int image_init(void)
{
    if ((I = (struct image *) calloc(IMAXINIT, sizeof (struct image))))
    {
        I[0].filename = "default";
        I[0].texture  =   0;
        I[0].w        = 128;
        I[0].h        = 128;
        I[0].b        =   3;

        I_max = IMAXINIT;
        return 1;
    }
    return 0;
}

void image_draw(int id)
{
    if (image_exists(id))
        glBindTexture(GL_TEXTURE_2D, I[id].texture);
}

void *image_load(const char *filename, int *width,
                                       int *height,
                                       int *bytes)
{
    const char *extension = filename + strlen(filename) - 4;

    if (strcmp(extension, ".png") == 0 ||
        strcmp(extension, ".PNG") == 0)
        return image_load_png(filename, width, height, bytes);
    else
        return image_punt("Unsupported image format");
}

/*---------------------------------------------------------------------------*/

int image_send_create(const char *filename)
{
    int id;

    /* Scan the current images for an existing instance of the named file. */

    for (id = 0; id < IMAXINIT; ++id)
        if (I[id].filename && strcmp(I[id].filename, filename) == 0)
            return id;

    /* Didn't find it.  It's new. */

    if ((id = buffer_unused(I_max, image_exists)) >= 0)
    {
        /* Note the file name. */

        if ((I[id].filename = (char *) calloc(strlen(filename) + 1, 1)))
            strcpy(I[id].filename, filename);

        /* Load and pack the image. */

        if ((I[id].p = image_load_png(filename, &I[id].w, &I[id].h, &I[id].b)))
        {
            pack_event(EVENT_IMAGE_CREATE);
            pack_index(id);
            pack_index(I[id].w);
            pack_index(I[id].h);
            pack_index(I[id].b);
            pack_alloc(I[id].w * I[id].h * I[id].b, I[id].p);

            I[id].texture = image_make_tex(I[id].p, I[id].w, I[id].h, I[id].b);

            return id;
        }
    }
    return -1;
}

void image_recv_create(void)
{
    int  id = unpack_index();

    I[id].w = unpack_index();
    I[id].h = unpack_index();
    I[id].b = unpack_index();
    I[id].p = unpack_alloc(I[id].w * I[id].h * I[id].b);

    I[id].texture  = image_make_tex(I[id].p, I[id].w, I[id].h, I[id].b);
    I[id].filename = "exists";
}

/*---------------------------------------------------------------------------*/

void image_delete(int id)
{
    if (image_exists(id))
    {
        if (I[id].filename) free(I[id].filename);
        if (I[id].p)        free(I[id].p);

        if (glIsTexture(I[id].texture))
            glDeleteTextures(1, &I[id].texture);

        memset(I + id, 0, sizeof (struct image));
    }
}

/*---------------------------------------------------------------------------*/

void image_get_p(int id, int x, int y, unsigned char p[4])
{
    if (image_exists(id))
    {
        unsigned char *pixels = (unsigned char *) I[id].p;

        switch (I[id].b)
        {
        case 1:
            p[0] = pixels[I[id].w * y + x];
            p[1] = pixels[I[id].w * y + x];
            p[2] = pixels[I[id].w * y + x];
            p[3] = 0xff;
            break;
        case 2:
            p[0] = pixels[(I[id].w * y + x) * 2 + 0];
            p[1] = pixels[(I[id].w * y + x) * 2 + 0];
            p[2] = pixels[(I[id].w * y + x) * 2 + 0];
            p[3] = pixels[(I[id].w * y + x) * 2 + 1];
            break;
        case 3:
            p[0] = pixels[(I[id].w * y + x) * 3 + 0];
            p[1] = pixels[(I[id].w * y + x) * 3 + 1];
            p[2] = pixels[(I[id].w * y + x) * 3 + 2];
            p[3] = 0xff;
            break;
        case 4:
            p[0] = pixels[(I[id].w * y + x) * 3 + 0];
            p[1] = pixels[(I[id].w * y + x) * 3 + 1];
            p[2] = pixels[(I[id].w * y + x) * 3 + 2];
            p[3] = pixels[(I[id].w * y + x) * 3 + 3];
            break;
        }
    }
}

int image_get_w(int id)
{
    return image_exists(id) ? I[id].w : 128;
}

int image_get_h(int id)
{
    return image_exists(id) ? I[id].h : 128;
}

/*---------------------------------------------------------------------------*/

