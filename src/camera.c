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
#include "matrix.h"
#include "buffer.h"
#include "entity.h"
#include "event.h"
#include "display.h"
#include "utility.h"
#include "camera.h"

/*---------------------------------------------------------------------------*/

#define CMAXINIT 8

static struct camera *C;
static int            C_max;

static int camera_exists(int cd)
{
    return (C && 0 <= cd && cd < C_max && C[cd].count);
}

static int alloc_camera(void)
{
    return balloc((void **) &C, &C_max, sizeof (struct camera), camera_exists);
}

/*---------------------------------------------------------------------------*/

int init_camera(void)
{
    if ((C = (struct camera *) calloc(CMAXINIT, sizeof (struct camera))))
    {
        C_max = CMAXINIT;
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

int draw_tile(int cd, struct frustum *F1, const float p[3], int i)
{
    if (C[cd].type == CAMERA_PERSP) return draw_persp(F1, p, 1.f, 10000.f, i);
    if (C[cd].type == CAMERA_ORTHO) return draw_ortho(F1, p, -1000.f, 1000.f, i);

    return 0;
}

void draw_camera(int id, int cd, const struct frustum *F0, float a)
{
    struct frustum F1;
    struct frustum F2;

    float p[3];
    float r[3];
    float o[3] = { 0, 0, 0 };

    get_entity_position(id, p + 0, p + 1, p + 2);
    get_entity_rotation(id, r + 0, r + 1, r + 2);

    if (camera_exists(cd))
    {
        int i = 0;

        /* Supply the view position as a vertex program parameter. */
            
        if (GL_has_vertex_program)
            glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
                                       0, p[0], p[1], p[2], 1);

        /* Load projection and modelview matrices for each tile. */

        while ((i = draw_tile(cd, &F1, o, i)))
        {
            glLoadIdentity();
            transform_entity(id, &F2, &F1);

            /* Render all children using this camera. */
            
            draw_entity_list(id, &F2, a * get_entity_alpha(id));
        }
    }
}

/*---------------------------------------------------------------------------*/

int send_create_camera(int t)
{
    int cd;

    if (C && (cd = alloc_camera()) >= 0)
    {
        pack_event(EVENT_CREATE_CAMERA);
        pack_index(cd);
        pack_index(t);

        C[cd].count = 1;
        C[cd].type  = t;

        return send_create_entity(TYPE_CAMERA, cd);
    }
    return -1;
}

void recv_create_camera(void)
{
    int cd = unpack_index();
    int t  = unpack_index();

    C[cd].count = 1;
    C[cd].type  = t;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/
/* These may only be called by create_clone and delete_entity, respectively. */

void clone_camera(int cd)
{
    if (camera_exists(cd))
        C[cd].count++;
}

void delete_camera(int cd)
{
    if (camera_exists(cd))
    {
        C[cd].count--;

        if (C[cd].count == 0)
            memset(C + cd, 0, sizeof (struct camera));
    }
}

/*---------------------------------------------------------------------------*/

void get_camera_direction(int id, int cd, float p[3], float v[3])
{
    float T, P, r[3];

    get_entity_position(id, p + 0, p + 1, p + 2);
    get_entity_rotation(id, r + 0, r + 1, r + 2);

    T = PI * r[1] / 180.0f;
    P = PI * r[0] / 180.0f;

    v[0] = -(float) (cos(P) * sin(T));
    v[1] =  (float) (sin(P));
    v[2] = -(float) (cos(P) * cos(T));
}

/*---------------------------------------------------------------------------*/
