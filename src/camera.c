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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "opengl.h"
#include "viewport.h"
#include "buffer.h"
#include "shared.h"
#include "entity.h"
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

static void camera_transform(int id)
{
    float p[3];
    float r[3];

    entity_get_position(id, p + 0, p + 1, p + 2);
    entity_get_rotation(id, r + 0, r + 1, r + 2);

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

static void camera_plane(float d[4], const float a[3],
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

    d[0] = x[1] * y[2] - x[2] * y[1];
    d[1] = x[2] * y[0] - x[0] * y[2];
    d[2] = x[0] * y[1] - x[1] * y[0];

    k = (float) sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);

    d[0] /= k;
    d[1] /= k;
    d[2] /= k;
    d[3]  = (d[0] * a[0] + d[1] * a[1] + d[2] * a[2]);
}

static void camera_persp(float V[4][4], float P[3],
                         float th, float ph,
                         float l,  float r,
                         float b,  float t)
{
    const float n = CAMERA_NEAR;

    float R[3], U[3], v[3], c[3];
    float A[3], B[3], C[3], D[3];

    /* Find the view vector. */

    v[0] = -(float) (cos(ph) * sin(th));
    v[1] =  (float) (sin(ph));
    v[2] = -(float) (cos(ph) * cos(th));

    /* Find the view right vector. */

    R[0] =  (float) cos(th);
    R[1] =  0;
    R[2] = -(float) sin(th);

    /* Find the view up vector. */
    
    U[0] = R[1] * v[2] - R[2] * v[1];
    U[1] = R[2] * v[0] - R[0] * v[2];
    U[2] = R[0] * v[1] - R[1] * v[0];

    /* Find the view center.*/

    c[0] = P[0] + v[0] * n;
    c[1] = P[1] + v[1] * n;
    c[2] = P[2] + v[2] * n;

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

    camera_plane(V[0], P, A, B);
    camera_plane(V[1], P, B, C);
    camera_plane(V[2], P, C, D);
    camera_plane(V[3], P, D, A);
}

static void camera_frustum(float P[3],
                         float th, float ph,
                         float l,  float r,
                         float b,  float t)
{
    const float n = CAMERA_NEAR;

    float R[3], U[3], v[3], c[3];
    float A[3], B[3], C[3], D[3];

    /* Find the view vector. */

    v[0] = -(float) (cos(ph) * sin(th));
    v[1] =  (float) (sin(ph));
    v[2] = -(float) (cos(ph) * cos(th));

    /* Find the view right vector. */

    R[0] =  (float) cos(th);
    R[1] =  0;
    R[2] = -(float) sin(th);

    /* Find the view up vector. */
    
    U[0] = R[1] * v[2] - R[2] * v[1];
    U[1] = R[2] * v[0] - R[0] * v[2];
    U[2] = R[0] * v[1] - R[1] * v[0];

    /* Find the view center.*/

    c[0] = P[0] + v[0] * n;
    c[1] = P[1] + v[1] * n;
    c[2] = P[2] + v[2] * n;

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

    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glLineWidth(8);

        glBegin(GL_LINES);
        {
            glColor3f(1.0f, 1.0f, 1.0f);

            glVertex3fv(A); glVertex3fv(B);
            glVertex3fv(B); glVertex3fv(C);
            glVertex3fv(C); glVertex3fv(D);
            glVertex3fv(D); glVertex3fv(A);

            glVertex3fv(P); glVertex3fv(A);
            glVertex3fv(P); glVertex3fv(B);
            glVertex3fv(P); glVertex3fv(C);
            glVertex3fv(P); glVertex3fv(D);
        }
        glEnd();
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

int camera_init(void)
{
    if ((C = (struct camera *) calloc(CMAXINIT, sizeof (struct camera))))
    {
        C_max = CMAXINIT;
        return 1;
    }
    return 0;
}

void camera_draw(int id, int cd, float P[3], float V[4][4])
{
    float Q[3], W[4][4];

    float pos[3];
    float rot[3];

    entity_get_position(id, pos + 0, pos + 1, pos + 2);
    entity_get_rotation(id, rot + 0, rot + 1, rot + 2);

    if (camera_exists(cd))
    {
        GLdouble l, r, b, t;

        int viewport_x0 = viewport_local_x();
        int viewport_x1 = viewport_local_w() + viewport_x0;
        int viewport_y0 = viewport_local_y();
        int viewport_y1 = viewport_local_h() + viewport_y0;
        int viewport_W  = viewport_total_w();

        double th = PI * rot[1] / 180.0;
        double ph = PI * rot[0] / 180.0;

        /* Compute the camera position and orientation vectors. */

        pos[0] += (float) (sin(th) * cos(ph) * C[cd].dist);
        pos[1] -= (float) (          sin(ph) * C[cd].dist);
        pos[2] += (float) (cos(th) * cos(ph) * C[cd].dist);

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

                glOrtho(l, r, b, t, -CAMERA_FAR, CAMERA_FAR);
            }
            if (C[cd].type == CAMERA_PERSP)
            {
                l =  C[cd].zoom * viewport_x0 / viewport_W;
                r =  C[cd].zoom * viewport_x1 / viewport_W;
                b = -C[cd].zoom * viewport_y1 / viewport_W;
                t = -C[cd].zoom * viewport_y0 / viewport_W;

                glFrustum(l, r, b, t, CAMERA_NEAR, CAMERA_FAR);

                camera_persp(W, pos, th, ph, l, r, b, t);
            }
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glLoadIdentity();
            glTranslatef(0, 0, -C[cd].dist);

            camera_transform(id);
        }

        /* Use the view configuration as vertex program parameters. */

        glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
                                   0, pos[0], pos[1], pos[2], 1);

        /* Render all children using this camera. */

        entity_traversal(id, pos, W);
    }
}

/*---------------------------------------------------------------------------*/

int camera_send_create(int type)
{
    int cd;

    if ((cd = buffer_unused(C_max, camera_exists)) >= 0)
    {
        pack_event(EVENT_CAMERA_CREATE);
        pack_index(cd);
        pack_index(type);

        C[cd].type = type;
        C[cd].dist = 0.0f;
        C[cd].zoom = 1.0f;

        return entity_send_create(TYPE_CAMERA, cd);
    }
    return -1;
}

void camera_recv_create(void)
{
    int cd = unpack_index();

    C[cd].type = unpack_index();
    C[cd].dist = 0.0f;
    C[cd].zoom = 1.0f;

    entity_recv_create();
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by the entity delete function. */

void camera_delete(int cd)
{
    memset(C + cd, 0, sizeof (struct camera));
}

/*---------------------------------------------------------------------------*/

void camera_send_dist(int cd, float d)
{
    pack_event(EVENT_CAMERA_DIST);
    pack_index(cd);

    pack_float((C[cd].dist = d));
}

void camera_recv_dist(void)
{
    int cd = unpack_index();

    C[cd].dist = unpack_float();
}

/*---------------------------------------------------------------------------*/

void camera_send_zoom(int cd, float z)
{
    pack_event(EVENT_CAMERA_ZOOM);
    pack_index(cd);

    pack_float((C[cd].zoom = z));
}

void camera_recv_zoom(void)
{
    int cd = unpack_index();

    C[cd].zoom = unpack_float();
}

/*---------------------------------------------------------------------------*/
