#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#include "opengl.h"

/*---------------------------------------------------------------------------*/

GLboolean gl_supported(const char *extension)
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
                return 1;

        start = space;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

const char *gl_read_text(const char *filename)
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

void gl_fps(void)
{
    static int fps   = 0;
    static int then  = 0;
    static int count = 0;

    int now = SDL_GetTicks();

    if (now - then > 250)
    {
        char buf[16];

        fps   = count * 1000 / (now - then);
        then  = now;
        count = 0;

        sprintf(buf, "%d", fps);
        SDL_WM_SetCaption(buf, buf);
    }
    else count++;
}

/*---------------------------------------------------------------------------*/
