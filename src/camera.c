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
#include "viewport.h"
#include "buffer.h"
#include "entity.h"
#include "event.h"
#include "camera.h"

/*---------------------------------------------------------------------------*/

#define CMAXINIT 8

static struct camera *C;
static int            C_max;

static int camera_exists(int cd)
{
    return (C && 0 <= cd && cd < C_max && C[cd].type);
}

/*---------------------------------------------------------------------------*/

static void transform_camera(int id)
{
    float p[3];
    float r[3];

    get_entity_position(id, p + 0, p + 1, p + 2);
    get_entity_rotation(id, r + 0, r + 1, r + 2);

    /* Rotation. */

    if (fabs(r[0]) > 0.0)
        glRotatef(-r[0], 1.0f, 0.0f, 0.0f);

    if (fabs(r[1]) > 0.0)
        glRotatef(-r[1], 0.0f, 1.0f, 0.0f);

    if (fabs(r[2]) > 0.0)
        glRotatef(-r[2], 0.0f, 0.0f, 1.0f);

    /* Translation. */

    if (fabs(p[0]) > 0.0 ||
        fabs(p[1]) > 0.0 ||
        fabs(p[2]) > 0.0)
        glTranslatef(-p[0], -p[1], -p[2]);
}

/*---------------------------------------------------------------------------*/

static void camera_plane(float plane[4], const float a[3],
                                         const float b[3],
                                         const float c[3])
{
    float x[3];
    float y[3];
    float k;
    
    x[0] = b[0] - a[0];
    x[1] = b[1] - a[1];
    x[2] = b[2] - a[2];

    y[0] = c[0] - a[0];
    y[1] = c[1] - a[1];
    y[2] = c[2] - a[2];

    plane[0] = x[1] * y[2] - x[2] * y[1];
    plane[1] = x[2] * y[0] - x[0] * y[2];
    plane[2] = x[0] * y[1] - x[1] * y[0];

    k = (float) sqrt(plane[0] * plane[0] +
                     plane[1] * plane[1] +
                     plane[2] * plane[2]);

    plane[0] /= k;
    plane[1] /= k;
    plane[2] /= k;
    plane[3]  = (plane[0] * a[0] + plane[1] * a[1] + plane[2] * a[2]);
}

/*---------------------------------------------------------------------------*/

static void ortho_camera(float V[16], const float pos[3], float T, float P,
                                                          float l, float r,
                                                          float b, float t)
{
    float R[3], U[3], v[3];

    /* Find the view vector. */

    v[0] = -(float) (cos(P) * sin(T));
    v[1] =  (float) (sin(P));
    v[2] = -(float) (cos(P) * cos(T));

    /* Find the view right vector. */

    R[0] =  (float) cos(T);
    R[1] =  0;
    R[2] = -(float) sin(T);

    /* Find the view up vector. */
    
    U[0] = R[1] * v[2] - R[2] * v[1];
    U[1] = R[2] * v[0] - R[0] * v[2];
    U[2] = R[0] * v[1] - R[1] * v[0];

    /* Find the bottom plane. */

    V[0] =  U[0];
    V[1] =  U[1];
    V[2] =  U[2];
    V[3] =  V[0] * pos[0] + V[1] * pos[1] + V[2] * pos[2] + b;

    /* Find the top plane. */

    V[4] = -U[0];
    V[5] = -U[1];
    V[6] = -U[2];
    V[7] =  V[4] * pos[0] + V[5] * pos[1] + V[6] * pos[2] - t;

    /* Find the left plane. */

    V[ 8] =  R[0];
    V[ 9] =  R[1];
    V[10] =  R[2];
    V[11] =  V[8] * pos[0] + V[9] * pos[1] + V[10] * pos[2] + l;

    /* Find the right plane. */

    V[12] = -R[0];
    V[13] = -R[1];
    V[14] = -R[2];
    V[15] =  V[12] * pos[0] + V[13] * pos[1] + V[14] * pos[2] - r;
}

