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
#include "video.h"
#include "vector.h"
#include "matrix.h"
#include "buffer.h"
#include "entity.h"
#include "stereo.h"
#include "event.h"
#include "image.h"
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
    
    float eye_offset[2][3];
    float pos_offset[3];
    float view_basis[3][3];

    float n;
    float f;
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
        C(i)->n     = (t == CAMERA_ORTHO) ? -1000.0f :     0.1f;
        C(i)->f     = (t == CAMERA_ORTHO) ?  1000.0f : 10000.0f;

        send_event(EVENT_CREATE_CAMERA);
        send_index(t);
        send_float(C(i)->n);
        send_float(C(i)->f);

        return send_create_entity(TYPE_CAMERA, i);
    }
    return -1;
}

void recv_create_camera(void)
{
    int i = new_camera();
    int t = recv_index();

    C(i)->count = 1;
    C(i)->type  = t;
    C(i)->n     = recv_float();
    C(i)->f     = recv_float();

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_camera_offset(int i, const float p[3], const float M[16])
{
    send_event(EVENT_SET_CAMERA_OFFSET);
    send_index(i);

    send_float((C(i)->pos_offset[0]    = p[0]));
    send_float((C(i)->pos_offset[1]    = p[1]));
    send_float((C(i)->pos_offset[2]    = p[2]));

    send_float((C(i)->view_basis[0][0] = M[0]));
    send_float((C(i)->view_basis[0][1] = M[1]));
    send_float((C(i)->view_basis[0][2] = M[2]));

    send_float((C(i)->view_basis[1][0] = M[4]));
    send_float((C(i)->view_basis[1][1] = M[5]));
    send_float((C(i)->view_basis[1][2] = M[6]));

    send_float((C(i)->view_basis[2][0] = M[8]));
    send_float((C(i)->view_basis[2][1] = M[9]));
    send_float((C(i)->view_basis[2][2] = M[10]));
}

void recv_set_camera_offset(void)
{
    int i = recv_index();

    C(i)->pos_offset[0]    = recv_float();
    C(i)->pos_offset[1]    = recv_float();
    C(i)->pos_offset[2]    = recv_float();

    C(i)->view_basis[0][0] = recv_float();
    C(i)->view_basis[0][1] = recv_float();
    C(i)->view_basis[0][2] = recv_float();

    C(i)->view_basis[1][0] = recv_float();
    C(i)->view_basis[1][1] = recv_float();
    C(i)->view_basis[1][2] = recv_float();

    C(i)->view_basis[2][0] = recv_float();
    C(i)->view_basis[2][1] = recv_float();
    C(i)->view_basis[2][2] = recv_float();
 }

/*---------------------------------------------------------------------------*/

void send_set_camera_stereo(int i, const float L[3],
                                   const float R[3], int mode)
{
    send_event(EVENT_SET_CAMERA_STEREO);
    send_index(i);

    send_index((C(i)->mode             = mode));
    send_float((C(i)->eye_offset[0][0] = L[0]));
    send_float((C(i)->eye_offset[0][1] = L[1]));
    send_float((C(i)->eye_offset[0][2] = L[2]));
    send_float((C(i)->eye_offset[1][0] = R[0]));
    send_float((C(i)->eye_offset[1][1] = R[1]));
    send_float((C(i)->eye_offset[1][2] = R[2]));
}

void recv_set_camera_stereo(void)
{
    int i = recv_index();

    C(i)->mode             = recv_index();
    C(i)->eye_offset[0][0] = recv_float();
    C(i)->eye_offset[0][1] = recv_float();
    C(i)->eye_offset[0][2] = recv_float();
    C(i)->eye_offset[1][0] = recv_float();
    C(i)->eye_offset[1][1] = recv_float();
    C(i)->eye_offset[1][2] = recv_float();
}

/*===========================================================================*/
/*
static void test_camera(int tile, int eye, int flag)
{
    float o[3];
    float r[3];
    float u[3];

    get_tile_o(tile, o);
    get_tile_r(tile, r);
    get_tile_u(tile, u);
    
    if (flag & DRAW_VARRIER_TEXGEN)
        set_texture_coordinates();

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    {
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LESS);

        draw_image(0);

        glBegin(GL_POLYGON);
        {
            if (eye == 0)
                glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
            else
                glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

            glVertex3f(o[0],
                       o[1],
                       o[2]);
            glVertex3f(o[0] + r[0],
                       o[1] + r[1],
                       o[2] + r[2]);
            glVertex3f(o[0] + r[0] + u[0],
                       o[1] + r[1] + u[1],
                       o[2] + r[2] + u[2]);
            glVertex3f(o[0] + u[0],
                       o[1] + u[1],
                       o[2] + u[2]);
        }
        glEnd();
    }
    glPopAttrib();
}
*/

static void test_camera(int eye, int flags)
{
    /* Map the tile onto the unit cube. */

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    /* Fill the tile. */

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    {
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LESS);

        if (flags & DRAW_VARRIER_TEXGEN)
            set_texture_coordinates();

        draw_image(0);
        
        glBegin(GL_QUADS);
        {
            if (eye == 0)
                glColor3f(0.0f, 1.0f, 0.0f);
            else
                glColor3f(0.0f, 0.0f, 1.0f);

            glVertex3f(0, 0, 0);
            glVertex3f(1, 0, 0);
            glVertex3f(1, 1, 0);
            glVertex3f(0, 1, 0);
        }
        glEnd();
    }
    glPopAttrib();

    /* Revert to the previous transformation. */

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


static int draw_tile(int i, int tile, const float d[3])
{
    if (C(i)->type == CAMERA_PERSP)
        return draw_persp(tile, C(i)->n, C(i)->f, d);

    if (C(i)->type == CAMERA_ORTHO)
        return draw_ortho(tile, C(i)->n, C(i)->f);

    return 0;
}

void draw_camera(int j, int i, int f, float a)
{
    struct camera *c = C(i);

    int flag = f | ((c->mode == STEREO_VARRIER_01) ? DRAW_VARRIER_TEXGEN : 0);
    int eye;

    /* Iterate over the eyes. */

    for (eye = 0; eye < (c->mode ? 2 : 1); ++eye)
    {
        int tile = 0;
        int next = 0;
        int pass = 0;

        float d[3];

        /* Compute the world-space eye position. */

        d[0] = c->pos_offset[0] + c->eye_offset[eye][0] * c->view_basis[0][0]
                                + c->eye_offset[eye][1] * c->view_basis[1][0]
                                + c->eye_offset[eye][2] * c->view_basis[2][0];
        d[1] = c->pos_offset[1] + c->eye_offset[eye][0] * c->view_basis[0][1]
                                + c->eye_offset[eye][1] * c->view_basis[1][1]
                                + c->eye_offset[eye][2] * c->view_basis[2][1];
        d[2] = c->pos_offset[2] + c->eye_offset[eye][0] * c->view_basis[0][2]
                                + c->eye_offset[eye][1] * c->view_basis[1][2]
                                + c->eye_offset[eye][2] * c->view_basis[2][2];

        /* Iterate over all tiles of this host. */

        while ((next = draw_tile(i, tile, d)))
        {
            pass = 0;

            glTranslatef(-d[0], -d[1], -d[2]);

            /* Iterate over all passes of this tile. */

            while ((pass = draw_pass(c->mode, eye, tile, pass, d)))
            {
                glPushMatrix();
                {
                    transform_camera(j);

                    if (get_tile_flags(tile) & TILE_TEST)
                        test_camera(eye, flag);
                    else
                        draw_entity_tree(j, flag, a * get_entity_alpha(j));
                }
                glPopMatrix();
            }

            tile = next;
        }
    }
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
    NULL,
    draw_camera,
    dupe_camera,
    free_camera,
};

struct entity_func *startup_camera(void)
{
    if ((camera = vecnew(MIN_CAMERAS, sizeof (struct camera))))
        return &camera_func;
    else
        return NULL;
}

