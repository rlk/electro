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
#include "shared.h"
#include "server.h"
#include "entity.h"
#include "pivot.h"

/*---------------------------------------------------------------------------*/

int pivot_create(void)
{
    if (mpi_isroot())
        server_send(EVENT_PIVOT_CREATE);

    return entity_create(TYPE_PIVOT, 0);
}

void pivot_render(int id, int pd)
{
    glPushMatrix();
    {
        entity_transform(id);
        entity_traversal(id);
    }
    glPopMatrix();

    opengl_check("pivot_render");
}

/*---------------------------------------------------------------------------*/