static void persp_camera(float V[16], const float pos[3], float T, float P,
                                                          float l, float r,
                                                          float b, float t)
{
    const float n = CAMERA_NEAR;

    float R[3], U[3], v[3], c[3];
    float A[3], B[3], C[3], D[3];

    /* Find the view vector. */

    v[0] = -(float) (cos(P) * sin(T));
    v[1] =  (float) (sin(P));
    v[2] = -(float) (cos(P) * cos(T));

    /* Find the view right vector. */

    R[0] =  (float) cos(T);
    R[1] =  0;
    R[2] = -(float) sin(T);

    /* Find the view up vector. */
    
    U[0] = R[1] * v[2] - R[2] * v[1];
    U[1] = R[2] * v[0] - R[0] * v[2];
    U[2] = R[0] * v[1] - R[1] * v[0];

    /* Find the view center.*/

    c[0] = pos[0] + v[0] * n;
    c[1] = pos[1] + v[1] * n;
    c[2] = pos[2] + v[2] * n;

    /* Find the bottom left position. */

    A[0] = c[0] + l * R[0] + b * U[0];
    A[1] = c[1] + l * R[1] + b * U[1];
    A[2] = c[2] + l * R[2] + b * U[2];

    /* Find the bottom right position. */

    B[0] = c[0] + r * R[0] + b * U[0];
    B[1] = c[1] + r * R[1] + b * U[1];
    B[2] = c[2] + r * R[2] + b * U[2];

    /* Find the top right position. */

    C[0] = c[0] + r * R[0] + t * U[0];
    C[1] = c[1] + r * R[1] + t * U[1];
    C[2] = c[2] + r * R[2] + t * U[2];

    /* Find the top left position. */

    D[0] = c[0] + l * R[0] + t * U[0];
    D[1] = c[1] + l * R[1] + t * U[1];
    D[2] = c[2] + l * R[2] + t * U[2];

    /* Find the view frustum planes. */

    camera_plane(V +  0, pos, B, A);
    camera_plane(V +  4, pos, C, B);
    camera_plane(V +  8, pos, D, C);
    camera_plane(V + 12, pos, A, D);
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

void draw_camera(int id, int cd, const float V[16], float a)
{
    float W[16];

    float pos[3];
    float rot[3];

    get_entity_position(id, pos + 0, pos + 1, pos + 2);
    get_entity_rotation(id, rot + 0, rot + 1, rot + 2);

    if (camera_exists(cd))
    {
        GLdouble l, r, b, t;

        int viewport_x0 = get_local_viewport_x();
        int viewport_x1 = get_local_viewport_w() + viewport_x0;
        int viewport_y0 = get_local_viewport_y();
        int viewport_y1 = get_local_viewport_h() + viewport_y0;
        int viewport_W  = get_total_viewport_w();

        double T = PI * rot[1] / 180.0;
        double P = PI * rot[0] / 180.0;

        /* Compute the camera position and orientation vectors. */

        pos[0] += (float) (sin(T) * cos(P) * C[cd].dist);
        pos[1] -= (float) (         sin(P) * C[cd].dist);
        pos[2] += (float) (cos(T) * cos(P) * C[cd].dist);

        /* Load projection and modelview matrices for this camera. */

        glMatrixMode(GL_PROJECTION);
        {
            glLoadIdentity();

            if (C[cd].type == CAMERA_ORTHO)
            {
                l =  C[cd].zoom * viewport_x0;
                r =  C[cd].zoom * viewport_x1;
                b = -C[cd].zoom * viewport_y1;
                t = -C[cd].zoom * viewport_y0;

                ortho_camera(W, pos, T, P, l, r, b, t);

                glOrtho(l, r, b, t, -CAMERA_FAR, CAMERA_FAR);
            }
            if (C[cd].type == CAMERA_PERSP)
            {
                l =  C[cd].zoom * viewport_x0 / viewport_W;
                r =  C[cd].zoom * viewport_x1 / viewport_W;
                b = -C[cd].zoom * viewport_y1 / viewport_W;
                t = -C[cd].zoom * viewport_y0 / viewport_W;

                persp_camera(W, pos, T, P, l, r, b, t);

                glFrustum(l, r, b, t, CAMERA_NEAR, CAMERA_FAR);
            }
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glLoadIdentity();
            glTranslatef(0, 0, -C[cd].dist);

            transform_camera(id);
        }

        /* Use the view configuration as vertex program parameters. */

        glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0,
                                   pos[0], pos[1], pos[2], 1);

        /* Render all children using this camera. */

        draw_entity_list(id, W, a * get_entity_alpha(id));
    }
}

/*---------------------------------------------------------------------------*/

int send_create_camera(int type)
{
    int cd;

    if ((cd = buffer_unused(C_max, camera_exists)) >= 0)
    {
        pack_event(EVENT_CREATE_CAMERA);
        pack_index(cd);
        pack_index(type);

        C[cd].type = type;
        C[cd].dist = 0.0f;
        C[cd].zoom = 1.0f;

        return send_create_entity(TYPE_CAMERA, cd);
    }
    return -1;
}

void recv_create_camera(void)
{
    int cd = unpack_index();

    C[cd].type = unpack_index();
    C[cd].dist = 0.0f;
    C[cd].zoom = 1.0f;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by the entity delete function. */

void delete_camera(int cd)
{
    memset(C + cd, 0, sizeof (struct camera));
}

/*---------------------------------------------------------------------------*/

void send_set_camera_distance(int cd, float d)
{
    pack_event(EVENT_SET_CAMERA_DISTANCE);
    pack_index(cd);

    pack_float((C[cd].dist = d));
}

void recv_set_camera_distance(void)
{
    int cd = unpack_index();

    C[cd].dist = unpack_float();
}

/*---------------------------------------------------------------------------*/

void send_set_camera_zoom(int cd, float z)
{
    pack_event(EVENT_SET_CAMERA_ZOOM);
    pack_index(cd);

    pack_float((C[cd].zoom = z));
}

void recv_set_camera_zoom(void)
{
    int cd = unpack_index();

    C[cd].zoom = unpack_float();
}

/*---------------------------------------------------------------------------*/
