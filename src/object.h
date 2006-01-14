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

int  get_mesh(int, int);
void get_vert(int, int, float[3], float[3], float[2]);
void get_face(int, int, int, int[3]);
void get_edge(int, int, int, int[2]);

int  get_mesh_count(int);
int  get_vert_count(int);
int  get_face_count(int, int);
int  get_edge_count(int, int);

/*---------------------------------------------------------------------------*/

int  send_create_object(const char *);
void recv_create_object(void);

/*---------------------------------------------------------------------------*/

int  send_create_mesh(int);
int  send_create_vert(int);
int  send_create_face(int, int);
int  send_create_edge(int, int);

void send_delete_mesh(int, int);
void send_delete_vert(int, int);
void send_delete_face(int, int, int);
void send_delete_edge(int, int, int);

void send_set_mesh(int, int, int);
void send_set_vert(int, int, float[3], float[3], float[2]);
void send_set_face(int, int, int, int[3]);
void send_set_edge(int, int, int, int[2]);

/*---------------------------------------------------------------------------*/

void recv_create_mesh(void);
void recv_create_vert(void);
void recv_create_face(void);
void recv_create_edge(void);

void recv_delete_mesh(void);
void recv_delete_vert(void);
void recv_delete_face(void);
void recv_delete_edge(void);

void recv_set_mesh(void);
void recv_set_vert(void);
void recv_set_face(void);
void recv_set_edge(void);

/*---------------------------------------------------------------------------*/

#endif
