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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/errno.h>
#endif

#include "utility.h"
#include "console.h"

/*---------------------------------------------------------------------------*/

const char *system_error(void)
{
    return strerror(errno);
}

/*---------------------------------------------------------------------------*/
/* File path parsing                                                         */

#ifdef _WIN32
#define is_sep(c) ((c) == '/' || (c) == '\\')
#else
#define is_sep(c) ((c) == '/')
#endif

static int get_file_split(const char *path)
{
    int l = strlen(path);

    while (l >= 0 && !is_sep(path[l]))
        l--;

    return l;
}

const char *get_file_path(const char *file)
{
    static char path[MAXSTR];
    int l;

    memset(path, 0, MAXSTR);

    if ((l = get_file_split(file)) >= 0)
        strncpy(path, file, l);
    else
        strcpy(path, ".");

    return path;
}

const char *get_file_name(const char *file)
{
    static char name[MAXSTR];
    int l;

    memset(name, 0, MAXSTR);

    if ((l = get_file_split(file)) >= 0)
        strncpy(name, file + l + 1, MAXSTR);
    else
        strncpy(name, file, MAXSTR);

    return name;
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

void *error(char *format, ...)
{
    char string[MAXSTR];
    va_list args;

    va_start(args, format);
    vsprintf(string, format, args);
    va_end(args);

    error_console(string);

    return NULL;
}

void *print(char *format, ...)
{
    char string[MAXSTR];
    va_list args;

    va_start(args, format);
    vsprintf(string, format, args);
    va_end(args);

    print_console(string);

    return NULL;
}

/*---------------------------------------------------------------------------*/

#ifdef MPI

void assert_mpi(int err)
{
    if (err != MPI_SUCCESS)
    {
        char buf[MAXSTR];
        int  n = MAXSTR;

        MPI_Error_string(err, buf, &n);
        error("MPI Error: %s\n", buf);
    }
}

#endif

/*---------------------------------------------------------------------------*/
