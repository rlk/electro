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

/*---------------------------------------------------------------------------*/

#define NPOT(n) (((n) & ((n) - 1)) != 0)

void snap(char *filename)
{
    FILE       *filep  = NULL;
    png_structp writep = NULL;
    png_infop   infop  = NULL;
    png_bytep  *bytep  = NULL;

    int w = 2560;
    int h = 1600;
    int i;

    unsigned char *p = NULL;

    /* Initialize all PNG export data structures. */

    if (!(filep = fopen(filename, "wb")))
        return;
    if (!(writep = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        return;
    if (!(infop = png_create_info_struct(writep)))
        return;

    /* Enable the default PNG error handler. */

    if (setjmp(png_jmpbuf(writep)) == 0)
    {
        /* Initialize the PNG header. */

        png_init_io (writep, filep);
        png_set_IHDR(writep, infop, w, h, 8,
                     PNG_COLOR_TYPE_RGB,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT,
                     PNG_FILTER_TYPE_DEFAULT);

        /* Allocate the pixel buffer and copy pixels there. */

        if ((p = (unsigned char *) malloc(w * h * 4)))
        {
            glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, p);

            /* Allocate and initialize the row pointers. */

            if ((bytep = png_malloc(writep, h * sizeof (png_bytep))))
            {
                for (i = 0; i < h; ++i)
                    bytep[h - i - 1] = (png_bytep) (p + i * w * 3);

                png_set_rows (writep, infop, bytep);

                /* Write the PNG image file. */

                png_write_info(writep, infop);
                png_write_png (writep, infop, 0, NULL);

                free(bytep);
            }
            free(p);
        }
    }

    /* Release all resources. */

    png_destroy_write_struct(&writep, &infop);
    fclose(filep);
}

/*---------------------------------------------------------------------------*/

#define FLAG_NPOT 1
#define FLAG_COMP 2

#define TYPE_MAP  1
#define TYPE_ENV  2
#define TYPE_ANI  3
#define TYPE_UDP  4
#define TYPE_FBO  5

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
    int count_c;
    int count_p;
};

