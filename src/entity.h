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
#define TYPE_GALAXY 5
#define TYPE_LIGHT  6
#define TYPE_PIVOT  7

#define FLAG_HIDDEN    1
#define FLAG_WIREFRAME 2
#define FLAG_BILLBOARD 4
#define FLAG_UNLIT     8

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

int buffer_unused(int, int (*)(int));

/*---------------------------------------------------------------------------*/

const char *get_entity_type_name(int);

int  entity_exists(int);

int  entity_data(int);
int  entity_type(int);

void transform_entity(int, float[16], const float[16]);
void draw_entity_list(int,            const float[16], float);

/*---------------------------------------------------------------------------*/

int  init_entity(void);
void draw_entity(void);

/*---------------------------------------------------------------------------*/

int  send_create_entity(int, int);
void recv_create_entity(void);

int  send_create_clone(int);
void recv_create_clone(void);

void send_parent_entity(int, int);
void recv_parent_entity(void);

void send_delete_entity(int);
void recv_delete_entity(void);

void send_set_entity_flag(int, int, int);
void recv_set_entity_flag(void);

/*---------------------------------------------------------------------------*/

void send_set_entity_position(int, float, float, float);
void send_set_entity_rotation(int, float, float, float);
void send_set_entity_scale   (int, float, float, float);
void send_set_entity_alpha   (int, float);

void recv_set_entity_position(void);
void recv_set_entity_rotation(void);
void recv_set_entity_scale   (void);
void recv_set_entity_alpha   (void);

/*---------------------------------------------------------------------------*/

void  get_entity_position(int, float *, float *, float *);
void  get_entity_rotation(int, float *, float *, float *);
void  get_entity_scale   (int, float *, float *, float *);
float get_entity_alpha   (int);

/*---------------------------------------------------------------------------*/

#endif
