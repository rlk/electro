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

#ifdef EXPERIMENTAL
#include <sys/sem.h>
#include <sys/shm.h>
#endif

#include "opengl.h"
#include "vector.h"
#include "buffer.h"
#include "utility.h"
#include "event.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

#define MAX_FILE 6

struct image
{
    int count;
    int state;

    int frame;
    int shmid;
    int semid;

    GLuint texture;

    int   n;
    int   w;
    int   h;
    int   b;
    void *p[MAX_FILE];
    char *s[MAX_FILE];
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
static int power_of_two(int n)
{
    int i = 1;

    while (i < n)
        i *= 2;

    return i;
}
*/

GLuint make_texture(void *p[6], int n, int w, int h, int b)
{
    GLuint o = 0;

    /* Create a GL texture object. */

    glGenTextures(1, &o);

    if (n > 1)
    {
        /* Several images comprise a cube map. */

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
                        GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                        GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                        GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                        GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
                        GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    else
    {
        /* A single image comprises a 2D texture map. */

        glBindTexture(GL_TEXTURE_2D, o);

        gluBuild2DMipmaps(GL_TEXTURE_2D, format[b], w, h,
                          format[b], GL_UNSIGNED_BYTE, p[0]);

        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
    if (name1 == NULL && name2 == NULL)
        return 0;

    if (name1 == NULL || name2 == NULL)
        return 1;

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
        p->semid = -1;
        p->shmid = -1;

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
            send_array(p->p[j], p->w * p->h, p->b);

        return i;
    }
    return -1;
}

#ifdef EXPERIMENTAL
int send_create_movie(int key, int w, int h, int b)
{
    int i;

    if ((i = new_image()) >= 0)
    {
        struct image *p = get_image(i);

        p->state = 1;
        p->count = 1;
        p->w     = w;
        p->h     = h;
        p->b     = b;

        /* Acquire the semaphore and shared memory buffers.  Pack the image. */

        if ((p->shmid = shmget(key, w * h * b + 4, 0666 | IPC_CREAT)) >=0 )
        {
            if ((p->p = shmat(p->shmid, NULL, SHM_RDONLY)))
            {
                if ((p->semid = semget(key, 1, 0666 | IPC_CREAT)) >= 0)
                {
                    send_event(EVENT_CREATE_IMAGE);
                    send_index(p->w);
                    send_index(p->h);
                    send_index(p->b);

                    send_array(p->p[0], p->w * p->h, p->b);

                    return i;
                }
                else error("Shared image mutex %d: %s", key, system_error());
            }
            else error("Shared image attach %d: %s", key, system_error());
        }
        else error("Shared image buffer %d: %s", key, system_error());
    }
    return -1;
}
#endif

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
        p->p[j] = (GLubyte *) malloc(p->w * p->h * p->b);
        recv_array(p->p[j], p->w * p->h, p->b);
    }
}

void recv_set_image_pixels(void)
{
    struct image *p = get_image(recv_index());

    recv_array(p->p[0], p->w * p->h, p->b);
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

#ifdef EXPERIMENTAL
                if (p->semid >= 0)
                {
                    shmdt(p->p);
                    semctl(p->semid, 0, IPC_RMID, NULL);
                    shmctl(p->shmid,    IPC_RMID, NULL);
                }
                else
#endif
                    for (j = 0; j < MAX_FILE; ++j)
                    {
                        if (p->s[j]) free(p->s[j]);
                        if (p->p[j]) free(p->p[j]);
                    }

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
        p->texture = make_texture(p->p, p->n, p->w, p->h, p->b);
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

        if (p->n > 1)
        {
            glEnable(GL_TEXTURE_CUBE_MAP_ARB);
            glDisable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, get_image(i)->texture);
        }
        else
        {
            glDisable(GL_TEXTURE_CUBE_MAP_ARB);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, get_image(i)->texture);
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

#ifdef EXPERIMENTAL
void step_images(void)
{
    int i, n = vecnum(image);

    for (i = 0; i < n; ++i)
    {
        struct image *p = get_image(i);

        /* If this is a shared image... */

        if (p->count && p->semid >= 0)
        {
            struct sembuf s;

            /* Acquire the buffer. */

            s.sem_num =  0;
            s.sem_op  = -1;
            s.sem_flg =  0;
            semop(p->semid, &s, 1);

            /* If the frame has changed... */

            if (p->frame < *((int *) p->p))
            {
                /* Update the texture object. */

                glBindTexture(GL_TEXTURE_2D, p->texture);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, p->w, p->h,
                                format[p->b], GL_UNSIGNED_BYTE, p->p);

                /* Push the new image to all clients. */

                send_event(EVENT_SET_IMAGE_PIXELS);
                send_index(i);
                send_array(p->p, p->w * p->h, p->b);
            }

            /* Release the buffer. */

            s.sem_num =  0;
            s.sem_op  =  1;
            s.sem_flg =  0;
            semop(p->semid, &s, 1);
        }
    }
}
#endif

/*---------------------------------------------------------------------------*/

int startup_image(void)
{
    int i;

    if ((image = vecnew(256, sizeof (struct image))))
    {
        if ((i = new_image()) >= 0)
        {
            struct image *p = get_image(i);

            p->count    =    1;
            p->state    =    0;
            p->semid    =   -1;
            p->shmid    =   -1;
            p->texture  =    0;
            p->n        =    1;
            p->w        =  128;
            p->h        =  128;
            p->b        =    4;
            p->p[0]     = malloc(128 * 128 * 4);

            memset(p->p[0], 0xFF, 128 * 128 * 4);
        }
        return 1;
    }
    else
        return 0;
}
