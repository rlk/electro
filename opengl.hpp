#ifndef OPENGL_HPP
#define OPENGL_HPP

//-----------------------------------------------------------------------------

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Ha.
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

//-----------------------------------------------------------------------------

#endif
