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

#include <stdlib.h>

#include "opengl.h"
#include "buffer.h"
#include "entity.h"
#include "pivot.h"
#include "event.h"

/*===========================================================================*/

int send_create_pivot(void)
{
    send_event(EVENT_CREATE_PIVOT);

    return send_create_entity(TYPE_PIVOT, 0);
}

void recv_create_pivot(void)
{
    recv_create_entity();
}

/*===========================================================================*/

static void draw_pivot(int i, int j, int f, float a)
{
    glPushMatrix();
    {
        transform_entity(j);

        if (test_entity_aabb(j) >= 0)
            draw_entity_tree(j, f, a * get_entity_alpha(j));
    }
    glPopMatrix();
}

/*===========================================================================*/

static struct entity_func pivot_func = {
    "pivot",
    NULL,
    NULL,
    NULL,
    draw_pivot,
    NULL,
    NULL,
};

struct entity_func *startup_pivot(void)
{
    return &pivot_func;
}
