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

#include "frustum.h"

/*---------------------------------------------------------------------------*/

static int test_plane(const float V[4], const float b[6])
{
    const float k00 = b[0] * V[0];
    const float k11 = b[1] * V[1];
    const float k22 = b[2] * V[2];
    const float k30 = b[3] * V[0];
    const float k41 = b[4] * V[1];
    const float k52 = b[5] * V[2];

    int c = 0;

    if (k00 + k11 + k22 > V[3]) c++;
    if (k00 + k11 + k52 > V[3]) c++;
    if (k00 + k41 + k22 > V[3]) c++;
    if (k00 + k41 + k52 > V[3]) c++;
    if (k30 + k11 + k22 > V[3]) c++;
    if (k30 + k11 + k52 > V[3]) c++;
    if (k30 + k41 + k22 > V[3]) c++;
    if (k30 + k41 + k52 > V[3]) c++;

    return c;
}

int test_frustum(const struct frustum *F, const float b[6])
{
    int c0, c1, c2, c3;

    if ((c0 = test_plane(F->V[0], b)) == 0) return -1;
    if ((c1 = test_plane(F->V[1], b)) == 0) return -1;
    if ((c2 = test_plane(F->V[2], b)) == 0) return -1;
    if ((c3 = test_plane(F->V[3], b)) == 0) return -1;

    return (c0 + c1 + c2 + c3 == 32) ? 1 : 0;
}

/*---------------------------------------------------------------------------*/
