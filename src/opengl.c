/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    ELECTRO is free software;  you can redistribute it and/or modify it    */
/*    under the terms of the  GNU General Public License  as published by    */
/*    the  Free Software Foundation;  either version 2 of the License, or    */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "opengl.h"
#include "utility.h"
#include "console.h"

#ifdef __linux__
#include <GL/glx.h>
#endif

/*---------------------------------------------------------------------------*/

GLboolean GL_has_fragment_program     = 0;
GLboolean GL_has_vertex_program       = 0;
GLboolean GL_has_vertex_buffer_object = 0;
GLboolean GL_has_point_sprite         = 0;
GLboolean GL_has_texture_rectangle    = 0;
GLboolean GL_has_texture_compression  = 0;
GLboolean GL_has_multitexture         = 0;
GLenum    GL_max_multitexture         = 0;

/*---------------------------------------------------------------------------*/

/* Confirm that the named OpenGL extension is supported by the current       */
/* implementation.                                                           */

GLboolean opengl_need(const char *extension)
{
    const char *string = (const char *) glGetString(GL_EXTENSIONS);
    const char *start  = string;

    char *where;
    char *space;

    while (1)
    {
        if ((where = strstr(start, extension)) == NULL)
            return 0;

        space = where + strlen(extension);

        if (where == start || *(where - 1) == ' ')
            if (*space == ' ' || *space == '\0')
                return GL_TRUE;

        start = space;
    }

    return GL_FALSE;
}

/* Acquire a pointer to the named OpenGL function.  Print an error if this   */
/* function is not supported by the current implementation.  Note that SDL's */
/* SDL_GL_GetProcAddress function doesn't work correctly on older Linux      */
/* systems, so we call GLX directly.                                         */

void *opengl_proc(const char *name)
{
#ifdef __linux__
    void *p = glXGetProcAddressARB(name);
#else
    void *p = SDL_GL_GetProcAddress(name);
#endif

    if (p == NULL)
        error("OpenGL procedure '%s' not found", name);

    return p;
}

/*---------------------------------------------------------------------------*/

#ifdef __APPLE__

void init_opengl(void)
{
    GLint TUs;

    GL_has_fragment_program     = opengl_need("GL_ARB_fragment_program");
    GL_has_vertex_program       = opengl_need("GL_ARB_vertex_program");
    GL_has_vertex_buffer_object = opengl_need("GL_ARB_vertex_buffer_object");
    GL_has_point_sprite         = opengl_need("GL_ARB_point_sprite");
    GL_has_texture_compression  = opengl_need("GL_ARB_texture_compression");
    GL_has_texture_rectangle    = opengl_need("GL_ARB_texture_rectangle");
    GL_has_multitexture         = opengl_need("GL_ARB_multitexture");

    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &TUs);
    GL_max_multitexture = GL_TEXTURE0_ARB +  TUs;
}

#else

PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArrayARB;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB;
PFNGLPROGRAMENVPARAMETER4FARBPROC    glProgramEnvParameter4fARB;
PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointerARB;
PFNGLPROGRAMSTRINGARBPROC            glProgramStringARB;
PFNGLBINDPROGRAMARBPROC              glBindProgramARB;
PFNGLGENPROGRAMSARBPROC              glGenProgramsARB;
PFNGLISPROGRAMARBPROC                glIsProgramARB;
PFNGLDELETEPROGRAMSARBPROC           glDeleteProgramsARB;
PFNGLBINDBUFFERARBPROC               glBindBufferARB;
PFNGLGENBUFFERSARBPROC               glGenBuffersARB;
PFNGLBUFFERDATAARBPROC               glBufferDataARB;
PFNGLISBUFFERARBPROC                 glIsBufferARB;
PFNGLDELETEBUFFERSARBPROC            glDeleteBuffersARB;
PFNGLACTIVETEXTUREARBPROC            glActiveTextureARB;

