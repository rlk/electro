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

#define MAX_FILE 6

struct image
{
    int count;
    int state;

    GLuint texture;

    /* Pixel buffer attributes. */

    int   n;
    int   w;
    int   h;
    int   b;
    void *p[MAX_FILE];
    char *s[MAX_FILE];

    /* Video socket attributes. */

    int sock;
    int code;
};

static vector_t image;

static GLenum format[5] = {
    0,
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA,
    GL_RGB,
    GL_RGBA,
};

/*---------------------------------------------------------------------------*/

#define BUFMAX 65536

struct header
{
    int   code;
    short x, y;
    short w, h;
    short W, H;
};

static void *buffer = NULL;

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

#ifdef OBSOLETE

static void copy_yuv411(unsigned char *dst, unsigned char *src, int w, int h)
{
    int i, n = w * h;

    if (0) /*(GL_has_fragment_program) */
    {
        unsigned int *pix = (unsigned int *) dst;

        /* Extract raw YUV and allow RGB conversion via fragment program. */

        for (i = 0; i < n; i += 4)
        {
            const int UV = 0xFF000000 | (src[0] << 8) | (src[3] << 16);

            pix[0] = src[1] | UV;
            pix[1] = src[2] | UV;
            pix[2] = src[4] | UV;
            pix[3] = src[5] | UV;

            src += 6;
            pix += 4;
        }
    }
    else
    {
        /* Extract RGBA color from YUV */

        for (i = 0; i < n; i += 4)
        {
            const int U  = src[0];
            const int Y0 = src[1];
            const int Y1 = src[2];
            const int V  = src[3];
            const int Y2 = src[4];
            const int Y3 = src[5];

            yuv2rgb(dst +  0, Y0, U, V);
            yuv2rgb(dst +  4, Y1, U, V);
            yuv2rgb(dst +  8, Y2, U, V);
            yuv2rgb(dst + 12, Y3, U, V);

            src +=  6;
            dst += 16;
        }
    }
}

static void copy_stereo(unsigned char *dst, unsigned char *src, int w, int h)
{
    int i, n = w * h;

    const unsigned char *srcL = src;
    const unsigned char *srcR = src + 1;

    unsigned int *dstL = (unsigned int *) dst;
    unsigned int *dstR = (unsigned int *) dst + n / 2;

    for (i = 0; i < n; i += 2)
    {
        *dstL = 0xFF000000 | *srcL | (*srcL << 8) | (*srcL << 16);
        *dstR = 0xFF000000 | *srcR | (*srcR << 8) | (*srcR << 16);

        dstL += 1;
        dstR += 1;
        srcL += 2;
        srcR += 2;
    }
}

static void decode_bayer(unsigned char *buf, unsigned int w, unsigned int h)
{
    unsigned int r;
    unsigned int c;

    for (r = 0; r < h; r += 2)
    {
        unsigned int *p00 = (unsigned int *) buf +  r    * w, *p01 = p00 + 1;
        unsigned int *p10 = (unsigned int *) buf + (r+1) * w, *p11 = p10 + 1;

        for (c = 0; c < w; c += 2)
        {
            const unsigned int pr = *p00 & 0xFF0000FF;
            const unsigned int pg = *p01 & 0xFF00FF00;
            const unsigned int qg = *p10 & 0xFF00FF00;
            const unsigned int pb = *p11 & 0xFFFF0000;

            *p00 = pr | pg | pb;
            *p01 = pr | pg | pb;
            *p10 = pr | qg | pb;
            *p11 = pr | qg | pb;

            p00 += 2;
            p01 += 2;
            p10 += 2;
            p11 += 2;
        }
    }
}

static void step_texture(int i)
{
    struct image *p = get_image(i);
    GLenum t;

    if (p->frame)
    {
        unsigned char *dst = (unsigned char *) p->p[0];
        unsigned char *src = (unsigned char *) p->frame;

        if (p->bits == 12)
            copy_yuv411(dst, src, p->w, p->h);
        if (p->bits ==  8)
        {
            copy_stereo(dst, src, p->w, p->h);
            decode_bayer(dst, p->w, p->h);
        }
    }

    if (p->texture)
    {
        if (GL_has_texture_rectangle && (NPOT(p->w) || NPOT(p->h)))
            t = GL_TEXTURE_RECTANGLE_ARB;
        else
            t = GL_TEXTURE_2D;

        glBindTexture(t, p->texture);
        glTexSubImage2D(t, 0, 0, 0, p->w, p->h,
                        format[p->b], GL_UNSIGNED_BYTE, p->p[0]);
    }
}

#endif /* OBSOLETE */

/*===========================================================================*/

