#ifndef OPENGL_H
#define OPENGL_H

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

/*---------------------------------------------------------------------------*/

#endif
