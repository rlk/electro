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
#include "socket.h"
#include "utility.h"
#include "event.h"
#include "image.h"

#define NPOT(n) (((n) & ((n) - 1)) != 0)

/*---------------------------------------------------------------------------*/

#define FLAG_NPOT 1
#define FLAG_COMP 2

#define TYPE_MAP  1
#define TYPE_ENV  2
#define TYPE_ANI  3
#define TYPE_UDP  4

struct image_map
{
    char    *name;
    GLubyte *data;
};

struct image_env
{
    char    *name[6];
    GLubyte *data[6];
};

struct image_ani
{
    char    *name;
    GLubyte *data;

    int frame_0;
    int frame_i;
    int frame_n;

    int count_a;
    int count_b;
};

struct image_udp
{
    int sock;
};

union image_nfo
{
    struct image_map map;
    struct image_env env;
    struct image_ani ani;
    struct image_udp udp;
};

struct image
{
    int count;
    int state;
    int flags;
    int dirty;
    int type;
    int code;

    GLuint texture;

    int w;
    int h;
    int b;

    union image_nfo nfo;
};

static vector_t image;

static GLenum format[5] = {
    0,
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA,
    GL_RGB,
    GL_RGBA
};

/*---------------------------------------------------------------------------*/

#define BUFMAX (1024 * 1024)

struct header
{
    int   code;
    short r;
    short n;
    short w;
    short h;
    short b;
};

static GLubyte *buffer = NULL;

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
/*
#define PIXEL(r, g, b) (0xFF000000 | ((b) << 16) | ((g) <<  8) | (r))

static GLubyte byte(int n)
{
    if (n > 255) return 0xFF;
    if (n <   0) return 0x00;

    return (GLubyte) (n);
}

static void decode_Y411(GLubyte *p, int w, int h)
{
    GLuint  *dst = (GLuint  *) p + (w * h - 1) * 4;
    GLubyte *src = (GLubyte *) p + (w * h - 1) * 6;

    const int o = 16;
    const int k = 2 << o;

    const int Yc = (int) ( 1.164f * k);
    const int Ug = (int) (-0.391f * k);
    const int Ub = (int) ( 2.018f * k);
    const int Vr = (int) ( 1.596f * k);
    const int Vg = (int) (-0.813f * k);

    while (src >= p)
    {
        const int U  = (int) src[0] - 128;
        const int Y0 = (int) src[1] -  16;
        const int Y1 = (int) src[2] -  16;
        const int V  = (int) src[3] - 128;
        const int Y2 = (int) src[4] -  16;
        const int Y3 = (int) src[5] -  16;

        const int UVr =          V * Vr;
        const int UVg = U * Ug + V * Vg;
        const int UVb = U * Ub;

        const int Y0c = Y0 * Yc;
        const int Y1c = Y1 * Yc;
        const int Y2c = Y2 * Yc;
        const int Y3c = Y3 * Yc;

        int r, g, b;

        r = (Y0c + UVr) >> o;
        g = (Y0c + UVg) >> o;
        b = (Y0c + UVb) >> o;

        dst[0] = PIXEL(byte(r), byte(g), byte(b));

        r = (Y1c + UVr) >> o;
        g = (Y1c + UVg) >> o;
        b = (Y1c + UVb) >> o;

        dst[1] = PIXEL(byte(r), byte(g), byte(b));

        r = (Y2c + UVr) >> o;
        g = (Y2c + UVg) >> o;
        b = (Y2c + UVb) >> o;

        dst[2] = PIXEL(byte(r), byte(g), byte(b));

        r = (Y3c + UVr) >> o;
        g = (Y3c + UVg) >> o;
        b = (Y3c + UVb) >> o;

        dst[3] = PIXEL(byte(r), byte(g), byte(b));

        src -= 6;
        dst -= 4;
    }
}
*/
/*===========================================================================*/

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
                                              int *bytes,
                                              int *flags)
{
    void *pixels = NULL;

    if (filename)
    {
        const char *ext = filename + strlen(filename) - 4;
    
        if      (strcmp(ext, ".png") == 0 || strcmp(ext, ".PNG") == 0)
            pixels = load_png_image(filename, width, height, bytes);
        else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".JPG") == 0)
            pixels = load_jpg_image(filename, width, height, bytes);
        else
            pixels = error("Unsupported image format for '%s'", filename);

        if (NPOT(*width) || NPOT(*height))
            *flags = FLAG_NPOT;
        else
            *flags = 0;
    }
    return pixels;
}

