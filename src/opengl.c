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

/*---------------------------------------------------------------------------*/

int opengl_has_program      = 0;
int opengl_has_point_sprite = 0;

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

    fprintf(stderr, "Requires %s\n", extension);

    return GL_FALSE;
}

void *opengl_proc(const char *name)
{
    void *p = SDL_GL_GetProcAddress(name);

    if (p == NULL)
        fprintf(stderr, "%s not found\n", name);

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

void init_opengl(void)
{
    if (opengl_need("GL_ARB_vertex_program") &&
        opengl_need("GL_ARB_fragment_program"))
    {
        glDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)
            opengl_proc("glDisableVertexAttribArrayARB");
        glEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)
            opengl_proc("glEnableVertexAttribArrayARB");
        glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)
            opengl_proc("glProgramEnvParameter4fARB");
        glVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC)
            opengl_proc("glVertexAttribPointerARB");
        glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)
            opengl_proc("glProgramStringARB");
        glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)
            opengl_proc("glBindProgramARB");
        glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)
            opengl_proc("glGenProgramsARB");

        opengl_has_program = (glDisableVertexAttribArrayARB
                           && glEnableVertexAttribArrayARB
                           && glProgramEnvParameter4fARB
                           && glVertexAttribPointerARB
                           && glProgramStringARB
                           && glBindProgramARB
                           && glGenProgramsARB);
    }

    if (opengl_need("GL_ARB_point_sprite"))
    {
        opengl_has_point_sprite = 1;
    }
}

#endif

/*---------------------------------------------------------------------------*/

GLint opengl_perf(void)
{
    static GLint fps   = 0;
    static GLint then  = 0;
    static GLint count = 0;

    GLint now = (GLint) SDL_GetTicks();
    
    if (now - then > 250)
    {
        fps   = count * 1000 / (now - then);
        then  = now;
        count = 0;
    }
    else count++;

    return fps;
}

/*---------------------------------------------------------------------------*/

const char *opengl_read(const char *filename)
{
    struct stat buf;

    char *ptr = NULL;
    FILE *fp;

    if (stat(filename, &buf) == 0)
    {
        if ((fp = fopen(filename, "r")))
        {
            if ((ptr = (char *) calloc(1, buf.st_size + 1)))
                fread(ptr, 1, buf.st_size + 1, fp);

            fclose(fp);
        }
        else perror("gl_read_text: fopen()");
    }
    else perror("gl_read_text: stat()");

    return ptr;
}

/*---------------------------------------------------------------------------*/

#ifndef NDEBUG

void opengl_check(const char *str)
{
    GLenum error;

    while ((error = glGetError()) != GL_NO_ERROR)
        fprintf(stderr, "OpenGL error: %s: %s\n", str, gluErrorString(error));
}

#endif

/*---------------------------------------------------------------------------*/

