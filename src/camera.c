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
    {
        glTranslatef(-p[0], -p[1], -p[2]);
    }
}

int camera_init(void)
{
    if ((C = (struct camera *) calloc(CMAXINIT, sizeof (struct camera))))
    {
        C_max = CMAXINIT;
        return 1;
    }
    return 0;
}

void camera_draw(int id, int cd, float a)
{
    float p[3];
    float r[3];

    entity_get_position(id, p + 0, p + 1, p + 2);
    entity_get_rotation(id, r + 0, r + 1, r + 2);

    if (camera_exists(cd))
    {
        int viewport_x0 = viewport_get_x();
        int viewport_x1 = viewport_get_x() + viewport_get_w();
        int viewport_y0 = viewport_get_y();
        int viewport_y1 = viewport_get_y() + viewport_get_h();

        double T = PI * r[1] / 180.0;
        double P = PI * r[0] / 180.0;

        /* Compute the camera position given origin, rotation, and distance. */

        if (fabs(C[cd].dist) > 0.0f)
        {
            p[0] += (float) (sin(T) * cos(P) * C[cd].dist);
            p[1] -= (float) (         sin(P) * C[cd].dist);
            p[2] += (float) (cos(T) * cos(P) * C[cd].dist);
        }

        /* Load projection and modelview matrices for this camera. */

        glMatrixMode(GL_PROJECTION);
        {
            GLdouble l =  C[cd].zoom * viewport_x0;
            GLdouble r =  C[cd].zoom * viewport_x1;
            GLdouble b = -C[cd].zoom * viewport_y1;
            GLdouble t = -C[cd].zoom * viewport_y0;
            GLdouble f =  CAMERA_FAR;

            glLoadIdentity();

            if (C[cd].type == CAMERA_PERSP) glFrustum(l, r, b, t, 1.0, f);
            if (C[cd].type == CAMERA_ORTHO) glOrtho  (l, r, b, t,  -f, f);
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glLoadIdentity();
            glTranslatef(0, 0, -C[cd].dist);

            camera_transform(id);
        }

        opengl_check("camera_draw");

        /* Use the view configuration as vertex program parameters. */

        glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,
                                   0, p[0], p[1], p[2], 1);

        /* Render all children using this camera. */

        entity_traversal(id, a);
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
