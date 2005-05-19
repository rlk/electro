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
#include "frustum.h"

/*---------------------------------------------------------------------------*/

/* Entity type tags. */

#define TYPE_NULL   0
#define TYPE_ROOT   1
#define TYPE_CAMERA 2
#define TYPE_SPRITE 3
#define TYPE_OBJECT 4
#define TYPE_GALAXY 5
#define TYPE_LIGHT  6
#define TYPE_PIVOT  7
#define TYPE_COUNT  8

/* Entity state flags. */

#define FLAG_HIDDEN        0x0001
#define FLAG_WIREFRAME     0x0002
#define FLAG_BILLBOARD     0x0004
#define FLAG_UNLIT         0x0008
#define FLAG_LINE_SMOOTH   0x0010
#define FLAG_POS_TRACKED_0 0x0020
#define FLAG_ROT_TRACKED_0 0x0040
#define FLAG_POS_TRACKED_1 0x0080
#define FLAG_ROT_TRACKED_1 0x0100

/*---------------------------------------------------------------------------*/

/* Entity virtual function table. */

typedef void (*init_func)(int);
typedef void (*fini_func)(int);
typedef void (*dupe_func)(int);
typedef void (*free_func)(int);
typedef void (*draw_func)(int, int, const float[16],
                                    const float[16],
                                    const struct frustum *, float);
struct entity_func
{
    const char *name;
    init_func   init;
    fini_func   fini;
    draw_func   draw;
    dupe_func   dupe;
    free_func   free;
};

/*===========================================================================*/

int         entity_data(int);
int         entity_type(int);
const char *entity_name(int);

/*---------------------------------------------------------------------------*/

void transform_camera(int, float[16], const float[16],
                           float[16], const float[16], const float[3]);
void transform_entity(int, float[16], const float[16],
                           float[16], const float[16]);
void draw_entity_tree(int, const float[16],
                           const float[16], const struct frustum *, float);

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
void  send_set_entity_basis    (int, const float[3],
                                     const float[3],
                                     const float[3]);

void  send_set_entity_alpha    (int, float);
void  send_set_entity_flag     (int, int, int);
void  send_set_entity_frag_prog(int, const char *);
void  send_set_entity_vert_prog(int, const char *);

void  send_move_entity(int, const float[3]);
void  send_turn_entity(int, const float[3]);

/*---------------------------------------------------------------------------*/

void  recv_set_entity_position (void);
void  recv_set_entity_basis    (void);
void  recv_set_entity_scale    (void);
void  recv_set_entity_alpha    (void);
void  recv_set_entity_flag     (void);
void  recv_set_entity_frag_prog(void);
void  recv_set_entity_vert_prog(void);

/*---------------------------------------------------------------------------*/

void  get_entity_position(int, float[3]);
void  get_entity_x_vector(int, float[3]);
void  get_entity_y_vector(int, float[3]);
void  get_entity_z_vector(int, float[3]);
void  get_entity_scale   (int, float[3]);
float get_entity_alpha   (int);

/*---------------------------------------------------------------------------*/

int   get_entity_parent(int);
int   get_entity_child(int, int);

/*===========================================================================*/

void draw_entities(void);
void step_entities(void);
void init_entities(void);
void fini_entities(void);

/*===========================================================================*/

#endif