static void yuv2rgb(GLubyte c[3], GLubyte Y,
                                  GLubyte U,
                                  GLubyte V)
{
    int R = (int) (1.164 * (Y - 16) + 1.596 * (V - 128));
    int G = (int) (1.164 * (Y - 16) - 0.813 * (V - 128) - 0.391 * (U - 128));
    int B = (int) (1.164 * (Y - 16)                     + 2.018 * (U - 128));

    if      (R <   0) c[0] = 0x00;
    else if (R > 255) c[0] = 0xFF;
    else              c[0] = (unsigned char) R;

    if      (G <   0) c[1] = 0x00;
    else if (G > 255) c[1] = 0xFF;
    else              c[1] = (unsigned char) G;

    if      (B <   0) c[2] = 0x00;
    else if (B > 255) c[2] = 0xFF;
    else              c[2] = (unsigned char) B;
}

static void decode_Y411(GLubyte *p, int w, int h)
{
    GLubyte *dst = p + (w * h - 1) * 12;
    GLubyte *src = p + (w * h - 1) *  6;

    while (src >= p && dst >= p)
    {
        const int U  = src[0];
        const int Y0 = src[1];
        const int Y1 = src[2];
        const int V  = src[3];
        const int Y2 = src[4];
        const int Y3 = src[5];

        yuv2rgb(dst + 0, Y0, U, V);
        yuv2rgb(dst + 3, Y1, U, V);
        yuv2rgb(dst + 6, Y2, U, V);
        yuv2rgb(dst + 9, Y3, U, V);

        src -=  6;
        dst -= 12;
    }
}

/*---------------------------------------------------------------------------*/

static void step_video(int i)
{
    struct image *p = get_image(i);
    ssize_t n;

    struct header *head = (struct header *) buffer;
    GLubyte       *data =       (GLubyte *) buffer + sizeof (struct header);

    /* Receive an incoming subimage. */

    if ((n = recv(p->sock, buffer, BUFMAX, 0)) > 0)
    {
        /* Decode the subimage header. */

        int code = ntohl(head->code);
        int x    = ntohs(head->x);
        int y    = ntohs(head->y);
        int w    = ntohs(head->w);
        int h    = ntohs(head->h);
        int W    = ntohs(head->W);
        int H    = ntohs(head->H);

        /* If the video format changes, refresh the texture object. */

        if (p->w != W || p->h != H || p->code != code)
        {
            p->code = code;
            p->w    = W;
            p->h    = H;

            switch (code)
            {
            case 0x31313459: p->b = 3; break;  /* Y411 */
            case 0x30303859: p->b = 1; break;  /* Y800 */
            }

            fini_image(i);
            init_image(i);
        }

        /* Decode the incoming image data as necessary. */

        switch (code)
        {
        case 0x31313459: decode_Y411(data, w, h); break;
        }

        /* Apply the incoming subimage to the existing texture object. */

        if (p->texture)
        {
            if (GL_has_texture_rectangle && (NPOT(w) || NPOT(h)))
            {
                glBindTexture  (GL_TEXTURE_RECTANGLE_ARB, p->texture);
                glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, x, y, w, h,
                                format[p->b], GL_UNSIGNED_BYTE, data);
            }
            else
            {
                glBindTexture  (GL_TEXTURE_2D, p->texture);
                glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
                                format[p->b], GL_UNSIGNED_BYTE, data);
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

static GLuint make_video(int w, int h, int b)
{
    GLuint o = 0;

    if (w * h * b > 0)
    {
        /* Create a GL texture object. */

        glGenTextures(1, &o);

        /* Non-power-of-two size implies texture rectangle. */

        if (GL_has_texture_rectangle && (NPOT(w) || NPOT(h)))
        {
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, o);

            glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, format[b],
                         w, h, 0, format[b], GL_UNSIGNED_BYTE, NULL);

            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                            GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                            GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        /* A power-of-two image comprises a 2D texture map. */

        else
        {
            glBindTexture(GL_TEXTURE_2D, o);

            glTexImage2D(GL_TEXTURE_2D, 0, format[b],
                         w, h, 0, format[b], GL_UNSIGNED_BYTE, NULL);

            glTexParameteri(GL_TEXTURE_2D,
                            GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,
                            GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
    }
    return o;
}

static GLuint make_image(void *p[6], int n, int w, int h, int b)
{
    GLuint o = 0;

    /* Create a GL texture object. */

    glGenTextures(1, &o);

    /* Several images comprise a cube map. */

    if (n > 1)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, o);

        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, format[b],
                          w, h, format[b], GL_UNSIGNED_BYTE, p[0]);
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, format[b],
                          w, h, format[b], GL_UNSIGNED_BYTE, p[1]);
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, format[b],
                          w, h, format[b], GL_UNSIGNED_BYTE, p[2]);
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, format[b],
                          w, h, format[b], GL_UNSIGNED_BYTE, p[3]);
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, format[b],
                          w, h, format[b], GL_UNSIGNED_BYTE, p[4]);
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, format[b],
                          w, h, format[b], GL_UNSIGNED_BYTE, p[5]);

        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                        GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                        GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                        GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                        GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                        GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    /* Non-power of two size implies texture rectangle. */

    else if (GL_has_texture_rectangle && (NPOT(w) || NPOT(h)))
    {
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, o);

        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, format[b],
                     w, h, 0, format[b], GL_UNSIGNED_BYTE, p[0]);

        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                        GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                        GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    /* A single image comprises a 2D texture map. */

    else
    {
        glBindTexture(GL_TEXTURE_2D, o);

        gluBuild2DMipmaps(GL_TEXTURE_2D, format[b], w, h,
                          format[b], GL_UNSIGNED_BYTE, p[0]);

        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

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
    if (filename)
    {
        const char *ext = filename + strlen(filename) - 4;
    
        if      (strcmp(ext, ".png") == 0 || strcmp(ext, ".PNG") == 0)
            return load_png_image(filename, width, height, bytes);
        else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".JPG") == 0)
            return load_jpg_image(filename, width, height, bytes);
        else
            return error("Unsupported image format for '%s'", filename);
    }
    return NULL;
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

