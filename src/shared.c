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

#ifdef MPI
#include <mpi.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#endif

/*---------------------------------------------------------------------------*/
/* Optional MPI function abstractions                                        */

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

/*---------------------------------------------------------------------------*/

void mpi_barrier(void)
{
#ifdef MPI
    mpi_assert(MPI_Barrier(MPI_COMM_WORLD));
#endif
}

/*---------------------------------------------------------------------------*/