/*===========================================================================*/

static int cmpname(const char *name1, const char *name2)
{
    if (name1 == NULL && name2 == NULL) return 0;
    if (name1 == NULL || name2 == NULL) return 1;

    return strcmp(name1, name2);
}

int dupe_create_image(int i)
{
    get_image(i)->count++;
    return i;
}

/*---------------------------------------------------------------------------*/
/* Plain ol' image map                                                       */

int send_create_image_map(const char *name)
{
    int i, n = vecnum(image);

    struct image *p;

    /* Return the default image on NULL. */

    if (name == NULL)
        return 0;

    /* Scan the current images for an existing instance of the named file. */

    for (i = 0; i < n; ++i)
        if ((p = get_image(i))->type == TYPE_MAP)
        {
            if (cmpname(name, p->nfo.map.name) == 0)
                return dupe_create_image(i);
        }

    /* Didn't find it.  It's new. */

    if ((i = new_image()) >= 0)
    {
        p = get_image(i);

        p->state = 0;
        p->count = 1;
        p->flags = 0;
        p->type  = TYPE_MAP;

        /* Store the name and image data. */

        p->nfo.map.name = memdup(name, strlen(name) + 1, 1);
        p->nfo.map.data = load_image(name, &p->w, &p->h, &p->b, &p->flags);

        /* Send the header and data. */

        send_event(EVENT_CREATE_IMAGE);
        send_event(TYPE_MAP);

        send_index(p->flags);
        send_index(p->w);
        send_index(p->h);
        send_index(p->b);
        send_array(p->nfo.map.data, p->w * p->h * p->b, 1);

        return i;
    }
    return -1;
}

static void recv_create_image_map(void)
{
    /* Initialize a new map image. */

    struct image *p = get_image(new_image());

    p->state = 0;
    p->count = 1;
    p->type  = TYPE_MAP;

    /* Receive the header. */

    p->flags = recv_index();
    p->w     = recv_index();
    p->h     = recv_index();
    p->b     = recv_index();

    /* Receive the image data. */

    if ((p->nfo.map.data =   malloc(p->w * p->h * p->b)))
        recv_array(p->nfo.map.data, p->w * p->h * p->b, 1);
}

