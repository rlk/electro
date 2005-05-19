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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "opengl.h"
#include "vector.h"
#include "matrix.h"
#include "buffer.h"
#include "entity.h"
#include "stereo.h"
#include "event.h"
#include "display.h"
#include "tracker.h"
#include "utility.h"
#include "camera.h"

/*---------------------------------------------------------------------------*/

struct camera
{
    int   count;
    int   type;
    int   mode;
    
    float eye_offset[3];
    float pos_offset[3];
    float view_basis[3][3];

    float near;
    float far;
};

static vector_t camera;

/*---------------------------------------------------------------------------*/

#define C(i) ((struct camera *) vecget(camera, i))

static int new_camera(void)
{
    int i, n = vecnum(camera);

    for (i = 0; i < n; ++i)
        if (C(i)->count == 0)
            return i;

    return vecadd(camera);
}

/*===========================================================================*/

int send_create_camera(int t)
{
    int i;

    if ((i = new_camera()) >= 0)
    {
        C(i)->count = 1;
        C(i)->type  = t;
        C(i)->near  = (t == CAMERA_ORTHO) ? -1000.0f :     0.1f;
        C(i)->far   = (t == CAMERA_ORTHO) ?  1000.0f : 10000.0f;

        pack_event(EVENT_CREATE_CAMERA);
        pack_index(t);
        pack_float(C(i)->near);
        pack_float(C(i)->far);

        return send_create_entity(TYPE_CAMERA, i);
    }
    return -1;
}

void recv_create_camera(void)
{
    int i = new_camera();
    int t = unpack_index();

    C(i)->count = 1;
    C(i)->type  = t;
    C(i)->near  = unpack_float();
    C(i)->far   = unpack_float();

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_camera_offset(int i, const float p[3], float e[3][3])
{
    pack_event(EVENT_SET_CAMERA_OFFSET);
    pack_index(i);

    pack_float((C(i)->pos_offset[0]    = p[0]));
    pack_float((C(i)->pos_offset[1]    = p[1]));
    pack_float((C(i)->pos_offset[2]    = p[2]));

    pack_float((C(i)->view_basis[0][0] = e[0][0]));
    pack_float((C(i)->view_basis[0][1] = e[0][1]));
    pack_float((C(i)->view_basis[0][2] = e[0][2]));

    pack_float((C(i)->view_basis[1][0] = e[1][0]));
    pack_float((C(i)->view_basis[1][1] = e[1][1]));
    pack_float((C(i)->view_basis[1][2] = e[1][2]));

    pack_float((C(i)->view_basis[2][0] = e[2][0]));
    pack_float((C(i)->view_basis[2][1] = e[2][1]));
    pack_float((C(i)->view_basis[2][2] = e[2][2]));
}

void recv_set_camera_offset(void)
{
    int i = unpack_index();

    C(i)->pos_offset[0]    = unpack_float();
    C(i)->pos_offset[1]    = unpack_float();
    C(i)->pos_offset[2]    = unpack_float();

    C(i)->view_basis[0][0] = unpack_float();
    C(i)->view_basis[0][1] = unpack_float();
    C(i)->view_basis[0][2] = unpack_float();

    C(i)->view_basis[1][0] = unpack_float();
    C(i)->view_basis[1][1] = unpack_float();
    C(i)->view_basis[1][2] = unpack_float();

    C(i)->view_basis[2][0] = unpack_float();
    C(i)->view_basis[2][1] = unpack_float();
    C(i)->view_basis[2][2] = unpack_float();
 }

/*---------------------------------------------------------------------------*/

void send_set_camera_stereo(int i, const float v[3], int mode)
{
    pack_event(EVENT_SET_CAMERA_STEREO);
    pack_index(i);

    pack_index((C(i)->mode          = mode));
    pack_float((C(i)->eye_offset[0] = v[0]));
    pack_float((C(i)->eye_offset[1] = v[1]));
    pack_float((C(i)->eye_offset[2] = v[2]));
}

void recv_set_camera_stereo(void)
{
    int i = unpack_index();

    C(i)->mode          = unpack_index();
    C(i)->eye_offset[0] = unpack_float();
    C(i)->eye_offset[1] = unpack_float();
    C(i)->eye_offset[2] = unpack_float();
}

/*===========================================================================*/

static int draw_tile(int i, const float d[3], struct frustum *F, int k)
{
    if (C(i)->type == CAMERA_PERSP)
        return draw_persp(F, d, C(i)->near, C(i)->far, k);

    if (C(i)->type == CAMERA_ORTHO)
        return draw_ortho(F,    C(i)->near, C(i)->far, k);

    return 0;
}

static void draw_eye(int j, int i, const float M[16],
                                   const float I[16],
                                   const struct frustum *F, float a, int e)
{
    struct frustum G;

    float N[16];
    float J[16];
    float d[3];

    int k = 0;

    /* Compute the world-space camera offset. */

    d[0] = (C(i)->eye_offset[0] * C(i)->view_basis[0][0] * e +
            C(i)->eye_offset[1] * C(i)->view_basis[1][0]     +
            C(i)->eye_offset[2] * C(i)->view_basis[2][0]) +C(i)->pos_offset[0];
    d[1] = (C(i)->eye_offset[0] * C(i)->view_basis[0][1] * e +
            C(i)->eye_offset[1] * C(i)->view_basis[1][1]     +
            C(i)->eye_offset[2] * C(i)->view_basis[2][1]) +C(i)->pos_offset[1];
    d[2] = (C(i)->eye_offset[0] * C(i)->view_basis[0][2] * e +
            C(i)->eye_offset[1] * C(i)->view_basis[1][2]     +
            C(i)->eye_offset[2] * C(i)->view_basis[2][2]) +C(i)->pos_offset[2];

    /* Iterate over all tiles. */

    while ((k = draw_tile(i, d, &G, k)))
    {
        G.p[0] = 0.0f;
        G.p[1] = 0.0f;
        G.p[2] = 0.0f;
        G.p[3] = 1.0f;

        transform_camera(j, N, M, J, I, d);

        /* Render all children using this camera. */

        enable_stereo(C(i)->mode, e);
        {
            draw_entity_tree(j, N, J, &G, a * get_entity_alpha(j));
        }
        disable_stereo(C(i)->mode);
    }
}

static void draw_camera(int j, int i, const float M[16],
                                      const float I[16],
                                      const struct frustum *F, float a)
{
    /* If drawing in stereo, render each eye separately. */

    if (C(i)->mode)
    {
        draw_eye(j, i, M, I, F, a, -1);
        draw_eye(j, i, M, I, F, a, +1);
    }
    else
        draw_eye(j, i, M, I, F, a,  0);
}

/*---------------------------------------------------------------------------*/

static void dupe_camera(int i)
{
    C(i)->count++;
}

static void free_camera(int i)
{
    if (--C(i)->count == 0)
        memset(C(i), 0, sizeof (struct camera));
}

/*===========================================================================*/

static struct entity_func camera_func = {
    "camera",
    NULL,
    NULL,
    draw_camera,
    dupe_camera,
    free_camera,
};

struct entity_func *startup_camera(void)
{
    if ((camera = vecnew(4, sizeof (struct camera))))
        return &camera_func;
    else
        return NULL;
}

