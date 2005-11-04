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

#include "server.h"
#include "client.h"
#include "utility.h"

/*---------------------------------------------------------------------------*/

#if defined(_WIN32) || defined(__APPLE__)
#define MAIN SDL_main
#else
#define MAIN main
#endif

/*---------------------------------------------------------------------------*/

#ifdef CONF_MPI
#include <mpi.h>

int MAIN(int argc, char *argv[])
{
    if (MPI_Init(&argc, &argv) == MPI_SUCCESS)
    {
        int rank = 0;

        assert_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

        if (rank == 0)
            server(argc, argv);
        else
            client();

        MPI_Finalize();
    }
    return 0;
}

#else  /* MPI */

int MAIN(int argc, char *argv[])
{
    server(argc, argv);
    return 0;
}

#endif /* MPI */

/*---------------------------------------------------------------------------*/
