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
    int texture;

    float d[4];
    float a[3];
    float s[3];
    float e[3];
    float x[1];
};

struct object_vert
{
    float v[3];
    float n[3];
    float t[2];
};

struct object_face
{
    int vi[3];
};

struct object_surf
{
    int fc;
    struct object_face *fv;
};

struct object
{
    int mc;
    int vc;
    int sc;
    struct object_mtrl *mv;
    struct object_vert *vv;
    struct object_surf *sv;
};

/*---------------------------------------------------------------------------*/

int  object_load(const char *);
void object_free(int);
void object_draw(int);

/*---------------------------------------------------------------------------*/
