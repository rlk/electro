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

#ifndef FRUSTUM_H
#define FRUSTUM_H

/*---------------------------------------------------------------------------*/

struct frustum
{
    float p[3];       /* View position   (in world coordinates)              */
    float c[3];       /* Viewport center (in world coordinates)              */
    float r[3];       /* Right vector (from center to viewport right extent) */
    float u[3];       /* Up vector    (from center to viewport upper extent) */

    float V[4][4];    /* View volume                                         */
};

/*---------------------------------------------------------------------------*/

int test_frustum(const struct frustum *, const float[6]);

/*---------------------------------------------------------------------------*/

#endif