static GLuint init_image_map(struct image_map *nfo,
                             int w, int h, int b, int flags)
{
    GLuint o = 0;
    GLenum f = format[b];
    GLenum m = GL_TEXTURE_2D;

    glGenTextures(1, &o);

    /* Bind the texture as power-of-two or rectangular. */

    if ((flags & FLAG_NPOT) && GL_has_texture_rectangle)
        m = GL_TEXTURE_RECTANGLE_ARB;

    glBindTexture(m, o);

    /* Apply pixel data in compressed or raw format. */

    if ((flags & FLAG_COMP) && GL_has_texture_compression)
        glCompressedTexImage2DARB(m, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                  w, h, 0, w * h / 2, nfo->data);
    else
        glTexImage2D(m, 0, f, w, h, 0, f, GL_UNSIGNED_BYTE, nfo->data);

    /* Enable bilinear filtering. */

    glTexParameteri(m, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(m, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return o;
}

static void free_image_map(struct image_map *nfo)
{
    /* Release the pixel data buffer and name string buffer. */

    if (nfo->data) free(nfo->data);
    if (nfo->name) free(nfo->name);
}

/*---------------------------------------------------------------------------*/

int send_create_image_env(const char *name_nx,
                          const char *name_px,
                          const char *name_ny,
                          const char *name_py,
                          const char *name_nz,
                          const char *name_pz)
{
    int f, i, j, n = vecnum(image);

    struct image *p;

    /* Scan the current images for an existing instance of the named files. */

    for (i = 0; i < n; ++i)
        if ((p = get_image(i))->type == TYPE_ENV)
        {
            if (cmpname(name_nx, p->nfo.env.name[0]) == 0 &&
                cmpname(name_px, p->nfo.env.name[1]) == 0 &&
                cmpname(name_ny, p->nfo.env.name[2]) == 0 &&
                cmpname(name_py, p->nfo.env.name[3]) == 0 &&
                cmpname(name_nz, p->nfo.env.name[4]) == 0 &&
                cmpname(name_pz, p->nfo.env.name[5]) == 0)
                return dupe_create_image(i);
        }

    /* Didn't find it.  It's new. */

    if ((i = new_image()) >= 0)
    {
        p = get_image(i);

        p->state = 0;
        p->count = 1;
        p->flags = 0;
        p->type  = TYPE_ENV;

        /* Note the file names. */

        p->nfo.env.name[0] = memdup(name_nx, strlen(name_nx) + 1, 1);
        p->nfo.env.name[1] = memdup(name_px, strlen(name_px) + 1, 1);
        p->nfo.env.name[2] = memdup(name_ny, strlen(name_ny) + 1, 1);
        p->nfo.env.name[3] = memdup(name_py, strlen(name_py) + 1, 1);
        p->nfo.env.name[4] = memdup(name_nz, strlen(name_nz) + 1, 1);
        p->nfo.env.name[5] = memdup(name_pz, strlen(name_pz) + 1, 1);

        /* Load the image data. */

        p->nfo.env.data[0] = load_image(name_nx, &p->w, &p->h, &p->b, &f);
        p->nfo.env.data[1] = load_image(name_px, &p->w, &p->h, &p->b, &f);
        p->nfo.env.data[2] = load_image(name_ny, &p->w, &p->h, &p->b, &f);
        p->nfo.env.data[3] = load_image(name_py, &p->w, &p->h, &p->b, &f);
        p->nfo.env.data[4] = load_image(name_nz, &p->w, &p->h, &p->b, &f);
        p->nfo.env.data[5] = load_image(name_pz, &p->w, &p->h, &p->b, &f);
        
        p->flags = f;

        /* Send the header and data. */

        send_event(EVENT_CREATE_IMAGE);
        send_event(TYPE_ENV);

        send_index(p->flags);
        send_index(p->w);
        send_index(p->h);
        send_index(p->b);

        for (j = 0; j < 6; j++)
            send_array(p->nfo.env.data[j], p->w * p->h * p->b, 1);

        return i;
    }
    return -1;
}

static void recv_create_image_env(void)
{
    /* Initialize a new env image. */

    struct image *p = get_image(new_image());
    int j;

    p->state = 0;
    p->count = 1;
    p->type  = TYPE_ENV;

    /* Receive the header. */

    p->flags = recv_index();
    p->w     = recv_index();
    p->h     = recv_index();
    p->b     = recv_index();

    /* Receive the image data. */

    for (j = 0; j < 6; ++j)
        if ((p->nfo.env.data[j] =   malloc(p->w * p->h * p->b)))
            recv_array(p->nfo.env.data[j], p->w * p->h * p->b, 1);
}

static GLuint init_image_env(struct image_env *nfo,
                             int w, int h, int b, int flags)
{
    GLuint o;

    glGenTextures(1, &o);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, o);

    /* Apply pixel data to each of the cube map faces. */

    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, format[b],
                      w, h, format[b], GL_UNSIGNED_BYTE, nfo->data[0]);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, format[b],
                      w, h, format[b], GL_UNSIGNED_BYTE, nfo->data[1]);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, format[b],
                      w, h, format[b], GL_UNSIGNED_BYTE, nfo->data[2]);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, format[b],
                      w, h, format[b], GL_UNSIGNED_BYTE, nfo->data[3]);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, format[b],
                      w, h, format[b], GL_UNSIGNED_BYTE, nfo->data[4]);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, format[b],
                      w, h, format[b], GL_UNSIGNED_BYTE, nfo->data[5]);

    /* Enable trilinear filtering. */

    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    /* Clamp all cube map sides. */

    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                    GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                    GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                    GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return o;
}

static void free_image_env(struct image_env *nfo)
{
    int i;

    /* Release all pixel data buffers and name string buffers. */

    for (i = 0; i < 6; ++i)
    {
        if (nfo->name[i]) free(nfo->name[i]);
        if (nfo->data[i]) free(nfo->data[i]);
    }
}

/*---------------------------------------------------------------------------*/

