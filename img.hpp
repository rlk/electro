#ifndef IMG_HPP
#define IMG_HPP

#include <string>
#include <map>

#include "opengl.hpp"

//-----------------------------------------------------------------------------

class img
{
protected:

    char *p;
    int   w;
    int   h;
    int   b;

    GLuint o;

public:

    void init();
    void draw();

    virtual ~img();
};

//-----------------------------------------------------------------------------

#endif
