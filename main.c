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
