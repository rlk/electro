#include <iostream>
#include <stdexcept>
#include <png.h>

#include "png.hpp"

//-----------------------------------------------------------------------------

#ifdef _WIN32
#define FMODE_RB "rb"
#else
#define FMODE_RB "r"
#endif

//-----------------------------------------------------------------------------

png::png(std::string filename)
{
    FILE *fp;

    png_structp readp;
    png_infop   infop;
    png_bytep  *bytep;

    // Initialize all PNG import data structures.

    if (!(fp = fopen(filename.c_str(), FMODE_RB)))
        throw std::runtime_error("png::png failure opening file");

    if (!(readp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        throw std::runtime_error("png::png failure creating read struct");
        
    if (!(infop = png_create_info_struct(readp)))
        throw std::runtime_error("png::png failure creating info struct");

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(readp)) == 0)
    {
        // Read the PNG header.

        png_init_io(readp, fp);
        png_read_png(readp, infop,
                     PNG_TRANSFORM_STRIP_16 |
                     PNG_TRANSFORM_PACKING, NULL);
        
        // Extract and check image properties.

        w = int(png_get_image_width (readp, infop));
        h = int(png_get_image_height(readp, infop));

        switch (png_get_color_type(readp, infop))
        {
        case PNG_COLOR_TYPE_RGB:       b = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA: b = 4; break;

        default: throw std::runtime_error("png::png unsupported color type");
        }

        // Read the pixel data.

        if (!(bytep = png_get_rows(readp, infop)))
            throw std::runtime_error("png::png failure reading pixel data");

        // Allocate the final pixel buffer and copy pixels there.

        p = new char[w * h * b];

        for (int r = 0; r < h; r++)
            for (int c = 0; c < w; c++)
                for (int i = 0; i < b; i++)
                    p[r*w*b + c*b + i] = char(bytep[h - r - 1][c*b + i]);
    }
    else throw std::runtime_error("png::png read error");

    // TODO destroy readp, infop?

    fclose(fp);
}

//-----------------------------------------------------------------------------