struct image_udp
{
    int    dirty;
    int    sock;
    int    code;
    GLhandleARB frag;
    GLhandleARB prog;
    GLuint back;
    GLuint fbo;
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
    int type;

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

static void expand411(GLubyte *buf, int n)
{
    GLubyte *src = (GLubyte *) buf + n * 6 / 4;
    GLuint  *dst = (GLuint  *) buf + n;

    while (src >= buf)
    {
        const GLuint U  = (GLuint) src[0] << 8;
        const GLuint Y0 = (GLuint) src[1];
        const GLuint Y1 = (GLuint) src[2];
        const GLuint V  = (GLuint) src[3] << 16;
        const GLuint Y2 = (GLuint) src[4];
        const GLuint Y3 = (GLuint) src[5];

        dst[0] = 0xFF000000 | Y0 | U | V;
        dst[1] = 0xFF000000 | Y1 | U | V;
        dst[2] = 0xFF000000 | Y2 | U | V;
        dst[3] = 0xFF000000 | Y3 | U | V;

        src -= 6;
        dst -= 4;
    }
}

static void expand422(GLubyte *buf, int n)
{
    GLubyte *src = (GLubyte *) buf + n * 4 / 2;
    GLuint  *dst = (GLuint  *) buf + n;

    while (src >= buf)
    {
        const GLuint U  = (GLuint) src[0] << 8;
        const GLuint Y0 = (GLuint) src[1];
        const GLuint V  = (GLuint) src[2] << 16;
        const GLuint Y1 = (GLuint) src[3];

        dst[0] = 0xFF000000 | Y0 | U | V;
        dst[1] = 0xFF000000 | Y1 | U | V;

        src -= 4;
        dst -= 2;
    }
}

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
/* Empty image                                                               */

int send_create_image_nil(int w, int h, int b)
{
    int i;

    struct image *p;

    if ((i = new_image()) >= 0)
    {
        p = get_image(i);

        p->state = 0;
        p->count = 1;
        p->flags = (NPOT(w) || NPOT(h)) ? FLAG_NPOT : 0;
        p->type  = TYPE_MAP;
        p->w     = w;
        p->h     = h;
        p->b     = b;

        p->nfo.map.name = NULL;
        p->nfo.map.data = malloc(p->w * p->h * p->b);

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
    {
        glCompressedTexImage2DARB(m, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                  w, h, 0, w * h / 2, nfo->data);
        glTexParameteri(m, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(m, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        if (m == GL_TEXTURE_2D)
        {
            gluBuild2DMipmaps(m, f, w, h, f, GL_UNSIGNED_BYTE, nfo->data);
            glTexParameteri(m, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(m, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            glTexImage2D(m, 0, f, w, h, 0, f, GL_UNSIGNED_BYTE, nfo->data);
            glTexParameteri(m, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(m, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

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
        p->nfo.ani.count_c = ca - 1;
        p->nfo.ani.count_p = 1;

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

    if (p->nfo.ani.count_c-- == 0)
    {
        /* Build the file name from the name format and frame number. */

        sprintf(filename, p->nfo.ani.name, p->nfo.ani.frame_i);

        if ((fp = fopen(filename, "rb")))
        {
            size_t sz;

            /* Load compressed or raw pixel data. */

            if ((p->flags & FLAG_COMP) && GL_has_texture_compression)
                sz = p->w * p->h / 2;
            else
                sz = p->w * p->h * p->b;
             
            if (fread(p->nfo.ani.data, 1, sz, fp) < sz)
                error("'%s' load trucated", filename);

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

        /* Update the pulldown counter. */

        if (p->nfo.ani.count_p == 0)
            p->nfo.ani.count_c = p->nfo.ani.count_a - 1;
        else
            p->nfo.ani.count_c = p->nfo.ani.count_b - 1;

        p->nfo.ani.count_p = 1 - p->nfo.ani.count_p;
    }
}

/*---------------------------------------------------------------------------*/

int send_create_image_udp(int port)
{
    int i, n = 8 * 1024 * 1024;
    SOCKET s = INVALID_SOCKET;

    if ((i = new_image()) >= 0)
    {
        struct image *p = get_image(i);

        p->state = 0;
        p->count = 1;
        p->flags = FLAG_NPOT;
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
                    p->nfo.udp.dirty = 0;
                    p->nfo.udp.sock  = s;
                    p->nfo.udp.code  = 0;
                    p->nfo.udp.frag  = 0;
                    p->nfo.udp.prog  = 0;
                    p->nfo.udp.fbo   = 0;

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
    p->flags = FLAG_NPOT;
    p->type  = TYPE_UDP;

    /* Streaming images are configured on data receipt. */

    p->w = 0;
    p->h = 0;
    p->b = 0;

    p->nfo.udp.sock  = INVALID_SOCKET;
    p->nfo.udp.dirty = 0;
    p->nfo.udp.code  = 0;
    p->nfo.udp.frag  = 0;
    p->nfo.udp.prog  = 0;
    p->nfo.udp.fbo   = 0;
}

static char *yuv_text = \
    "uniform samplerRect rgb;                            \n"\

    "void main()                                         \n"\
    "{                                                   \n"\
    "    const vec3 d = vec3(0.0625, 0.5000, 0.5000);    \n"\
    "    const mat3 M = mat3(1.164,  1.164,  1.164,      \n"\
    "                        0.000, -0.813,  1.596,      \n"\
    "                        2.018, -0.391,  0.000);     \n"\

    "    vec3 c = textureRect(rgb, gl_FragCoord.xy).rgb; \n"\

    "    gl_FragColor = vec4(M * (c - d), 1.0);          \n"\
    "}                                                   \n";

static GLuint init_image_udp(struct image_udp *nfo, int w, int h)
{
    GLuint fore = 0;
    GLuint back = 0;

    /* Generate framebuffer and texture objects. */

    if (GL_has_framebuffer_object)
        glGenFramebuffersEXT(1, &nfo->fbo);

    if (GL_has_texture_rectangle)
    {
        /* Initialize the back color buffer. */

        glGenTextures(1, &fore);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, fore);

        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                        GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                        GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        /* Initialize the front color buffer. */

        glGenTextures(1, &back);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, back);

        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                        GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                        GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        nfo->back = back;
    }

    /* Initialize the frame buffer. */

    if (GL_has_framebuffer_object)
    {
        opengl_push_framebuffer(nfo->fbo);
        {
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                      GL_COLOR_ATTACHMENT0_EXT,
                                      GL_TEXTURE_RECTANGLE_ARB, fore, 0);
        }
        opengl_pop_framebuffer();
    }

    /* Initialize a YUV-to-RGB fragment shader. */

    if (GL_has_shader_objects)
    {
        nfo->frag = opengl_shader_object(GL_FRAGMENT_SHADER_ARB, yuv_text);
        nfo->prog = opengl_program_object(0, nfo->frag);
    }

    return fore;
}

static void fini_image_udp(struct image_udp *nfo)
{
    if (nfo->prog)
        glDeleteObjectARB(nfo->prog);

    if (nfo->fbo && GL_has_framebuffer_object)
        glDeleteFramebuffersEXT(1, &nfo->fbo);

    if (nfo->prog)
        glDeleteTextures(1, &nfo->back);
}

static void free_image_udp(struct image_udp *nfo)
{
    if (nfo->sock != INVALID_SOCKET)
        close(nfo->sock);
}

static void draw_image_udp(int i)
{
    struct image *p = get_image(i);

    if ((p->nfo.udp.dirty) &&
        (p->nfo.udp.code == 0x31313459 || p->nfo.udp.code == 0x59565955) &&
        (GL_has_framebuffer_object && GL_has_shader_objects))
    {
        opengl_push_framebuffer(p->nfo.udp.fbo);

        glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
        {
            GLboolean mask[4];

            glGetBooleanv(GL_COLOR_WRITEMASK, mask);
            glColorMask(1, 1, 1, 1);

            glEnable(GL_TEXTURE_RECTANGLE_ARB);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, p->nfo.udp.back);

            glUseProgramObjectARB(p->nfo.udp.prog);
            glUniform1iARB(glGetUniformLocationARB(p->nfo.udp.prog, "rgb"), 0);

            glMatrixMode(GL_PROJECTION);
            {
                glPushMatrix();
                glLoadIdentity();
                glOrtho(0, p->w, 0, p->h, -1, 1);
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glPushMatrix();
                glLoadIdentity();
            }

            glViewport(0, 0, p->w, p->h);
            glScissor (0, 0, p->w, p->h);

            glBegin(GL_POLYGON);
            {
                int x0 = 0, x1 = p->w;
                int y0 = 0, y1 = p->h;

                glTexCoord2i(x0, y0);
                glVertex2i  (x0, y0);
                glTexCoord2i(x1, y0);
                glVertex2i  (x1, y0);
                glTexCoord2i(x1, y1);
                glVertex2i  (x1, y1);
                glTexCoord2i(x0, y1);
                glVertex2i  (x0, y1);
            }
            glEnd();

            glMatrixMode(GL_PROJECTION);
            {
                glPopMatrix();
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glPopMatrix();
            }

            p->nfo.udp.dirty = 0;

            glColorMask(mask[0], mask[1], mask[2], mask[3]);
        }
        glPopAttrib();

        opengl_pop_framebuffer();
    }
}

static void set_image_udp_pixels(int i, GLubyte *data,
                                 int code, int r, int n, int w, int h)
{
    struct image *p = get_image(i);

    /* If the image size or type has changed, reinitialize. */
    
    if (p->nfo.udp.code != code || p->w != w || p->h != h)
    {
        p->nfo.udp.code = code;
        p->w = w;
        p->h = h;

        /* Flush the OpenGL state. */

        fini_image(i);
        init_image(i);
    }

    /* Copy the incoming data to the texture object. */

    if (GL_has_texture_rectangle)
    {
        switch (p->nfo.udp.code)
        {
        case 0x31313459: /* Y411 */
            expand411(data, w * n);
            glBindTexture  (GL_TEXTURE_RECTANGLE_ARB, p->nfo.udp.back);
            glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, r, w, n,
                            GL_RGBA, GL_UNSIGNED_BYTE, data);
            break;

        case 0x59565955: /* UYVY */
            expand422(data, w * n);
            glBindTexture  (GL_TEXTURE_RECTANGLE_ARB, p->nfo.udp.back);
            glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, r, w, n,
                            GL_RGBA, GL_UNSIGNED_BYTE, data);
            break;

        case 0x20424752: /* RGB  */
            glBindTexture  (GL_TEXTURE_RECTANGLE_ARB, p->texture);
            glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, r, w, n,
                            GL_RGB,  GL_UNSIGNED_BYTE, data);
            break;

        case 0x41424752: /* RGBA */
            glBindTexture  (GL_TEXTURE_RECTANGLE_ARB, p->texture);
            glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, r, w, n,
                            GL_RGBA, GL_UNSIGNED_BYTE, data);
            break;
        }

        p->nfo.udp.dirty = 1;
    }
}

static void step_image_udp(int i)
{
    struct image *p = get_image(i);

    struct header *head = (struct header *) buffer;
    GLubyte       *data =       (GLubyte *) buffer + sizeof (struct header);

    int size;

    /* Receive an incoming subimage. */

    if ((size = (int) recv(p->nfo.udp.sock, buffer, BUFMAX, 0)) > 0)
    {
        /* Decode the subimage header. */

        int code = ntohl(head->code);
        int r    = ntohs(head->r);
        int n    = ntohs(head->n);
        int w    = ntohs(head->w);
        int h    = ntohs(head->h);

        /* Distribute the new pixel data. */

        send_set_image_pixels(i, data, code, r, n, w, h, 0);
    }
}

/*---------------------------------------------------------------------------*/

static int buffer_size(int code, int w, int h)
{
    switch (code)
    {
    case 0x31313459: return w * h * 12 / 8; /* Y411 */
    case 0x59565955: return w * h * 16 / 8; /* UYVY */
    case 0x20424752: return w * h * 24 / 8; /* RGB  */
    case 0x41424752: return w * h * 32 / 8; /* RGBA */
    }
    return 0;
}

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

    if ((get_image(i)->flags & FLAG_COMP) && GL_has_texture_compression)
        send_array(data, MIN(w * n / 2, BUFMAX), 1);
    else
        send_array(data, MIN(buffer_size(code, w, n), BUFMAX), 1);

    switch (get_image(i)->type)
    {
    case TYPE_ANI: set_image_ani_pixels(i, data, code, r, n, w, h, b); break;
    case TYPE_UDP: set_image_udp_pixels(i, data, code, r, n, w, h);    break;
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

    if ((get_image(i)->flags & FLAG_COMP) && GL_has_texture_compression)
        recv_array(buffer, MIN(w * n / 2, BUFMAX), 1);
    else
        recv_array(buffer, MIN(buffer_size(code, w, n), BUFMAX), 1);

    switch (get_image(i)->type)
    {
    case TYPE_ANI: set_image_ani_pixels(i, buffer, code, r, n, w, h, b); break;
    case TYPE_UDP: set_image_udp_pixels(i, buffer, code, r, n, w, h);    break;
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

GLenum get_image_target(int i)
{
    if (NPOT(get_image_w(i)) || NPOT(get_image_h(i)))
        return GL_TEXTURE_RECTANGLE_ARB;
    else
        return GL_TEXTURE_2D;
}

GLuint get_image_buffer(int i)
{
    if (i > 0)
        return get_image(i)->texture;
    else
        return 0;
}

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

int get_image_i(int i)
{
    struct image *p = get_image(i);

    if (p->type == TYPE_ANI)
        return p->nfo.ani.frame_i;
    else
        return 0;
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
            p->texture = init_image_udp(&p->nfo.udp, p->w, p->h);     break;
        }
    }
}

void fini_image(int i)
{
    struct image *p = get_image(i);

    if (p->state == 1)
    {
        /* Invoke any type-specific finalizer. */

        switch (p->type)
        {
        case TYPE_UDP: fini_image_udp(&p->nfo.udp); break;
        }

        /* Release the texture object. */

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

            /* Invoke any type-specific draw handler. */

            switch (p->type)
            {
            case TYPE_UDP: draw_image_udp(i); break;
            }

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

    int c = 1024, s, i, m = 0, n = vecnum(image);

    fd_set fds0;
    fd_set fds1;

    /* Initialize a descriptor set including all video sockets. */

    FD_ZERO(&fds0);

    for (i = 0; i < n; ++i)
    {
        struct image *p = get_image(i);

        if (p->count && p->type == TYPE_UDP && p->nfo.udp.sock >= 0)
        {
            FD_SET(p->nfo.udp.sock, &fds0);

            if (m < (int) p->nfo.udp.sock + 1)
                m = (int) p->nfo.udp.sock + 1;
        }
    }

    memcpy(&fds1, &fds0, sizeof (fd_set));

    /* Handle all video socket activity. */

    if (m > 0)
        while ((c-- > 0) && (s = select(m, &fds1, NULL, NULL, &zero)) > 0)
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
