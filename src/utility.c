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

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "utility.h"

/*---------------------------------------------------------------------------*/
/* Current working directory management                                      */

static char path[MAXSTR];

const char *get_cwd(const char *file)
{
    /*
    static char absolute[MAXSTR];

    size_t d = strlen(path);

    strncpy(absolute, path, MAXSTR - 1);
    strncat(absolute, "/",  MAXSTR - d - 1);
    strncat(absolute, file, MAXSTR - d - 2);

    return absolute;
    */
    return file;
}

void set_cwd(const char *pathname)
{
    int l = strlen(pathname);

    strncpy(path, pathname, MAXSTR);

    /* Strip everything beyond the last directory divider. */

    while (l >= 0 && path[l] != '/' &&
                     path[l] != '\\')
        path[l--] = '\0';

    chdir(path);
}

/*---------------------------------------------------------------------------*/
/* Byte order float handlers                                                 */

union swapper
{
    float f;
    long  l;
};

float htonf(float f)
{
    union swapper s;

    s.f = f;
    s.l = htonl(s.l);

    return s.f;
}

float ntohf(float f)
{
    union swapper s;

    s.f = f;
    s.l = ntohl(s.l);

    return s.f;
}

/*---------------------------------------------------------------------------*/

int balloc(void **buf, int *len, size_t siz, int (*occupied)(int))
{
    void *ptr;
    int   i;

    /* Scan for an unused vector element. */

    for (i = 0; i < *len; ++i)
        if (!occupied(i))
            return i;

    /* The vector is full.  Reallocate it at double size. */

    if ((ptr = realloc(*buf, *len * 2 * siz)))
    {
        i    = *len;
        *len = *len * 2;
        *buf =  ptr;

        memset(((unsigned char *) (*buf)) + *len * siz, 0, *len * siz);
    }
    else i = -1;

    return i;
}

/*---------------------------------------------------------------------------*/
