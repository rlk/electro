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

#define TYPE_NULL    0
#define TYPE_ROOT    1
#define TYPE_CAMERA  2
#define TYPE_SPRITE  3
#define TYPE_OBJECT  4
#define TYPE_STRING  5
#define TYPE_GALAXY  6
#define TYPE_LIGHT   7
#define TYPE_PIVOT   8
#define TYPE_TERRAIN 9
#define TYPE_COUNT  10

/* Entity state flags. */

#define FLAG_HIDDEN        0x0001
#define FLAG_WIREFRAME     0x0002
#define FLAG_BILLBOARD     0x0004
#define FLAG_BALLBOARD     0x0008
#define FLAG_BOUNDED       0x0010
#define FLAG_LINE_SMOOTH   0x0020
#define FLAG_LEFT_EYE      0x0040
#define FLAG_RIGHT_EYE     0x0080
#define FLAG_TRACK_POS     0x0100
#define FLAG_TRACK_ROT     0x0200
#define FLAG_VISIBLE_GEOM  0x0400
#define FLAG_VISIBLE_BODY  0x0800

/* Entity tracking modes. */

#define TRACK_LOCAL        0x0000
#define TRACK_WORLD        0x0001

/* Initial entity vector sizes. */

#define MIN_CAMERAS    4
#define MIN_SPRITES  256
#define MIN_OBJECTS  128
#define MIN_STRINGS  128
#define MIN_GALAXIES   4
#define MIN_TERRAINS   2
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

int         get_entity_data(unsigned int);
int         get_entity_type(unsigned int);
const char *get_entity_name(unsigned int);

/*---------------------------------------------------------------------------*/

void transform_camera(unsigned int);
void transform_entity(unsigned int);
void draw_entity_tree(unsigned int, int, float);
int  test_entity_aabb(unsigned int);

/*---------------------------------------------------------------------------*/

int startup_entity(void);

/*===========================================================================*/

unsigned int send_create_entity(int, int);
void         recv_create_entity(void);

unsigned int send_create_clone(unsigned int);
void         recv_create_clone(void);

void send_parent_entity(unsigned int, unsigned int);
void recv_parent_entity(void);

void send_delete_entity(unsigned int);
void recv_delete_entity(void);

/*---------------------------------------------------------------------------*/

void  send_set_entity_tracking (unsigned int, int, int);
void  send_set_entity_position (unsigned int, const float[3]);
void  send_set_entity_rotation (unsigned int, const float[3]);
void  send_set_entity_scale    (unsigned int, const float[3]);
void  send_set_entity_bound    (unsigned int, const float[6]);
void  send_set_entity_basis    (unsigned int, const float[16]);
void  send_set_entity_alpha    (unsigned int, float);
void  send_set_entity_flags    (unsigned int, int, int);

void  send_move_entity(unsigned int, const float[3]);
void  send_turn_entity(unsigned int, const float[3]);

/*---------------------------------------------------------------------------*/

void  recv_set_entity_position(void);
void  recv_set_entity_basis   (void);
void  recv_set_entity_scale   (void);
void  recv_set_entity_bound   (void);
void  recv_set_entity_alpha   (void);
void  recv_set_entity_flags   (void);

/*---------------------------------------------------------------------------*/

void  get_entity_position(unsigned int, float[3]);
void  get_entity_rotation(unsigned int, float[3]);
void  get_entity_x_vector(unsigned int, float[3]);
void  get_entity_y_vector(unsigned int, float[3]);
void  get_entity_z_vector(unsigned int, float[3]);
void  get_entity_scale   (unsigned int, float[3]);
void  get_entity_bound   (unsigned int, float[6]);
float get_entity_alpha   (unsigned int);
int   get_entity_flags   (unsigned int);

/*---------------------------------------------------------------------------*/

void  set_entity_body_type(unsigned int,               int);
void  set_entity_geom_type(unsigned int,               int, const float *);
void  set_entity_join_type(unsigned int, unsigned int, int);

void  set_entity_body_attr_i(unsigned int,               int, int);
void  set_entity_geom_attr_i(unsigned int,               int, int);
void  set_entity_geom_attr_f(unsigned int,               int, float);
void  set_entity_join_attr_f(unsigned int, unsigned int, int, float);
void  set_entity_join_attr_v(unsigned int, unsigned int, int, const float *);

int   get_entity_body_attr_i(unsigned int, int);
void  get_entity_body_attr_v(unsigned int, int, float *);
int   get_entity_geom_attr_i(unsigned int, int);
float get_entity_geom_attr_f(unsigned int, int);
float get_entity_join_attr_f(unsigned int, unsigned int, int);
void  get_entity_join_attr_v(unsigned int, unsigned int, int, float *);

void  add_entity_force (unsigned int, float, float, float);
void  add_entity_torque(unsigned int, float, float, float);

/*---------------------------------------------------------------------------*/

unsigned int get_entity_parent(unsigned int);
unsigned int get_entity_child (unsigned int, unsigned int);

/*===========================================================================*/

int  step_entities(float, int);
void draw_entities(void);

void nuke_entities(void);
void init_entities(void);
void fini_entities(void);

/*===========================================================================*/

#endif
