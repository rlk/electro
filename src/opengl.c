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

/*---------------------------------------------------------------------------*/

GLboolean GL_has_fragment_program     = 0;
GLboolean GL_has_vertex_program       = 0;
GLboolean GL_has_vertex_buffer_object = 0;
GLboolean GL_has_point_sprite         = 0;

/*---------------------------------------------------------------------------*/

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

    error("Requires OpenGL extension: %s", extension);

    return GL_FALSE;
}

void *opengl_proc(const char *name)
{
    void *p = SDL_GL_GetProcAddress(name);

    if (p == NULL)
        error("OpenGL procedure '%s' not found\n", name);

    return p;
}

/*---------------------------------------------------------------------------*/

#ifdef __APPLE__

void init_opengl(void)
{
}

#else

PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArrayARB;
PFNGLPROGRAMENVPARAMETER4FARBPROC    glProgramEnvParameter4fARB;
PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointerARB;
PFNGLPROGRAMSTRINGARBPROC            glProgramStringARB;
PFNGLBINDPROGRAMARBPROC              glBindProgramARB;
PFNGLGENPROGRAMSARBPROC              glGenProgramsARB;
PFNGLBINDBUFFERARBPROC               glBindBufferARB;
PFNGLGENBUFFERSARBPROC               glGenBuffersARB;
PFNGLBUFFERDATAARBPROC               glBufferDataARB;
PFNGLISBUFFERARBPROC                 glIsBufferARB;
PFNGLDELETEBUFFERSARBPROC            glDeleteBuffersARB;

void init_opengl(void)
{
    print_console(glGetString(GL_EXTENSIONS));

    glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)
        opengl_proc("glProgramStringARB");
    glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)
        opengl_proc("glBindProgramARB");
    glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)
        opengl_proc("glGenProgramsARB");

    if (opengl_need("GL_ARB_fragment_program"))
    {
        glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)
            opengl_proc("glProgramEnvParameter4fARB");

        GL_has_fragment_program = (glProgramEnvParameter4fARB
                                && glProgramStringARB
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

        GL_has_vertex_program = (glDisableVertexAttribArrayARB
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

    if (opengl_need("GL_ARB_point_sprite"))
    {
        GL_has_point_sprite = GL_TRUE;
    }
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
    
    /* Compute the average FPS over 250 milliseconds. */

    if (now - then > 250)
    {
        fps   = 1000.0f * count / (now - then);
        then  = now;
        count = 0;
    }
    else count++;

    /* Compute the total average FPS. */

    if (start)
        total++;
    else
        start = SDL_GetTicks();

    if (all) *all = 1000.0f * total / (now - start);

    return fps;
}

/*---------------------------------------------------------------------------*/

#ifndef NDEBUG

void opengl_check(const char *str)
{
    GLenum err;

    while ((err = glGetError()) != GL_NO_ERROR)
        error("OpenGL error: %s: %s", str, gluErrorString(err));
}

#endif

/*---------------------------------------------------------------------------*/

