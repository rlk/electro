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

/*---------------------------------------------------------------------------*/

#include "opengl.h"
#include "buffer.h"
#include "shared.h"
#include "entity.h"
#include "event.h"
#include "pivot.h"

/*---------------------------------------------------------------------------*/

void draw_pivot(int id, int pd, const float V[16], float a)
{
    float W[16];

    glPushMatrix();
    {
        transform_entity(id, W, V);
        draw_entity_list(id, W, a * get_entity_alpha(id));
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

int send_create_pivot(void)
{
    pack_event(EVENT_CREATE_PIVOT);

    return send_create_entity(TYPE_PIVOT, 0);
}

void recv_create_pivot(void)
{
    recv_create_entity();
}

/*---------------------------------------------------------------------------*/
