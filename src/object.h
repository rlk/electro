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

#ifndef OBJECT_H
#define OBJECT_H

/*---------------------------------------------------------------------------*/

#define FLAG_MTRL_TRNS 0x0001
#define FLAG_MTRL_DIFF 0x0002
#define FLAG_MTRL_AMBT 0x0004
#define FLAG_MTRL_SPEC 0x0008
#define FLAG_MTRL_EMSV 0x0010
#define FLAG_MTRL_SHIN 0x0020

/* Default material properties */

#define MTRL_DIFF_R 0.8f
#define MTRL_DIFF_G 0.8f
#define MTRL_DIFF_B 0.8f
#define MTRL_DIFF_A 1.0f

#define MTRL_AMBT_R 0.2f
#define MTRL_AMBT_G 0.2f
#define MTRL_AMBT_B 0.2f
#define MTRL_AMBT_A 1.0f

#define MTRL_SPEC_R 0.0f
#define MTRL_SPEC_G 0.0f
#define MTRL_SPEC_B 0.0f
#define MTRL_SPEC_A 1.0f

#define MTRL_EMSV_R 0.0f
#define MTRL_EMSV_G 0.0f
#define MTRL_EMSV_B 0.0f
#define MTRL_EMSV_A 1.0f

#define MTRL_SHIN_K 0.0f

/*---------------------------------------------------------------------------*/

struct entity_func *startup_object(void);

/*---------------------------------------------------------------------------*/

int  send_create_object(const char *);
void recv_create_object(void);

/*---------------------------------------------------------------------------*/

int  create_object_mtrl(int, const char *,
                             const float[4],
                             const float[4],
                             const float[4],
                             const float[4], float);
int  create_object_vert(int, const float[3],
                             const float[3],
                             const float[3]);
int  create_object_face(int, int, int, int, int);
int  create_object_edge(int, int, int, int);
int  create_object_surf(int, int);

/*---------------------------------------------------------------------------*/

void delete_object_mtrl(int, int);
void delete_object_vert(int, int);
void delete_object_surf(int, int);
void delete_object_face(int, int, int);
void delete_object_edge(int, int, int);
void delete_object_surf(int, int);

/*---------------------------------------------------------------------------*/

#endif
