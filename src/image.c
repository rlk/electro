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
#include <jpeglib.h>
#include <png.h>

#include "opengl.h"
#include "vector.h"
#include "buffer.h"
#include "utility.h"
#include "event.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

struct image
{
    int    count;
    int    state;
    GLuint texture;
    char  *filename;
    void  *p;
    int    w;
    int    h;
    int    b;
};

static vector_t image;

/*---------------------------------------------------------------------------*/

static struct image *get_image(int i)
{
    return (struct image *) vecget(image, i);
}

static int new_image(void)
{
    int i, n = vecnum(image);

    for (i = 0; i < n; ++i)
        if (get_image(i)->count == 0)
            return i;

    return vecadd(image);
}

/*===========================================================================*/

static int power_of_two(int n)
{
    int i = 1;

    while (i < n)
        i *= 2;

    return i;
}

GLuint make_texture(const void *p, int w, int h, int b)
{
    GLenum i = GL_RGB;
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
    case 1: f = i = GL_LUMINANCE;       break;
    case 2: f = i = GL_LUMINANCE_ALPHA; break;
    case 3: f = i = GL_RGB;             break;
    case 4: f = i = GL_RGBA;            break;
    }

/*
    if (GL_has_texture_compression)
        switch (b)
        {
        case 1: i = GL_COMPRESSED_LUMINANCE_ARB;       break;
        case 2: i = GL_COMPRESSED_LUMINANCE_ALPHA_ARB; break;
        case 3: i = GL_COMPRESSED_RGB_ARB;             break;
        case 4: i = GL_COMPRESSED_RGBA_ARB;            break;
        }
*/

    /* Ensure that the image is power-of-two in size.  Generate mipmaps. */

    if (W != w || H != h)
    {
        void *P;

        if ((P = malloc(W * H * b)))
        {
            gluScaleImage(f, w, h, GL_UNSIGNED_BYTE, p,
                             W, H, GL_UNSIGNED_BYTE, P);
            gluBuild2DMipmaps(GL_TEXTURE_2D, i, W, H, f, GL_UNSIGNED_BYTE, P);

            free(P);
        }
    }
    else
        gluBuild2DMipmaps(GL_TEXTURE_2D, i, w, h, f, GL_UNSIGNED_BYTE, p);


    /* Enable mipmapping on it. */

    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return o;
}

/*---------------------------------------------------------------------------*/

static void *load_png_image(const char *filename, int *width,
                                                  int *height,
                                                  int *bytes)
{
    GLubyte *p = NULL;
    FILE   *fp;

    png_structp readp = NULL;
    png_infop   infop = NULL;
    png_bytep  *bytep = NULL;

    /* Initialize all PNG import data structures. */

    if (!(fp = open_file(filename, FMODE_RB)))
        return error("PNG file '%s': %s", filename, system_error());

    if (!(readp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        return error("Failure creating PNG read struct '%s'", filename);
        
    if (!(infop = png_create_info_struct(readp)))
        return error("Failure creating PNG info struct '%s'", filename);

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

        default: return error("Unsupported PNG color type '%s'", filename);
        }

        /* Read the pixel data. */

        if (!(bytep = png_get_rows(readp, infop)))
            return error("Failure reading PNG pixel data '%s'", filename);

        /* Allocate the final pixel buffer and copy pixels there. */

        if ((p = (GLubyte *) malloc(w * h * b)))
        {
            for (r = 0; r < h; r++)
                for (c = 0; c < w; c++)
                    for (i = 0; i < b; i++)
                        p[r*w*b + c*b + i] = (GLubyte) bytep[h-r-1][c*b+i];

            *width  = w;
            *height = h;
            *bytes  = b;
        }
    }
    else return error("PNG read error");

    /* Free all resources. */

    png_destroy_read_struct(&readp, &infop, NULL);
    fclose(fp);

    return p;
}

static void *load_jpg_image(const char *filename, int *width,
                                                  int *height,
                                                  int *bytes)
{
    GLubyte *p = NULL;
    FILE   *fp;

    if ((fp = open_file(filename, FMODE_RB)))
    {
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr         jerr;
        int w, h, b, i = 0;

        /* Initialize the JPG decompressor. */

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, fp);

        /* Grab the JPG header info. */

        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);

        w = cinfo.output_width;
        h = cinfo.output_height;
        b = cinfo.output_components;

        /* Allocate the final pixel buffer and copy pixels there. */

        if ((p = (GLubyte *) malloc (w * h * b)))
        {
            while (cinfo.output_scanline < cinfo.output_height)
            {
                GLubyte *buffer = p + w * b * (h - i - 1);
                i += jpeg_read_scanlines(&cinfo, &buffer, 1);
            }

            *width  = w;
            *height = h;
            *bytes  = b;
        }

        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        fclose(fp);
    }
    else error("JPG file '%s': %s", filename, system_error());

    return p;
}

static void *load_image(const char *filename, int *width,
                                              int *height,
                                              int *bytes)
{
    const char *extension = filename + strlen(filename) - 4;
    
    if      (strcmp(extension, ".png") == 0 || strcmp(extension, ".PNG") == 0)
        return load_png_image(filename, width, height, bytes);
    else if (strcmp(extension, ".jpg") == 0 || strcmp(extension, ".JPG") == 0)
        return load_jpg_image(filename, width, height, bytes);
    else
        return error("Unsupported image format for '%s'", filename);
}