int send_create_image(const char *file_nx,
                      const char *file_px,
                      const char *file_ny,
                      const char *file_py,
                      const char *file_nz,
                      const char *file_pz)
{
    int i, j, n = vecnum(image);

    /* Return the default texture on NULL. */

    if (file_nx == NULL)
        return 0;

    /* Scan the current images for an existing instance of the named files. */

    for (i = 0; i < n; ++i)
    {
        struct image *p = get_image(i);

        if (cmpname(file_nx, p->s[0]) == 0 &&
            cmpname(file_px, p->s[1]) == 0 &&
            cmpname(file_ny, p->s[2]) == 0 &&
            cmpname(file_py, p->s[3]) == 0 &&
            cmpname(file_nz, p->s[4]) == 0 &&
            cmpname(file_pz, p->s[5]) == 0)
            return dupe_create_image(i);
    }

    /* Didn't find it.  It's new. */

    if ((i = new_image()) >= 0)
    {
        struct image *p = get_image(i);

        p->state =  0;
        p->count =  1;
        p->sock  = -1;

        /* Note the file names. */

        p->s[0] = file_nx ? memdup(file_nx, strlen(file_nx) + 1, 1) : NULL;
        p->s[1] = file_px ? memdup(file_px, strlen(file_px) + 1, 1) : NULL;
        p->s[2] = file_ny ? memdup(file_ny, strlen(file_ny) + 1, 1) : NULL;
        p->s[3] = file_py ? memdup(file_py, strlen(file_py) + 1, 1) : NULL;
        p->s[4] = file_nz ? memdup(file_nz, strlen(file_nz) + 1, 1) : NULL;
        p->s[5] = file_pz ? memdup(file_pz, strlen(file_pz) + 1, 1) : NULL;

        /* Load and pack the images. */

        if ((p->p[0] = load_image(file_nx, &p->w, &p->h, &p->b))) p->n++;
        if ((p->p[1] = load_image(file_px, &p->w, &p->h, &p->b))) p->n++;
        if ((p->p[2] = load_image(file_ny, &p->w, &p->h, &p->b))) p->n++;
        if ((p->p[3] = load_image(file_py, &p->w, &p->h, &p->b))) p->n++;
        if ((p->p[4] = load_image(file_nz, &p->w, &p->h, &p->b))) p->n++;
        if ((p->p[5] = load_image(file_pz, &p->w, &p->h, &p->b))) p->n++;

        send_event(EVENT_CREATE_IMAGE);
        send_index(p->n);
        send_index(p->w);
        send_index(p->h);
        send_index(p->b);

        /* If any file is missing, this might reorder cubemap sides.  Meh. */

        for (j = 0; j < p->n; j++)
            send_array(p->p[j], p->w * p->h * p->b, 1);

        return i;
    }
    return -1;
}

int send_create_video(int port)
{
    int i, s = -1, n = 1024 * 1024 * 8;

    if ((i = new_image()) >= 0)
    {
        struct image *p = get_image(i);

        p->state =   0;
        p->count =   1;
        p->w     = 128;
        p->h     = 128;
        p->n     =   1;
        p->b     =   3;
        p->sock  =  -1;

        /* Open a UDP socket for receiving. */

        if ((s = socket(PF_INET, SOCK_DGRAM, 0)) >= 0)
        {
            /* Increase the receive buffer size to handle a gigabit stream. */

            if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &n, sizeof (int)) >= 0)
            {
                sockaddr_t addr;
                size_t     addr_len = sizeof (sockaddr_t);

                /* Accept connections from any address on the given port. */

                addr.sin_family      = AF_INET;
                addr.sin_port        = htons(port);
                addr.sin_addr.s_addr = INADDR_ANY;

                /* Bind the socket to this address. */

                if (bind(s, (struct sockaddr *) &addr, addr_len) >= 0)
                {
                    p->sock = s;
                    return i;
                }
                else error("bind %d : %s", p, system_error());
            }
            else error("setsockopt : %s", system_error());
        }
        else error("socket : %s", system_error());
    }
    return -1;
}

