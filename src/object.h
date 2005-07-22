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

struct entity_func *startup_object(void);

/*---------------------------------------------------------------------------*/

int  create_mesh(int, int);
int  create_vert(int, float[3], float[3], float[2]);
int  create_face(int, int, int[3]);
int  create_edge(int, int, int[2]);

int  set_mesh(int, int, int);
int  set_vert(int, int, float[3], float[3], float[2]);
int  set_face(int, int, int, int[3]);
int  set_edge(int, int, int, int[2]);

int  get_mesh(int, int);
void get_vert(int, int, float[3], float[3], float[2]);
void get_face(int, int, int, int[3]);
void get_edge(int, int, int, int[2]);

int  get_mesh_count(int);
int  get_vert_count(int);
int  get_face_count(int, int);
int  get_edge_count(int, int);

void delete_mesh(int, int);
void delete_vert(int, int);
void delete_face(int, int, int);
void delete_edge(int, int, int);

/*---------------------------------------------------------------------------*/

int  send_create_object(const char *);
void recv_create_object(void);

int  send_create_object_elem(int, int);
void recv_create_object_elem(void);

int  send_delete_object_elem(int, int, int);
void recv_delete_object_elem(void);

int  send_modify_object_elem(int, int, int, int *, int, float *);
void recv_modify_object_elem(void);

/*---------------------------------------------------------------------------*/

#endif