/*===========================================================================*/

int send_create_image(const char *filename)
{
    int i, n = vecnum(image);

    /* Return the default texture on NULL. */

    if (filename == NULL)
        return 0;

    /* Scan the current images for an existing instance of the named file. */

    for (i = 0; i < n; ++i)
    {
        struct image *p = get_image(i);

        if (filename && p->filename && strcmp(p->filename, filename) == 0)
        {
            p->count++;
            return i;
        }
    }

    /* Didn't find it.  It's new. */

    if ((i = new_image()) >= 0)
    {
        struct image *p = get_image(i);

        p->count = 1;

        /* Note the file name. */

        p->filename = memdup(filename, strlen(filename) + 1, 1);

        /* Load and pack the image. */

        if ((p->p = load_image(filename, &p->w, &p->h, &p->b)))
        {
            send_event(EVENT_CREATE_IMAGE);
            send_index(p->w);
            send_index(p->h);
            send_index(p->b);
            send_array(p->p, p->w * p->h, p->b);

            return i;
        }
    }
    return -1;
}

void recv_create_image(void)
{
    struct image *p = get_image(new_image());

    p->w = recv_index();
    p->h = recv_index();
    p->b = recv_index();
    p->p = (GLubyte *) malloc(p->w * p->h * p->b);

    recv_array(p->p, p->w * p->h, p->b);

    p->count = 1;
}

/*---------------------------------------------------------------------------*/

void get_image_c(int i, int x, int y, unsigned char c[4])
{
    struct image  *p      = get_image(i);
    unsigned char *pixels = (unsigned char *) p->p;

    /* Return a pixel in any format as RGBA format. */

    if (pixels)
        switch (p->b)
        {
        case 1:
            c[0] = pixels[p->w * y + x];
            c[1] = pixels[p->w * y + x];
            c[2] = pixels[p->w * y + x];
            c[3] = 0xff;
            break;
        case 2:
            c[0] = pixels[(p->w * y + x) * 2 + 0];
            c[1] = pixels[(p->w * y + x) * 2 + 0];
            c[2] = pixels[(p->w * y + x) * 2 + 0];
            c[3] = pixels[(p->w * y + x) * 2 + 1];
            break;
        case 3:
            c[0] = pixels[(p->w * y + x) * 3 + 0];
            c[1] = pixels[(p->w * y + x) * 3 + 1];
            c[2] = pixels[(p->w * y + x) * 3 + 2];
            c[3] = 0xff;
            break;
        case 4:
            c[0] = pixels[(p->w * y + x) * 3 + 0];
            c[1] = pixels[(p->w * y + x) * 3 + 1];
            c[2] = pixels[(p->w * y + x) * 3 + 2];
            c[3] = pixels[(p->w * y + x) * 3 + 3];
            break;
        }
    else
        c[0] = c[1] = c[2] = c[3] = 0xFF;
}

int get_image_w(int i)
{
    return get_image(i)->w;
}

int get_image_h(int i)
{
    return get_image(i)->h;
}

/*===========================================================================*/

void init_image(int i)
{
    struct image *p = get_image(i);

    if (p->state == 0)
    {
        p->texture = make_texture(p->p, p->w, p->h, p->b);
        p->state   = 1;
    }
}

void fini_image(int i)
{
    struct image *p = get_image(i);

    if (p->state == 1)
    {
        if (glIsTexture(p->texture))
            glDeleteTextures(1, &p->texture);

        p->texture = 0;
        p->state   = 0;
    }
}

void draw_image(int i)
{
    init_image(i);
    glBindTexture(GL_TEXTURE_2D, get_image(i)->texture);
}

void dupe_image(int i)
{
    get_image(i)->count++;
}

void free_image(int i)
{
    struct image *p = get_image(i);

    if (i > 0 && --p->count == 0)
    {
        fini_image(i);

        if (p->filename) free(p->filename);
        if (p->p)        free(p->p);

        memset(p, 0, sizeof (struct image));
    }
}

/*---------------------------------------------------------------------------*/

void init_images(void)
{
    int i, n = vecnum(image);

    for (i = 0; i < n; ++i)
        if (get_image(i)->count)
            init_image(i);
}

void fini_images(void)
{
    int i, n = vecnum(image);

    for (i = 0; i < n; ++i)
        if (get_image(i)->count)
            fini_image(i);
}

/*---------------------------------------------------------------------------*/

int startup_image(void)
{
    int i;

    if ((image = vecnew(256, sizeof (struct image))))
    {
        if ((i = new_image()) >= 0)
        {
            struct image *p = get_image(i);

            p->texture  =    0;
            p->state    =    0;
            p->count    =    1;
            p->w        =  128;
            p->h        =  128;
            p->b        =    4;
            p->p        = malloc(128 * 128 * 4);

            memset(p->p, 0xFF, 128 * 128 * 4);
        }
        return 1;
    }
    else
        return 0;
}
