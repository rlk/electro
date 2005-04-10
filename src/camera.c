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
#include "tracker.h"
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
    int cd = -1;

    C = (struct camera *) balloc(C, &cd, &C_max,
                                 sizeof (struct camera), camera_exists);
    return cd;
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

static void enable_stereo_quad(int eye)
{
    if (eye < 0)
        glDrawBuffer(GL_BACK_LEFT);
    else
        glDrawBuffer(GL_BACK_RIGHT);
}

static void disable_stereo_quad(void)
{
    glDrawBuffer(GL_BACK);
}

/*---------------------------------------------------------------------------*/

static void enable_stereo_red_blue(int eye)
{
    if (eye < 0)
    {
        glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    else
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
}

static void disable_stereo_red_blue(void)
{
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

/*---------------------------------------------------------------------------*/

static void enable_camera_stereo(int mode, int eye)
{
    switch (mode)
    {
    case CAMERA_STEREO_QUAD:     enable_stereo_quad(eye);     break;
    case CAMERA_STEREO_RED_BLUE: enable_stereo_red_blue(eye); break;
    }
}

static void disable_camera_stereo(int mode)
{
    switch (mode)
    {
    case CAMERA_STEREO_QUAD:     disable_stereo_quad();     break;
    case CAMERA_STEREO_RED_BLUE: disable_stereo_red_blue(); break;
    }
}

/*---------------------------------------------------------------------------*/

static int draw_tile(int cd, const float p[3], struct frustum *F, int i)
{
    if (C[cd].type == CAMERA_PERSP)
        return draw_persp(F, p, 1.0f, 10000.f, i);

    if (C[cd].type == CAMERA_ORTHO)
        return draw_ortho(F, -1000.f,  1000.f, i);

    return 0;
}

void draw_camera_eye(int id, int cd, const float M[16],
                                     const float I[16],
                                     const struct frustum *F, float a, int e)
{
    struct frustum G;

    float N[16];
    float J[16];

    float p[3], P[3];
    float d[3], D[3];
    float q[3], Q[3];

    int i = 0;

    /* Find the world-space camera position. */

    get_entity_position(id, p);

    d[0] = C[cd].offset[0] * e;
    d[1] = C[cd].offset[1];
    d[2] = C[cd].offset[2];
    d[3] = C[cd].offset[3];

    m_xfrm(P, M, p);
    m_pfrm(D, M, d);

    q[0] = p[0] + d[0];
    q[1] = p[1] + d[1];
    q[2] = p[2] + d[2];

    Q[0] = P[0] + D[0];
    Q[1] = P[1] + D[1];
    Q[2] = P[2] + D[2];

    while ((i = draw_tile(cd, q, &G, i)))
    {
        /* Supply the view position as a vertex program parameter. */

        if (GL_has_vertex_program)
            glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
                                       0, Q[0], Q[1], Q[2], 1);

        transform_entity(id, N, M, J, I);

        /* Render all children using this camera. */

        enable_camera_stereo(C[cd].mode, e);
        {
            draw_entity_list(id, N, J, &G, a * get_entity_alpha(id));
        }
        disable_camera_stereo(C[cd].mode);
    }
}

void draw_camera(int id, int cd, const float M[16],
                                 const float I[16],
                                 const struct frustum *F, float a)
{
    if (camera_exists(cd))
    {
        /* If drawing in stereo, render each eye separately. */

        if (C[cd].mode)
        {
            draw_camera_eye(id, cd, M, I, F, a, -1);
            draw_camera_eye(id, cd, M, I, F, a, +1);
        }
        else
            draw_camera_eye(id, cd, M, I, F, a,  0);
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

void send_set_camera_offset(int cd, const float v[3])
{
    pack_event(EVENT_SET_CAMERA_OFFSET);
    pack_index(cd);

    pack_float((C[cd].offset[0] = v[0]));
    pack_float((C[cd].offset[1] = v[1]));
    pack_float((C[cd].offset[2] = v[2]));
}

void recv_set_camera_offset(void)
{
    int cd = unpack_index();

    C[cd].offset[0] = unpack_float();
    C[cd].offset[1] = unpack_float();
    C[cd].offset[2] = unpack_float();
}

/*---------------------------------------------------------------------------*/

void send_set_camera_stereo(int cd, int mode)
{
    pack_event(EVENT_SET_CAMERA_STEREO);
    pack_index(cd);
    pack_index((C[cd].mode = mode));
}

void recv_set_camera_stereo(void)
{
    int cd     = unpack_index();
    C[cd].mode = unpack_index();
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
