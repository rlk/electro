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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

/*---------------------------------------------------------------------------*/

static struct viewport *Vi;
static struct viewport *Vo;
static int              V_max = 0;
static int              V_num = 0;

void viewport_init(int n)
{
    Vi = (struct viewport *) calloc(n, sizeof (struct viewport));
    Vo = (struct viewport *) calloc(n, sizeof (struct viewport));

    V_max = n;
    V_num = 0;
}

void viewport_tile(const char *name, int gx, int gy,
                                     int lx, int ly, int w, int h)
{
    if (V_num < V_max)
    {
        strncpy(Vi[V_num].name, name, NAMELEN);

        Vi[V_num].gx = gx;
        Vi[V_num].gy = gy;
        Vi[V_num].lx = lx;
        Vi[V_num].ly = ly;
        Vi[V_num].w  =  w;
        Vi[V_num].h  =  h;

        V_num++;
    }
}

void viewport_sync(int i, int n)
{
    size_t sz = sizeof (struct viewport);
    int err;

    struct viewport Vt;

    /* Get this client's host name.  Set a default viewport. */

    gethostname(Vt.name, NAMELEN);

    Vt.gx =   0;
    Vt.gy =   0;
    Vt.lx =   0;
    Vt.ly =   0;
    Vt.w  = 800;
    Vt.h  = 600;

    /* Gather all names at the root. */

    if ((err = MPI_Gather(&Vt, sz, MPI_BYTE,
                           Vo, sz, MPI_BYTE, 0, MPI_COMM_WORLD))
            != MPI_SUCCESS)
        mpi_error(err);

    /* If this is the root, assign viewports by matching names. */

    if (i == 0)
    {
        int j;
        int k;

        for (j = 1; j < n; j++)
            for (k = 0; k < V_num; k++)
                if (strcmp(Vo[j].name, Vi[k].name) == 0)
                {
                    /* A name matches.  Copy the viewport definition. */

                    Vo[j].gx = Vi[k].gx;
                    Vo[j].gy = Vi[k].gy;
                    Vo[j].lx = Vi[k].lx;
                    Vo[j].ly = Vi[k].ly;
                    Vo[j].w  = Vi[k].w;
                    Vo[j].h  = Vi[k].h;

                    /* Destroy the name to ensure it matches at most once. */

                    strcpy(Vi[k].name, "");

                    break;
                }
    }

    /* Scatter the assignments to all clients. */

    if ((err = MPI_Scatter(Vo, sz, MPI_BYTE,
                          &Vt, sz, MPI_BYTE, 0, MPI_COMM_WORLD))
            != MPI_SUCCESS)
        mpi_error(err);

    /* Apply this client's viewport. */

    status_set_viewport(Vt.gx, Vt.gy, Vt.lx, Vt.ly, Vt.w, Vt.h);
}

/*---------------------------------------------------------------------------*/

void mpi_error(int err)
{
    char buf[256];
    int len = 256;

    MPI_Error_string(err, buf, &len);
    fprintf(stderr, "%s\n", buf);
}

/*---------------------------------------------------------------------------*/