void recv_create_image(void)
{
    struct image *p = get_image(new_image());
    int j;

    p->count = 1;

    p->n = recv_index();
    p->w = recv_index();
    p->h = recv_index();
    p->b = recv_index();

    for (j = 0; j < p->n; ++j)
    {
        p->p[j] =    malloc(p->w * p->h * p->b);
        recv_array(p->p[j], p->w * p->h * p->b, 1);
    }
}

void recv_set_image_pixels(void)
{
/* TODO: fix
    int i           = recv_index();
    struct image *p = get_image(i);

    recv_array(p->p[0], p->w * p->h * p->b, 1);
    step_texture(i);
*/
}

/*---------------------------------------------------------------------------*/

static int free_image(int i)
{
    struct image *p = get_image(i);
    int j;

    if (i > 0)
    {
        if (p->count > 0)
        {
            p->count--;

            if (p->count == 0)
            {
                fini_image(i);

                for (j = 0; j < MAX_FILE; ++j)
                {
                    if (p->s[j]) free(p->s[j]);
                    if (p->p[j]) free(p->p[j]);
                }

                if (p->sock >= 0) close(p->sock);

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
    struct image  *p      = get_image(i);
    unsigned char *pixels = (unsigned char *) p->p[0];

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
            c[0] = pixels[(p->w * y + x) * 4 + 0];
            c[1] = pixels[(p->w * y + x) * 4 + 1];
            c[2] = pixels[(p->w * y + x) * 4 + 2];
            c[3] = pixels[(p->w * y + x) * 4 + 3];
            break;
        }
    else c[0] = c[1] = c[2] = c[3] = 0xFF;
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
        if (p->sock >= 0)
            p->texture = make_video(p->w, p->h, p->b);
        else
            p->texture = make_image(p->p, p->n, p->w, p->h, p->b);
        
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
    struct image *p = get_image(i);

    if (p->count)
    {
        init_image(i);

        if (p->texture)
        {
            GLenum t = GL_TEXTURE_2D;

            /* Ensure the wrong texture unit types are disabled. */

            glDisable(GL_TEXTURE_RECTANGLE_ARB);
            glDisable(GL_TEXTURE_CUBE_MAP_ARB);
            glDisable(GL_TEXTURE_2D);

            /* Determine the right texture unit type and enable it. */

            if      (p->n > 1)                 t = GL_TEXTURE_CUBE_MAP_ARB;
            else if (NPOT(p->w) || NPOT(p->h)) t = GL_TEXTURE_RECTANGLE_ARB;
            
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

    int i, m = 0, n = vecnum(image), c;

    fd_set fds0;
    fd_set fds1;

    /* Initialize a descriptor set including all video sockets. */

    FD_ZERO(&fds0);

    for (i = 0; i < n; ++i)
    {
        struct image *p = get_image(i);

        if (p->count && p->sock >= 0)
        {
            FD_SET(p->sock, &fds0);

            if (m < p->sock + 1)
                m = p->sock + 1;
        }
    }

    memcpy(&fds1, &fds0, sizeof (fd_set));

    /* Handle all video socket activity. */

    if (m > 0)
        while ((c = select(m, &fds1, NULL, NULL, &zero)) > 0)
        {
            for (i = 0; i < n; ++i)
            {
                struct image *p = get_image(i);

                if (p->count && p->sock >= 0 && FD_ISSET(p->sock, &fds1))
                    step_video(i);
            }
            memcpy(&fds1, &fds0, sizeof (fd_set));
        }
}

/*---------------------------------------------------------------------------*/

int startup_image(void)
{
    int i;

    if ((image = vecnew(256, sizeof (struct image))))
    {
        if ((buffer = malloc(BUFMAX)))
        {
            if ((i = new_image()) >= 0)
            {
                struct image *p = get_image(i);

                p->count   =   1;
                p->state   =   0;
                p->sock    =  -1;
                p->texture =   0;
                p->n       =   1;
                p->w       = 128;
                p->h       = 128;
                p->b       =   4;
                p->p[0]    =   malloc(128 * 128 * 4);

                memset(p->p[0], 0xFF, 128 * 128 * 4);

                return 1;
            }
        }
    }
    return 0;
}
