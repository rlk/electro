#include <stdlib.h>
#include <stdio.h>
#include <png.h>

#include "opengl.h"
#include "png.h"

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#define FMODE_RB "rb"
#else
#define FMODE_RB "r"
#endif

/*---------------------------------------------------------------------------*/

GLuint png_punt(const char *message)
{
    fprintf(stderr, "Error: %s\n", message);
    return 0;
}

GLuint png_load(const char *filename)
{
    FILE *fp = NULL;
    GLuint o = 0;

    png_structp readp = NULL;
    png_infop   infop = NULL;
    png_bytep  *bytep = NULL;

    /* Initialize all PNG import data structures. */

    if (!(fp = fopen(filename, FMODE_RB)))
        return png_punt("Failure opening PNG file");

    if (!(readp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        return png_punt("Failure creating PNG read struct");
        
    if (!(infop = png_create_info_struct(readp)))
        return png_punt("Failure creating PNG info struct");

    /* Enable the default PNG error handler. */

    if (setjmp(png_jmpbuf(readp)) == 0)
    {
        GLubyte *p;
        int b, f;
        int w, c;
        int h, r;
        int i;

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
        case PNG_COLOR_TYPE_RGB:       b = 3; f = GL_RGB;  break;
        case PNG_COLOR_TYPE_RGB_ALPHA: b = 4; f = GL_RGBA; break;

        default: return png_punt("Unsupported PNG color type");
        }

        /* Read the pixel data. */

        if (!(bytep = png_get_rows(readp, infop)))
            return png_punt("Failure reading PNG pixel data");

        /* Allocate the final pixel buffer and copy pixels there. */

        p = (GLubyte *) malloc(w * h * b);

        for (r = 0; r < h; r++)
            for (c = 0; c < w; c++)
                for (i = 0; i < b; i++)
                    p[r*w*b + c*b + i] = (GLubyte) bytep[h - r - 1][c*b + i];
        
        /* Create a GL texture object. */

        glGenTextures(1, &o);
        glBindTexture(GL_TEXTURE_2D, o);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        gluBuild2DMipmaps(GL_TEXTURE_2D, f, w, h, f, GL_UNSIGNED_BYTE, p);

        free(p);
    }
    else return png_punt("PNG read error");

    /* Free all resources. */

    png_destroy_read_struct(&readp, &infop, NULL);

    fclose(fp);

    return o;
}

/*---------------------------------------------------------------------------*/
