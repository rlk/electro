#ifndef PNG_H
#define PNG_H

#include "opengl.h"

/*---------------------------------------------------------------------------*/

GLuint   image_make_tex(const GLubyte *, GLsizei, GLsizei, GLsizei);
GLubyte *image_load_png(const char *, GLsizei *, GLsizei *, GLsizei *);

/*---------------------------------------------------------------------------*/

#endif
