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

#ifndef ENTITY_H
#define ENTITY_H

/*---------------------------------------------------------------------------*/

#define TYPE_PIVOT  1
#define TYPE_CAMERA 2
#define TYPE_SPRITE 3
#define TYPE_OBJECT 4
#define TYPE_LIGHT  5

struct entity
{
    int type;
    int data;

    float position[3];
    float rotation[3];
    float scale[3];

    int car;
    int cdr;
    int par;
};

/*---------------------------------------------------------------------------*/

int   buffer_unused(int, int (*)(int));
void *buffer_expand(void *, int *, int);

/*---------------------------------------------------------------------------*/

int  entity_exists(int);
int  entity_istype(int, int);

int  entity_create(int, int);
void entity_parent(int, int);
void entity_delete(int);
void entity_render(void);

void entity_position(int, float, float, float);
void entity_rotation(int, float, float, float);
void entity_scale   (int, float, float, float);

/*---------------------------------------------------------------------------*/

#endif
