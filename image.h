#ifndef IMAGE_H
#define IMAGE_H

#include "opengl.h"

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#define FMODE_RB "rb"
#define FMODE_WB "wb"
#else
#define FMODE_RB "r"
#define FMODE_WB "w"
#endif

/*---------------------------------------------------------------------------*/

GLuint   image_make_tex(const GLubyte *, GLsizei, GLsizei, GLsizei);
GLubyte *image_load_png(const char *, GLsizei *, GLsizei *, GLsizei *);

/*---------------------------------------------------------------------------*/

#endif
