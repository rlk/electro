#ifndef OPENGL_H
#define OPENGL_H

/*---------------------------------------------------------------------------*/

#ifdef WIDE
#define WIN_W 1024
#define WIN_H 460
#else
#define WIN_W 1024
#define WIN_H 768
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

#endif