int send_create_image_ani(const char *name, int w, int h, int b,
                                            int f0, int fn, int ca, int cb)
{
    int i;

    /* Initialize a new animated image. */

    if ((i = new_image()) >= 0)
    {
        struct image *p = get_image(i);

        p->state = 0;
        p->count = 1;
        p->flags = (NPOT(w) || NPOT(h)) ? FLAG_NPOT : 0;
        p->type  = TYPE_ANI;

        p->w = w;
        p->h = h;
        p->b = b;

        p->nfo.ani.frame_0 = f0;
        p->nfo.ani.frame_i = f0;
        p->nfo.ani.frame_n = fn;
        p->nfo.ani.count_a = ca;
        p->nfo.ani.count_b = cb;

        /* HACK: recognize compressed textures as having a DXT extension. */

        if (strcmp(name + strlen(name) - 3, "dxt") == 0)
            p->flags |= FLAG_COMP;

        /* Store the name and allocate a data buffer. */

        p->nfo.map.name = memdup(name, strlen(name) + 1, 1);

        if (p->flags & FLAG_COMP)
            p->nfo.map.data = (GLubyte *) calloc(p->w * p->h / 2, 1);
        else
            p->nfo.map.data = (GLubyte *) calloc(p->w * p->h * p->b, 1);
        
        /* Send the header. */

        send_event(EVENT_CREATE_IMAGE);
        send_event(TYPE_ANI);

        send_index(p->flags);
        send_index(p->w);
        send_index(p->h);
        send_index(p->b);

        return i;
    }
    return -1;
}

static void recv_create_image_ani(void)
{
    /* Initialize a new animated image. */

    struct image *p = get_image(new_image());

    p->state = 0;
    p->count = 1;
    p->type  = TYPE_ANI;

    /* Receive the header. */

    p->flags = recv_index();
    p->w     = recv_index();
    p->h     = recv_index();
    p->b     = recv_index();

    /* Allocate the image data buffer. */

    if (p->flags & FLAG_COMP)
        p->nfo.ani.data = (GLubyte *) calloc(p->w * p->h / 2, 1);
    else
        p->nfo.ani.data = (GLubyte *) calloc(p->w * p->h * p->b, 1);
}

