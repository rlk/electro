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

#ifndef SHARED_H
#define SHARED_H

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

#ifdef MPI
#include <mpi.h>
#endif

int  mpi_assert(int);
int  mpi_isroot(void);

int  mpi_rank(void);
int  mpi_size(void);

void mpi_barrier(void);

/*---------------------------------------------------------------------------*/

#endif
