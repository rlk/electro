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
#include <stdio.h>
#include <sys/stat.h>

#ifdef MPI
#include <mpi.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#endif

#include "image.h"
#include "shared.h"
#include "camera.h"

/*---------------------------------------------------------------------------*/
/* Optional MPI function abstractions                                        */

int mpi_assert(int err)
{
#ifdef MPI
    char buf[256];
    int len = 256;

    if (err == MPI_SUCCESS)
        return 1;
    else
    {
        MPI_Error_string(err, buf, &len);
        fprintf(stderr, "MPI Error: %s\n", buf);
        return 0;
    }
#else
    return 1;
#endif
}

int mpi_rank(void)
{
    int rank = 0;
#ifdef MPI
    mpi_assert(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
#endif
    return rank;
}

int mpi_size(void)
{
    int size = 0;
#ifdef MPI
    mpi_assert(MPI_Comm_size(MPI_COMM_WORLD, &size));
#endif
    return size;
}

int mpi_isroot(void)
{
    return (mpi_rank() == 0);
}

void mpi_barrier(void)
{
#ifdef MPI
#ifndef NDEBUG
    printf("%d of %d: barrier\n", mpi_rank(), mpi_size());
#endif
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}

/*---------------------------------------------------------------------------*/

const char *event_string(int type)
{
    switch (type)
    {
    case EVENT_DRAW:          return "draw";
    case EVENT_EXIT:          return "exit";

    case EVENT_ENTITY_PARENT: return "entity parent";
    case EVENT_ENTITY_DELETE: return "entity delete";
    case EVENT_ENTITY_MOVE:   return "entity move";
    case EVENT_ENTITY_TURN:   return "entity turn";
    case EVENT_ENTITY_SIZE:   return "entity size";

    case EVENT_CAMERA_CREATE: return "camera create";
    case EVENT_SPRITE_CREATE: return "sprite create";
    case EVENT_OBJECT_CREATE: return "object create";
    case EVENT_LIGHT_CREATE:  return "light create";
    case EVENT_PIVOT_CREATE:  return "pivot create";

    case EVENT_CAMERA_DIST:   return "camera dist";
    case EVENT_CAMERA_ZOOM:   return "camera zoom";
    }

    return "UNKNOWN";
}

/*---------------------------------------------------------------------------*/

