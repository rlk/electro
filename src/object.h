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

#include "opengl.h"
#include "frustum.h"

/*---------------------------------------------------------------------------*/

#define MAXOBJECT 128

struct object_mtrl
{
    const char *name;
    int image;

    float d[4];
    float a[4];
    float s[4];
    float e[4];
    float x[1];
};

struct object_vert
{
    float t[2];
    float n[3];
    float v[3];
};

struct object_face
{
    int vi[3];
};

struct object_edge
{
    int vi[2];
};

struct object_surf
{
    int mi;
    int fc;
    int ec;
    struct object_face *fv;
    struct object_edge *ev;
};

struct object
{
    int count;
    int state;

    GLuint buffer;

    int vc;
    int mc;
    int sc;
    struct object_vert *vv;
    struct object_mtrl *mv;
    struct object_surf *sv;
};

/*---------------------------------------------------------------------------*/

int  init_object(void);
void draw_object(int, int, const float[16],
                           const float[16], const struct frustum *, float);

void init_object_gl(int);
void free_object_gl(int);

int  send_create_object(const char *);
void recv_create_object(void);

void clone_object(int);
void delete_object(int);

/*---------------------------------------------------------------------------*/

#endif
