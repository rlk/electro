#include <mpi.h>
#include <stdio.h>

#include "shared.h"

/*---------------------------------------------------------------------------*/

void mpi_error(int err)
{
    char buf[256];
    int len = 256;

    MPI_Error_string(err, buf, &len);
    fprintf(stderr, "%s\n", buf);
}

/*---------------------------------------------------------------------------*/
