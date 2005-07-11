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

#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef MPI
#include <mpi.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#define FMODE_RB "rb"
#define FMODE_WB "wb"
#else
#define FMODE_RB "r"
#define FMODE_WB "w"
#endif

/*---------------------------------------------------------------------------*/

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MAXSTR
#define MAXSTR 256
#endif

/*---------------------------------------------------------------------------*/

void *memdup(const void *, size_t, size_t);

const char *system_error(void);

const char *get_file_name(const char *);
const char *get_file_path(const char *);

void  open_path(const char *);
int   stat_file(const char *, struct stat *);
FILE *open_file(const char *, const char *);
void *load_file(const char *, const char *, size_t *);

/*---------------------------------------------------------------------------*/

float host_to_net_float(float);
float net_to_host_float(float);

int   host_to_net_int(int);
int   net_to_host_int(int);

/*---------------------------------------------------------------------------*/

char *alloc_text(const char *);

/*---------------------------------------------------------------------------*/

void *error(char *, ...);
void *print(char *, ...);

/*---------------------------------------------------------------------------*/

void assert_mpi(int);

/*---------------------------------------------------------------------------*/

#endif
