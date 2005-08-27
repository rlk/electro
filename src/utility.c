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

#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <direct.h>
#else
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/errno.h>
#endif

#include "utility.h"
#include "console.h"

#define MAXPATH 16

/*---------------------------------------------------------------------------*/

const char *system_error(void)
{
    return strerror(errno);
}

/*---------------------------------------------------------------------------*/

void *memdup(const void *src, size_t num, size_t len)
{
    void *dst = NULL;

    if (src && (num * len > 0) && (dst = malloc(num * len)))
        memcpy(dst, src, num * len);

    return dst;
}

/*---------------------------------------------------------------------------*/
/* File path parsing                                                         */

#ifdef _WIN32
#define FILESEP "\\"
#define is_sep(c) ((c) == '/' || (c) == '\\')
#else
#define FILESEP "/"
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
        strncpy(path, file, l + 1);
    else
        strcpy(path, "./");

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
/* Path name stack for smart file locating.                                  */

static char path_stack[MAXPATH][MAXSTR];
static int  path_index = 0;

void path_push(const char *path)
{
    if (path_index < MAXPATH)
    {
        getcwd(path_stack[path_index++], MAXSTR);
        chdir(path);
    }
}

void path_pop(void)
{
    if (path_index > 0)
        chdir(path_stack[--path_index]);
}

/*---------------------------------------------------------------------------*/

int stat_file(const char *filename, struct stat *buf)
{
    int r;

    /* Search for the file in the current working directory. */

    if ((r = stat(filename, buf)) == 0)
        return 0;

    /* Can't find it.  Punt. */

    return r;
}

FILE *open_file(const char *filename, const char *mode)
{
    FILE *fp;

    /* Search for the file in the current working directory. */

    if ((fp = fopen(filename, mode)))
        return fp;

    /* Can't find it.  Punt. */

    return NULL;
}

void *load_file(const char *filename, const char *mode, size_t *size)
{
    struct stat buf;
    void *ptr = NULL;
    FILE *fp  = NULL;

    if (filename && mode)
    {
        if (stat_file(filename, &buf) == 0)
        {
            if ((fp = open_file(filename, mode)))
            {
                if ((ptr = calloc(buf.st_size + 1, 1)))
                    fread(ptr, 1, buf.st_size, fp);

                if (size)
                   *size = buf.st_size;

                fclose(fp);
            }
            else error ("'%s': %s", filename, system_error());
        }
        else error("'%s': %s", filename, system_error());
    }
    return ptr;
}

/*---------------------------------------------------------------------------*/
/* Byte order handlers                                                       */

static const int      swapint = 0x00010203;
static unsigned char *swapmap = (unsigned char *) &swapint;

union swapper
{
    float         f;
    int           d;
    unsigned char b[4];
};

void host_to_net(union swapper *s)
{
    union swapper t = *s;

    s->b[0] = t.b[swapmap[0]];
    s->b[1] = t.b[swapmap[1]];
    s->b[2] = t.b[swapmap[2]];
    s->b[3] = t.b[swapmap[3]];
}

void net_to_host(union swapper *s)
{
    union swapper t = *s;

    s->b[swapmap[0]] = t.b[0];
    s->b[swapmap[1]] = t.b[1];
    s->b[swapmap[2]] = t.b[2];
    s->b[swapmap[3]] = t.b[3];
}

/*---------------------------------------------------------------------------*/

float host_to_net_float(float f)
{
    union swapper s;

    s.f = f;
    host_to_net(&s);

    return s.f;
}

float net_to_host_float(float f)
{
    union swapper s;

    s.f = f;
    net_to_host(&s);

    return s.f;
}

int host_to_net_int(int d)
{
    union swapper s;

    s.d = d;
    host_to_net(&s);

    return s.d;
}

int net_to_host_int(int d)
{
    union swapper s;

    s.d = d;
    net_to_host(&s);

    return s.d;
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

void *debug(char *format, ...)
{
    char string[MAXSTR];
    va_list args;

    va_start(args, format);
    vsprintf(string, format, args);
    va_end(args);

    debug_console(string);

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
