#ifndef OPENGL_H
#define OPENGL_H

/*---------------------------------------------------------------------------*/

#define WINDOW

#ifdef WINDOW
#define WIN_W 1024
#define WIN_H 768
#define WIN_M 0
#else
#define WIN_W 1600
#define WIN_H 1200
#define WIN_M SDL_FULLSCREEN
#endif

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#define WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glext.h"

/*---------------------------------------------------------------------------*/

GLboolean   gl_supported(const char *);
const char *gl_read_text(const char *);

/*---------------------------------------------------------------------------*/

#endif
