#include <iostream>
#include <stdexcept>

#include "img.hpp"

//-----------------------------------------------------------------------------

void img::init()
{
    // Create a GL texture object.

    glGenTextures(1, &o);
    glBindTexture(GL_TEXTURE_2D, o);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (b == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, p);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  w, h, 0,
                     GL_RGB,  GL_UNSIGNED_BYTE, p);
}

void img::draw()
{
    if (o == 0)
        init();
    else
        glBindTexture(GL_TEXTURE_2D, o);
}

img::~img()
{
    if (p) delete [] p;
}

//-----------------------------------------------------------------------------
