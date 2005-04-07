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
#include "entity.h"
#include "event.h"

/*---------------------------------------------------------------------------*/

void draw_pivot(int id, int pd, const struct frustum *F0, float a)
{
    struct frustum F1;

    glPushMatrix();
    {
        const float d[3] = { 0.0f, 0.0f, 0.0f };

        transform_entity(id, &F1, F0, d);
        draw_entity_list(id, &F1, a * get_entity_alpha(id));
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
