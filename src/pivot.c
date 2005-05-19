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

#include "opengl.h"
#include "buffer.h"
#include "entity.h"
#include "event.h"

/*===========================================================================*/

int send_create_pivot(void)
{
    pack_event(EVENT_CREATE_PIVOT);

    return send_create_entity(TYPE_PIVOT, 0);
}

void recv_create_pivot(void)
{
    recv_create_entity();
}

/*===========================================================================*/

static void draw_pivot(int j, int i, const float M[16],
                                     const float I[16],
                                     const struct frustum *F, float a)
{
    float N[16];
    float J[16];

    glPushMatrix();
    {
        transform_entity(j, N, M, J, I);
        draw_entity_tree(j, N, J, F, a * get_entity_alpha(j));
    }
    glPopMatrix();
}

/*===========================================================================*/

struct entity_func pivot_func = {
    "pivot",
    NULL,
    NULL,
    draw_pivot,
    NULL,
    NULL,
};