static GLuint init_image_ani(struct image_ani *nfo,
                             int w, int h, int b, int flags)
{
    GLuint o = 0;
    GLenum m = GL_TEXTURE_2D;

    glGenTextures(1, &o);

    /* Bind the texture as power-of-two or rectangular. */

    if ((flags & FLAG_NPOT) && GL_has_texture_rectangle)
        m = GL_TEXTURE_RECTANGLE_ARB;

    glBindTexture(m, o);

    /* Apply the pixel data buffer. */

    if ((flags & FLAG_COMP) && GL_has_texture_compression)
        glCompressedTexImage2DARB(m, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                  w, h, 0, w * h / 2, nfo->data);
    else
        glTexImage2D(m, 0, format[b], w, h, 0,
                     format[b], GL_UNSIGNED_BYTE, nfo->data);

    /* Enable bilinear filtering. */

    glTexParameteri(m, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(m, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return o;
}

static void free_image_ani(struct image_ani *nfo)
{
    /* Release the pixel data buffer and name string buffer. */

    if (nfo->data) free(nfo->data);
    if (nfo->name) free(nfo->name);
}

static void set_image_ani_pixels(int i, GLubyte *data, int code,
                                 int r, int n, int w, int h, int b)
{
    struct image *p = get_image(i);

    GLenum m;

    /* Bind the texture as power-of-two or rectangular. */

    if ((p->flags & FLAG_NPOT) && GL_has_texture_rectangle)
        m = GL_TEXTURE_RECTANGLE_ARB;
    else
        m = GL_TEXTURE_2D;

    glBindTexture(m, p->texture);

    /* Apply the new pixel data as compressed or raw. */

    if ((p->flags & FLAG_COMP) && GL_has_texture_compression)
        glCompressedTexSubImage2DARB(m, 0, 0, r, w, n,
                                     GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                     w * h / 2, data);
    else
        glTexSubImage2D(m, 0, 0, r, w, n,
                        format[b], GL_UNSIGNED_BYTE, data);
}

static void step_image_ani(int i)
{
    struct image *p = get_image(i);

    char filename[MAXSTR];
    FILE *fp;

    /* Build the file name from the name format and frame number. */

    sprintf(filename, p->nfo.ani.name, p->nfo.ani.frame_i);

    if ((fp = fopen(filename, "rb")))
    {
        /* Load compressed or raw pixel data. */

        if ((p->flags & FLAG_COMP) && GL_has_texture_compression)
            fread(p->nfo.ani.data, 1, p->w * p->h / 2, fp);
        else
            fread(p->nfo.ani.data, 1, p->w * p->h * p->b, fp);

        /* Distribute the loaded pixels. */

        send_set_image_pixels(i, p->nfo.ani.data, 0, 0,
                              p->h, p->w, p->h, p->b);

        /* Advance the animation. */

        if (p->nfo.ani.frame_i < p->nfo.ani.frame_n)
            p->nfo.ani.frame_i = p->nfo.ani.frame_i + 1;
        else
            p->nfo.ani.frame_i = p->nfo.ani.frame_0;

        fclose(fp);
    }
}

/*---------------------------------------------------------------------------*/

int send_create_image_udp(int port)
{
    int i, n = 1024 * 1024;
    SOCKET s = INVALID_SOCKET;

    if ((i = new_image()) >= 0)
    {
        struct image *p = get_image(i);

        p->state = 0;
        p->count = 1;
        p->flags = 0;
        p->code  = 0;
        p->type  = TYPE_UDP;

        p->w = 0;
        p->h = 0;
        p->b = 0;

        p->nfo.udp.sock = INVALID_SOCKET;

        /* Open a UDP socket for receiving. */

        if ((s = socket(PF_INET, SOCK_DGRAM, 0)) >= 0)
        {
            /* Increase the receive buffer size to handle a gigabit stream. */

            if (setsockopt(s, SOL_SOCKET,
                              SO_RCVBUF, (char *) &n, sizeof (int)) >= 0)
            {
                sockaddr_t addr;
                size_t     addr_len = sizeof (sockaddr_t);

                /* Accept connections from any address on the given port. */

                addr.sin_family      = AF_INET;
                addr.sin_port        = htons((short) port);
                addr.sin_addr.s_addr = INADDR_ANY;

                /* Bind the socket to this address. */

                if (bind(s, (struct sockaddr *) &addr, addr_len) >= 0)
                {
                    p->nfo.udp.sock = s;

                    /* Notify the clients of the new streaming image. */

                    send_event(EVENT_CREATE_IMAGE);
                    send_event(TYPE_UDP);

                    return i;
                }
                else error("bind %d : %s", port, system_error());
            }
            else error("setsockopt : %s", system_error());
        }
        else error("socket : %s", system_error());
    }
    return -1;
}

static void recv_create_image_udp(void)
{
    /* initialize a new streaming image. */

    struct image *p = get_image(new_image());

    p->state = 0;
    p->count = 1;
    p->flags = 0;
    p->code  = 0;
    p->type  = TYPE_UDP;

    /* Streaming images are configured on data receipt. */

    p->w = 0;
    p->h = 0;
    p->b = 0;

    p->nfo.udp.sock = INVALID_SOCKET;
}

static GLuint init_image_udp(struct image_udp *nfo,
                             int w, int h, int b, int flags)
{
    GLuint o = 0;
    GLenum f = format[b];
    GLenum m = GL_TEXTURE_2D;

    glGenTextures(1, &o);

    /* Bind the texture as power-of-two or rectangular. */

    if ((flags & FLAG_NPOT) && GL_has_texture_rectangle)
        m = GL_TEXTURE_RECTANGLE_ARB;

    glBindTexture(m, o);

    /* Apply an empty pixel data buffer. */

    glTexImage2D(m, 0, f, w, h, 0, f, GL_UNSIGNED_BYTE, NULL);

    /* Enable bilinear filtering. */

    glTexParameteri(m, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(m, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return o;
}

static void free_image_udp(struct image_udp *nfo)
{
    if (nfo->sock != INVALID_SOCKET)
        close(nfo->sock);
}

static void set_image_udp_pixels(int i, GLubyte *data, int code, 
                                 int r, int n, int w, int h, int b)
{
    struct image *p = get_image(i);

    GLenum m;

    /* If the image size or type has changed, flush the OpenGL state. */
    
    if (p->code != code || p->w != w || p->h != h || p->b != b)
    {
        p->code = code;
        p->w    = w;
        p->h    = h;
        p->b    = b;

        fini_image(i);
        init_image(i);
    }

    /* Bind the texture as power-of-two or rectangular. */

    if ((p->flags & FLAG_NPOT) && GL_has_texture_rectangle)
        m = GL_TEXTURE_RECTANGLE_ARB;
    else
        m = GL_TEXTURE_2D;

    glBindTexture(m, p->texture);

    /* Apply the new pixel data as compressed or raw. */

    if ((p->flags & FLAG_COMP) && GL_has_texture_compression)
        glCompressedTexSubImage2DARB(m, 0, 0, r, w, n,
                                     GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                     w * h / 2, data);
    else
        glTexSubImage2D(m, 0, 0, r, w, n,
                        format[b], GL_UNSIGNED_BYTE, data);
}

static void step_image_udp(int i)
{
    struct image *p = get_image(i);

    struct header *head = (struct header *) buffer;
    GLubyte       *data =       (GLubyte *) buffer + sizeof (struct header);

    /* Receive an incoming subimage. */

    if ((int) recv(p->nfo.udp.sock, buffer, BUFMAX, 0) > 0)
    {
        /* Decode the subimage header. */

        int code = ntohl(head->code);
        int r    = ntohs(head->r);
        int n    = ntohs(head->n);
        int w    = ntohs(head->w);
        int h    = ntohs(head->h);
        int b    = ntohs(head->b);

        /* Distribute the new pixel data. */

        send_set_image_pixels(i, data, code, r, n, w, h, b);
    }
}

/*---------------------------------------------------------------------------*/

void send_set_image_pixels(int i, void *data, int code, int r, int n,
                                                        int w, int h, int b)
{
    send_event(EVENT_SET_IMAGE_PIXELS);
    send_index(i);
    send_index(code);
    send_index(r);
    send_index(n);
    send_index(w);
    send_index(h);
    send_index(b);
    send_array(data, MIN(w * h * b, BUFMAX), 1);

    switch (get_image(i)->type)
    {
    case TYPE_ANI: set_image_ani_pixels(i, data, code, r, n, w, h, b); break;
    case TYPE_UDP: set_image_udp_pixels(i, data, code, r, n, w, h, b); break;
    }
}

void recv_set_image_pixels(void)
{
    int i    = recv_index();
    int code = recv_index();
    int r    = recv_index();
    int n    = recv_index();
    int w    = recv_index();
    int h    = recv_index();
    int b    = recv_index();

    recv_array(buffer, MIN(w * n * b, BUFMAX), 1);

    switch (get_image(i)->type)
    {
    case TYPE_ANI: set_image_ani_pixels(i, buffer, code, r, n, w, h, b); break;
    case TYPE_UDP: set_image_udp_pixels(i, buffer, code, r, n, w, h, b); break;
    }
}

/*---------------------------------------------------------------------------*/

void recv_create_image(void)
{
    switch (recv_event())
    {
    case TYPE_MAP: recv_create_image_map(); break;
    case TYPE_ENV: recv_create_image_env(); break;
    case TYPE_ANI: recv_create_image_ani(); break;
    case TYPE_UDP: recv_create_image_udp(); break;
    }
}

/*---------------------------------------------------------------------------*/

static int free_image(int i)
{
    struct image *p = get_image(i);

    if (i > 0)
    {
        if (p->count > 0)
        {
            p->count--;

            if (p->count == 0)
            {
                fini_image(i);

                switch (p->type)
                {
                case TYPE_MAP: free_image_map(&p->nfo.map); break;
                case TYPE_ENV: free_image_env(&p->nfo.env); break;
                case TYPE_ANI: free_image_ani(&p->nfo.ani); break;
                case TYPE_UDP: free_image_udp(&p->nfo.udp); break;
                };

                memset(p, 0, sizeof (struct image));

                return 1;
            }
        }
    }
    return 0;
}

void send_delete_image(int i)
{
    if (get_rank() == 0 && free_image(i))
    {
        send_event(EVENT_DELETE_IMAGE);
        send_index(i);
    }
}

void recv_delete_image(void)
{
    free_image(recv_index());
}

/*---------------------------------------------------------------------------*/

void get_image_c(int i, int x, int y, unsigned char c[4])
{
    struct image  *p = get_image(i);
    unsigned char *data = (unsigned char *) p->nfo.map.data;

    c[0] = c[1] = c[2] = c[3] = 0xFF;

    /* Return a pixel in any format as RGBA format. */

    if (p->type == TYPE_MAP && data && 0 <= x && x < p->w &&
                                       0 <= y && y < p->h)
        switch (p->b)
        {
        case 1:
            c[0] = data[p->w * y + x];
            c[1] = data[p->w * y + x];
            c[2] = data[p->w * y + x];
            c[3] = 0xff;
            break;
        case 2:
            c[0] = data[(p->w * y + x) * 2 + 0];
            c[1] = data[(p->w * y + x) * 2 + 0];
            c[2] = data[(p->w * y + x) * 2 + 0];
            c[3] = data[(p->w * y + x) * 2 + 1];
            break;
        case 3:
            c[0] = data[(p->w * y + x) * 3 + 0];
            c[1] = data[(p->w * y + x) * 3 + 1];
            c[2] = data[(p->w * y + x) * 3 + 2];
            c[3] = 0xff;
            break;
        case 4:
            c[0] = data[(p->w * y + x) * 4 + 0];
            c[1] = data[(p->w * y + x) * 4 + 1];
            c[2] = data[(p->w * y + x) * 4 + 2];
            c[3] = data[(p->w * y + x) * 4 + 3];
            break;
        }
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
        p->state = 1;

        switch (p->type)
        {
        case TYPE_MAP:
            p->texture = init_image_map(&p->nfo.map,
                                         p->w, p->h, p->b, p->flags); break;
        case TYPE_ENV:
            p->texture = init_image_env(&p->nfo.env,
                                         p->w, p->h, p->b, p->flags); break;
        case TYPE_ANI:
            p->texture = init_image_ani(&p->nfo.ani,
                                         p->w, p->h, p->b, p->flags); break;
        case TYPE_UDP:
            p->texture = init_image_udp(&p->nfo.udp,
                                         p->w, p->h, p->b, p->flags); break;
        }
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
    struct image *p = get_image(i);

    if (p->count)
    {
        init_image(i);

        if (p->texture)
        {
            GLenum t;

            /* Ensure the wrong texture unit types are disabled. */

            glDisable(GL_TEXTURE_CUBE_MAP_ARB);
            glDisable(GL_TEXTURE_RECTANGLE_ARB);
            glDisable(GL_TEXTURE_2D);

            /* Determine the right texture unit type and enable it. */

            if      (p->type == TYPE_ENV)  t = GL_TEXTURE_CUBE_MAP_ARB;
            else if (p->flags & FLAG_NPOT) t = GL_TEXTURE_RECTANGLE_ARB;
            else                           t = GL_TEXTURE_2D;
            
            glEnable(t);
            glBindTexture(t, get_image(i)->texture);
        }
    }
}

/*---------------------------------------------------------------------------*/

void nuke_images(void)
{
    int i, n = vecnum(image);

    for (i = 1; i < n; ++i)
        while (get_image(i)->count)
            send_delete_image(i);
}

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

void step_images(void)
{
    struct timeval zero = { 0, 0 };

    int s, i, m = 0, n = vecnum(image);

    fd_set fds0;
    fd_set fds1;

    /* Initialize a descriptor set including all video sockets. */

    FD_ZERO(&fds0);

    for (i = 0; i < n; ++i)
    {
        struct image *p = get_image(i);

        if (p->count && p->type == TYPE_UDP)
        {
            FD_SET(p->nfo.udp.sock, &fds0);

            if (m < (int) p->nfo.udp.sock + 1)
                m = (int) p->nfo.udp.sock + 1;
        }
    }

    memcpy(&fds1, &fds0, sizeof (fd_set));

    /* Handle all video socket activity. */

    if (m > 0)
        while ((s = select(m, &fds1, NULL, NULL, &zero)) > 0)
        {
            for (i = 0; i < n; ++i)
            {
                struct image *p = get_image(i);

                if (p->count && p->type == TYPE_UDP)
                {
                    if (FD_ISSET(p->nfo.udp.sock, &fds1))
                        step_image_udp(i);
                }
            }
            memcpy(&fds1, &fds0, sizeof (fd_set));
        }

    /* Handle all flipbook images. */

    for (i = 0; i < n; ++i)
    {
        struct image *p = get_image(i);

        if (p->count && p->type == TYPE_ANI)
            step_image_ani(i);
    }
}

/*---------------------------------------------------------------------------*/

int startup_image(void)
{
    int i;

    if ((image = vecnew(256, sizeof (struct image))))
    {
        if ((buffer = (GLubyte *) malloc(BUFMAX)))
        {
            if ((i = new_image()) >= 0)
            {
                struct image *p = get_image(i);

                p->type    = TYPE_MAP;
                p->count   = 1;
                p->state   = 0;
                p->flags   = 0;
                p->texture = 0;

                p->w = 128;
                p->h = 128;
                p->b = 4;

                p->nfo.map.data = malloc(128 * 128 * 4);

                memset(p->nfo.map.data, 0xFF, 128 * 128 * 4);

                return 1;
            }
        }
    }
    return 0;
}
