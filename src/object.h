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

/*---------------------------------------------------------------------------*/

#define MAXOBJECT 128

struct object_mtrl
{
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

struct object_surf
{
    int mi;
    int fc;
    struct object_face *fv;
};

struct object
{
    int vc;
    int mc;
    int sc;
    struct object_vert *vv;
    struct object_mtrl *mv;
    struct object_surf *sv;
};

/*---------------------------------------------------------------------------*/

int  object_init(void);
void object_draw(int, int, float);

int  object_send_create(const char *);
void object_recv_create(void);

void object_delete(int);

/*---------------------------------------------------------------------------*/