void init_opengl(void)
{
   glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)
        opengl_proc("glProgramStringARB");
    glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)
        opengl_proc("glBindProgramARB");
    glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)
        opengl_proc("glGenProgramsARB");
    glIsProgramARB = (PFNGLISPROGRAMARBPROC)
        opengl_proc("glIsProgramARB");
    glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)
        opengl_proc("glDeleteProgramsARB");
    glProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)
        opengl_proc("glProgramLocalParameter4fvARB");
    glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)
        opengl_proc("glProgramEnvParameter4fARB");

    if (opengl_need("GL_ARB_fragment_program"))
    {
        GL_has_fragment_program = (glProgramStringARB
                                && glBindProgramARB
                                && glGenProgramsARB);
    }

    if (opengl_need("GL_ARB_vertex_program"))
    {
        glDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)
            opengl_proc("glDisableVertexAttribArrayARB");
        glEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)
            opengl_proc("glEnableVertexAttribArrayARB");
        glVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC)
            opengl_proc("glVertexAttribPointerARB");

        GL_has_vertex_program = (glProgramEnvParameter4fARB
                              && glDisableVertexAttribArrayARB
                              && glEnableVertexAttribArrayARB
                              && glVertexAttribPointerARB
                              && glProgramStringARB
                              && glBindProgramARB
                              && glGenProgramsARB);
    }

    if (opengl_need("GL_ARB_vertex_buffer_object"))
    {
        glBindBufferARB = (PFNGLBINDBUFFERARBPROC)
            opengl_proc("glBindBufferARB");
        glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)
            opengl_proc("glGenBuffersARB");
        glBufferDataARB = (PFNGLBUFFERDATAARBPROC)
            opengl_proc("glBufferDataARB");
        glIsBufferARB = (PFNGLISBUFFERARBPROC)
            opengl_proc("glIsBufferARB");
        glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)
            opengl_proc("glDeleteBuffersARB");

        GL_has_vertex_buffer_object = (glBindBufferARB
                                    && glGenBuffersARB
                                    && glBufferDataARB
                                    && glIsBufferARB
                                    && glDeleteBuffersARB);
    }

    if (opengl_need("GL_ARB_multitexture"))
    {
        GLint TUs;

        glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)
            opengl_proc("glActiveTextureARB");

        glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &TUs);
        GL_max_multitexture = GL_TEXTURE0_ARB +  TUs;
        GL_has_multitexture = (glActiveTextureARB && GL_TRUE);
    }

    GL_has_point_sprite
        = opengl_need("GL_ARB_point_sprite");
    GL_has_texture_rectangle
        = opengl_need("GL_ARB_texture_rectangle") | GL_TRUE;
    GL_has_texture_compression
        = opengl_need("GL_ARB_texture_compression");
}

#endif

/*---------------------------------------------------------------------------*/

GLfloat opengl_perf(GLfloat *all)
{
    static GLfloat fps   = 0.0f;
    static GLint   then  = 0;
    static GLint   count = 0;
    static GLint   total = 0;
    static GLint   start = 0;

    GLint now = (GLint) SDL_GetTicks();

    /* Compute the average FPS over 1000 milliseconds. */

    count++;

    if (now - then > 1000)
    {
        fps   = 1000.0f * count / (now - then);
        then  = now;
        count = 0;
    }

    /* Compute the total average FPS. */

    if (start)
        total++;
    else
        start = SDL_GetTicks();

    if (all) *all = 1000.0f * total / (now - start);

    return fps;
}

/*---------------------------------------------------------------------------*/

/* Generate and return a new vertex program object.  Bind the given program  */
/* text to it.  Print any program error message to the console.              */

GLuint opengl_vert_prog(const char *text)
{
    GLuint o;

    glGenProgramsARB(1, &o);
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, o);

    glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB, strlen(text), text);

    if (glGetError() == GL_INVALID_OPERATION)
        error("Vertex program: %s\n",
              glGetString(GL_PROGRAM_ERROR_STRING_ARB));

    return o;
}

/* Generate and return a new fragment program object.  Bind the given        */
/* program text to it.  Print any program error message to the console.      */

GLuint opengl_frag_prog(const char *text)
{
    GLuint o;

    glGenProgramsARB(1, &o);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, o);

    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB, strlen(text), text);

    if (glGetError() == GL_INVALID_OPERATION)
        error("Fragment program: %s\n",
              glGetString(GL_PROGRAM_ERROR_STRING_ARB));

    return o;
}

/*---------------------------------------------------------------------------*/

#ifndef NDEBUG

void opengl_check(const char *format, ...)
{
    GLenum err;

    while ((err = glGetError()) != GL_NO_ERROR)
    {
        char string[MAXSTR];
        va_list args;

        va_start(args, format);
        vsprintf(string, format, args);
        va_end(args);

        error("OpenGL error: %s: %s", gluErrorString(err), string);
    }
}

#endif

/*---------------------------------------------------------------------------*/

