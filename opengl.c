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

#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#include "opengl.h"

/*---------------------------------------------------------------------------*/

GLboolean opengl_need(const char *extension)
{
    const GLubyte *string = glGetString(GL_EXTENSIONS);
    const GLubyte *start  = string;

    GLubyte *where;
    GLubyte *space;

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

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArrayARB;
PFNGLPROGRAMENVPARAMETER4FARBPROC   glProgramEnvParameter4fARB;
PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointerARB;
PFNGLPROGRAMSTRINGARBPROC            glProgramStringARB;
#endif

GLboolean opengl_init(void)
{
    if (opengl_need("GL_ARB_vertex_program") &&
        opengl_need("GL_ARB_fragment_program"))
    {
#ifdef _WIN32
        glDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)
            SDL_GL_GetProcAddress("glDisableVertexAttribArrayARB");
        glEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)
            SDL_GL_GetProcAddress("glEnableVertexAttribArrayARB");
        glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)
            SDL_GL_GetProcAddress("glProgramenvParameter4fARB");
        glVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC)
            SDL_GL_GetProcAddress("glVertexAttribPointerARB");
        glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)
            SDL_GL_GetProcAddress("glProgramStringARB");
#endif
    }
    else return GL_FALSE;

    if (opengl_need("GL_ARB_point_sprite"))
        return GL_TRUE;
    else
        return GL_FALSE;
}

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
