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

#include <mpi.h>

#include "server.h"
#include "client.h"

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int np;
    int id;

    if (MPI_Init(&argc, &argv) == MPI_SUCCESS)
    {
        if (MPI_Comm_size(MPI_COMM_WORLD, &np) == MPI_SUCCESS &&
            MPI_Comm_rank(MPI_COMM_WORLD, &id) == MPI_SUCCESS)
        {
            if (id == 0)
                server(np, argc, argv);
            else
                client(np, id);
        }
        MPI_Finalize();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
