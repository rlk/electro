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
#include "pivot.h"

/*---------------------------------------------------------------------------*/

void pivot_draw(int id, int pd, const float V[16])
{
    float W[16];

    glPushMatrix();
    {
        entity_transform(id, W, V);
        entity_traversal(id, W);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

int pivot_send_create(void)
{
    pack_event(EVENT_PIVOT_CREATE);

    return entity_send_create(TYPE_PIVOT, 0);
}

void pivot_recv_create(void)
{
    entity_recv_create();
}

/*---------------------------------------------------------------------------*/
