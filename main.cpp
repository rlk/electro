#include <mpi.h>

#include "client.hpp"
#include "server.hpp"

//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int rank;

    if (MPI_Init(&argc, &argv) == MPI_SUCCESS)
    {
        if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) == MPI_SUCCESS)
        {
            if (rank == 0)
                server(320, 240);
            else
                client();
        }
        MPI_Finalize();
    }

    return 0;
}

//-----------------------------------------------------------------------------

