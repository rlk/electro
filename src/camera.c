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
#ifdef SNIP

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

static void persp_camera(struct frustum *F, const float p[3], float T, float P,
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

    /* Find the view center.*/

    F->c[0] = p[0] + v[0] * n;
    F->c[1] = p[1] + v[1] * n;
    F->c[2] = p[2] + v[2] * n;

    /* Find the bottom left position. */

    A[0] = F->c[0] + l * F->r[0] + b * F->u[0];
    A[1] = F->c[1] + l * F->r[1] + b * F->u[1];
    A[2] = F->c[2] + l * F->r[2] + b * F->u[2];

    /* Find the bottom right position. */

    B[0] = F->c[0] + r * F->r[0] + b * F->u[0];
    B[1] = F->c[1] + r * F->r[1] + b * F->u[1];
    B[2] = F->c[2] + r * F->r[2] + b * F->u[2];

    /* Find the top right position. */

    C[0] = F->c[0] + r * F->r[0] + t * F->u[0];
    C[1] = F->c[1] + r * F->r[1] + t * F->u[1];
    C[2] = F->c[2] + r * F->r[2] + t * F->u[2];

    /* Find the top left position. */

    D[0] = F->c[0] + l * F->r[0] + t * F->u[0];
    D[1] = F->c[1] + l * F->r[1] + t * F->u[1];
    D[2] = F->c[2] + l * F->r[2] + t * F->u[2];

    /* Find the view frustum planes. */

    camera_plane(V +  0, pos, B, A);
    camera_plane(V +  4, pos, C, B);
    camera_plane(V +  8, pos, D, C);
    camera_plane(V + 12, pos, A, D);
}
#endif
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
        float T = PI * r[1] / 180.0f;
        float P = PI * r[0] / 180.0f;
        int   i = 0;

        /* Compute the camera position and orientation vectors. */

        p[0] += (float) (sin(T) * cos(P) * C[cd].dist);
        p[1] -= (float) (         sin(P) * C[cd].dist);
        p[2] += (float) (cos(T) * cos(P) * C[cd].dist);

        /* Supply the view position as a vertex program parameter. */
            
        if (GL_has_vertex_program)
            glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
                                       0, p[0], p[1], p[2], 1);

        /* Load projection and modelview matrices for each tile. */

        while ((i = draw_display(&F1, o, 1, 10000, i)))
        {
            glLoadIdentity();
            glTranslatef(0, 0, -C[cd].dist);

/*          transform_camera(id); */

            transform_entity(id, &F2, &F1);

            /* Render all children using this camera. */
            
            draw_entity_list(id, &F2, a * get_entity_alpha(id));
        }
    }
}

/*---------------------------------------------------------------------------*/

int send_create_camera(int type)
{
    int cd;

    if (C && (cd = alloc_camera()) >= 0)
    {
        pack_event(EVENT_CREATE_CAMERA);
        pack_index(cd);
        pack_index(type);

        C[cd].count = 1;
        C[cd].type  = type;
        C[cd].dist  = 0.0f;
        C[cd].zoom  = 1.0f;

        return send_create_entity(TYPE_CAMERA, cd);
    }
    return -1;
}

void recv_create_camera(void)
{
    int cd = unpack_index();

    C[cd].count = 1;
    C[cd].type  = unpack_index();
    C[cd].dist  = 0.0f;
    C[cd].zoom  = 1.0f;

    recv_create_entity();
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

    p[0] += (float) (cos(P) * sin(T) * C[cd].dist);
    p[1] -= (float) (sin(P)          * C[cd].dist);
    p[2] += (float) (cos(P) * cos(T) * C[cd].dist);

    v[0] = -(float) (cos(P) * sin(T));
    v[1] =  (float) (sin(P));
    v[2] = -(float) (cos(P) * cos(T));
}

/*---------------------------------------------------------------------------*/
