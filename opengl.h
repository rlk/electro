/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    TOTAL PERSPECTIVE VORTEX is free software;  you can redistribute it    */
/*    and/or modify it under the terms of the  GNU General Public License    */
/*    as published by the  Free Software Foundation;  either version 2 of    */
/*    the License, or (at your option) any later version.                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#ifndef OPENGL_H
#define OPENGL_H

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
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

#include "glext.h"

#ifdef _WIN32
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArrayARB;
extern PFNGLPROGRAMENVPARAMETER4FARBPROC    glProgramEnvParameter4fARB;
extern PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointerARB;
extern PFNGLPROGRAMSTRINGARBPROC            glProgramStringARB;
#endif

/*---------------------------------------------------------------------------*/

#define PI 3.1415926535897932384626433832795

/*---------------------------------------------------------------------------*/

GLboolean   opengl_need(const char *);
GLboolean   opengl_init(void);
GLint       opengl_perf(void);
const char *opengl_read(const char *);

/*---------------------------------------------------------------------------*/

#endif
