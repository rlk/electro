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

#define TYPE_ROOT   1
#define TYPE_CAMERA 2
#define TYPE_SPRITE 3
#define TYPE_OBJECT 4
#define TYPE_LIGHT  5
#define TYPE_PIVOT  6

#define FLAG_HIDDEN    1
#define FLAG_WIREFRAME 2
#define FLAG_BILLBOARD 4

struct entity
{
    int type;
    int data;
    int flag;

    float position[3];
    float rotation[3];
    float scale[3];
    float alpha;

    int car;
    int cdr;
    int par;
};

/*---------------------------------------------------------------------------*/

int   buffer_unused(int, int (*)(int));
void *buffer_expand(void *, int *, int);

/*---------------------------------------------------------------------------*/

const char *entity_typename(int);

int  entity_exists(int);
int  entity_todata(int);
int  entity_istype(int, int);

void entity_transform(int);
void entity_traversal(int, float);

/*---------------------------------------------------------------------------*/

int  entity_init(void);
void entity_draw(void);

/*---------------------------------------------------------------------------*/

int  entity_send_create(int, int);
void entity_recv_create(void);

void entity_send_parent(int, int);
void entity_recv_parent(void);

void entity_send_delete(int);
void entity_recv_delete(void);

int  entity_send_clone(int);
void entity_recv_clone(void);

void entity_send_flag(int, int, int);
void entity_recv_flag(void);

/*---------------------------------------------------------------------------*/

void entity_send_position(int, float, float, float);
void entity_send_rotation(int, float, float, float);
void entity_send_scale   (int, float, float, float);
void entity_send_alpha   (int, float);

void entity_recv_position(void);
void entity_recv_rotation(void);
void entity_recv_scale   (void);
void entity_recv_alpha   (void);

void entity_get_position(int, float *, float *, float *);
void entity_get_rotation(int, float *, float *, float *);
void entity_get_scale   (int, float *, float *, float *);

/*---------------------------------------------------------------------------*/

#endif
