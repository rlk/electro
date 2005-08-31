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

#include "opengl.h"

/*---------------------------------------------------------------------------*/

/* Entity type tags. */

#define TYPE_NULL   0
#define TYPE_ROOT   1
#define TYPE_CAMERA 2
#define TYPE_SPRITE 3
#define TYPE_OBJECT 4
#define TYPE_STRING 5
#define TYPE_GALAXY 6
#define TYPE_LIGHT  7
#define TYPE_PIVOT  8
#define TYPE_COUNT  9

/* Entity state flags. */

#define FLAG_HIDDEN        0x0001
#define FLAG_WIREFRAME     0x0002
#define FLAG_BILLBOARD     0x0004
#define FLAG_LINE_SMOOTH   0x0010
#define FLAG_POS_TRACKED_0 0x0020
#define FLAG_ROT_TRACKED_0 0x0040
#define FLAG_POS_TRACKED_1 0x0080
#define FLAG_ROT_TRACKED_1 0x0100
#define FLAG_POS_TRACKED_2 0x0200
#define FLAG_ROT_TRACKED_2 0x0400
#define FLAG_STEREO        0x0800

/* Initial entity vector sizes. */

#define MIN_CAMERAS    4
#define MIN_SPRITES  256
#define MIN_OBJECTS  128
#define MIN_STRINGS  128
#define MIN_GALAXIES   4
#define MIN_LIGHTS     8

/*---------------------------------------------------------------------------*/

/* Entity virtual function table. */

typedef void (*init_func)(int);
typedef void (*fini_func)(int);
typedef void (*aabb_func)(int, float[6]);
typedef void (*draw_func)(int, int, int, float);
typedef void (*dupe_func)(int);
typedef void (*free_func)(int);

struct entity_func
{
    const char *name;
    init_func   init;
    fini_func   fini;
    aabb_func   aabb;
    draw_func   draw;
    dupe_func   dupe;
    free_func   free;
};

/*===========================================================================*/

int         entity_data(int);
int         entity_type(int);
const char *entity_name(int);

/*---------------------------------------------------------------------------*/

void transform_camera(int);
void transform_entity(int);
void draw_entity_tree(int, int, float);
int  test_entity_aabb(int);

/*---------------------------------------------------------------------------*/

int startup_entity(void);

/*===========================================================================*/

int  send_create_entity(int, int);
void recv_create_entity(void);

int  send_create_clone(int);
void recv_create_clone(void);

void send_parent_entity(int, int);
void recv_parent_entity(void);

void send_delete_entity(int);
void recv_delete_entity(void);

/*---------------------------------------------------------------------------*/

void  send_set_entity_position (int, const float[3]);
void  send_set_entity_rotation (int, const float[3]);
void  send_set_entity_scale    (int, const float[3]);
void  send_set_entity_bound    (int, const float[6]);
void  send_set_entity_basis    (int, const float[16]);
void  send_set_entity_alpha    (int, float);
void  send_set_entity_flags    (int, int, int);

void  send_move_entity(int, const float[3]);
void  send_turn_entity(int, const float[3]);

/*---------------------------------------------------------------------------*/

void  recv_set_entity_position(void);
void  recv_set_entity_basis   (void);
void  recv_set_entity_scale   (void);
void  recv_set_entity_bound   (void);
void  recv_set_entity_alpha   (void);
void  recv_set_entity_flags   (void);

/*---------------------------------------------------------------------------*/

void  get_entity_position(int, float[3]);
void  get_entity_x_vector(int, float[3]);
void  get_entity_y_vector(int, float[3]);
void  get_entity_z_vector(int, float[3]);
void  get_entity_scale   (int, float[3]);
void  get_entity_bound   (int, float[6]);
float get_entity_alpha   (int);
int   get_entity_flags   (int);

/*---------------------------------------------------------------------------*/

void  set_entity_body_type(int,      int);
void  set_entity_geom_type(int,      int, const float *);
void  set_entity_join_type(int, int, int);

void  set_entity_body_attr_i(int,      int, int);
void  set_entity_geom_attr_i(int,      int, int);
void  set_entity_geom_attr_f(int,      int, float);
void  set_entity_join_attr_f(int, int, int, float);
void  set_entity_join_attr_v(int, int, int, const float *);

int   get_entity_body_attr_i(int, int);
void  get_entity_body_attr_v(int, int, float *);
int   get_entity_geom_attr_i(int, int);
float get_entity_geom_attr_f(int, int);
float get_entity_join_attr_f(int, int, int);
void  get_entity_join_attr_v(int, int, int, float *);

void  add_entity_force (int, float, float, float);
void  add_entity_torque(int, float, float, float);

/*---------------------------------------------------------------------------*/

int   get_entity_parent(int);
int   get_entity_child(int, int);

/*===========================================================================*/

int  step_entities(float, int);
void draw_entities(void);

void nuke_entities(void);
void init_entities(void);
void fini_entities(void);

/*===========================================================================*/

#endif
