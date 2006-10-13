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

static void init_camera(int);
static void fini_camera(int);

/*===========================================================================*/

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

    /* Offscreen rendering attributes. */

    int    state;
    GLuint frame;
    GLuint depth;
    int    image;
    
    float l;
    float r;
    float b;
    float t;
};

static vector_t camera;

static float    camera_rot[16];
static float    camera_pos[3];
static int      camera_eye = 0;

/*---------------------------------------------------------------------------*/

static struct camera *get_camera(int i)
{
    return (struct camera *) vecget(camera, i);
}

static int new_camera(void)
{
    int i, n = vecnum(camera);

    for (i = 0; i < n; ++i)
        if (get_camera(i)->count == 0)
            return i;

    return vecadd(camera);
}

/*---------------------------------------------------------------------------*/

int get_camera_eye(void)
{
    return camera_eye;
}

void get_camera_pos(float p[3])
{
    p[0] = camera_pos[0];
    p[1] = camera_pos[1];
    p[2] = camera_pos[2];
}

void get_camera_rot(float M[16])
{
    memcpy(M, camera_rot, 16 * sizeof (float));
}

/*===========================================================================*/

int send_create_camera(int t)
{
    int i;

    if ((i = new_camera()) >= 0)
    {
        struct camera *c = get_camera(i);

        c->count = 1;
        c->type  = t;
        c->n     = (t == CAMERA_ORTHO) ? -1000.0f :    0.1f;
        c->f     = (t == CAMERA_ORTHO) ?  1000.0f : 1000.0f;

        c->frame = 0;
        c->depth = 0;
        c->image = 0;

        c->view_basis[0][0] = 1.0f;
        c->view_basis[0][1] = 0.0f;
        c->view_basis[0][2] = 0.0f;
        c->view_basis[1][0] = 0.0f;
        c->view_basis[1][1] = 1.0f;
        c->view_basis[1][2] = 0.0f;
        c->view_basis[2][0] = 0.0f;
        c->view_basis[2][1] = 0.0f;
        c->view_basis[2][2] = 1.0f;

        send_event(EVENT_CREATE_CAMERA);
        send_index(t);
        send_float(c->n);
        send_float(c->f);

        return send_create_entity(TYPE_CAMERA, i);
    }
    return -1;
}

void recv_create_camera(void)
{
    int i = new_camera();
    int t = recv_index();

    struct camera *c = get_camera(i);

    c->count = 1;
    c->type  = t;
    c->n     = recv_float();
    c->f     = recv_float();

    c->frame = 0;
    c->depth = 0;
    c->image = 0;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void get_camera_vector(int j, int i, float v[3], int x, int y)
{
    struct camera *c = get_camera(i);

    float X[3];
    float Y[3];
    float Z[3];
    float V[3];

    /* Get the point vector in camera coordinates. */

    if (c->type == CAMERA_PERSP)
        get_display_point(V, c->pos_offset, x, y);
    else
    {
        V[0] = (float) x;
        V[1] = (float) y;
        V[2] = -1.0;
    }

    /* Transform this vector to world coordinates. */

    get_entity_x_vector(j, X);
    get_entity_y_vector(j, Y);
    get_entity_z_vector(j, Z);

    v[0] = V[0] * X[0] + V[1] * Y[0] + V[2] * Z[0];
    v[1] = V[0] * X[1] + V[1] * Y[1] + V[2] * Z[1];
    v[2] = V[0] * X[2] + V[1] * Y[2] + V[2] * Z[2];
}

/*---------------------------------------------------------------------------*/

void send_set_camera_offset(int i, const float p[3], const float M[16])
{
    struct camera *c = get_camera(i);

    send_event(EVENT_SET_CAMERA_OFFSET);
    send_index(i);

    send_float((c->pos_offset[0]    = p[0]));
    send_float((c->pos_offset[1]    = p[1]));
    send_float((c->pos_offset[2]    = p[2]));

    send_float((c->view_basis[0][0] = M[0]));
    send_float((c->view_basis[0][1] = M[1]));
    send_float((c->view_basis[0][2] = M[2]));

    send_float((c->view_basis[1][0] = M[4]));
    send_float((c->view_basis[1][1] = M[5]));
    send_float((c->view_basis[1][2] = M[6]));

    send_float((c->view_basis[2][0] = M[8]));
    send_float((c->view_basis[2][1] = M[9]));
    send_float((c->view_basis[2][2] = M[10]));
}

void recv_set_camera_offset(void)
{
    struct camera *c = get_camera(recv_index());

    c->pos_offset[0]    = recv_float();
    c->pos_offset[1]    = recv_float();
    c->pos_offset[2]    = recv_float();

    c->view_basis[0][0] = recv_float();
    c->view_basis[0][1] = recv_float();
    c->view_basis[0][2] = recv_float();

    c->view_basis[1][0] = recv_float();
    c->view_basis[1][1] = recv_float();
    c->view_basis[1][2] = recv_float();

    c->view_basis[2][0] = recv_float();
    c->view_basis[2][1] = recv_float();
    c->view_basis[2][2] = recv_float();
 }

/*---------------------------------------------------------------------------*/

void send_set_camera_stereo(int i, const float L[3],
                                   const float R[3], int mode)
{
    struct camera *c = get_camera(i);

    send_event(EVENT_SET_CAMERA_STEREO);
    send_index(i);

    send_index((c->mode             = mode));
    send_float((c->eye_offset[0][0] = L[0]));
    send_float((c->eye_offset[0][1] = L[1]));
    send_float((c->eye_offset[0][2] = L[2]));
    send_float((c->eye_offset[1][0] = R[0]));
    send_float((c->eye_offset[1][1] = R[1]));
    send_float((c->eye_offset[1][2] = R[2]));
}

void recv_set_camera_stereo(void)
{
    struct camera *c = get_camera(recv_index());

    c->mode             = recv_index();
    c->eye_offset[0][0] = recv_float();
    c->eye_offset[0][1] = recv_float();
    c->eye_offset[0][2] = recv_float();
    c->eye_offset[1][0] = recv_float();
    c->eye_offset[1][1] = recv_float();
    c->eye_offset[1][2] = recv_float();
}

/*---------------------------------------------------------------------------*/

void send_set_camera_range(int i, float n, float f)
{
    struct camera *c = get_camera(i);

    send_event(EVENT_SET_CAMERA_RANGE);
    send_index(i);
    send_float((c->n = n));
    send_float((c->f = f));
}

void recv_set_camera_range(void)
{
    struct camera *c = get_camera(recv_index());

    c->n = recv_float();
    c->f = recv_float();
}

/*---------------------------------------------------------------------------*/

void send_set_camera_image(int i, int j, float l, float r, float b, float t)
{
    struct camera *c = get_camera(i);

    send_event(EVENT_SET_CAMERA_IMAGE);
    send_index(i);
    send_index((c->image = j));
    send_float((c->l     = l));
    send_float((c->r     = r));
    send_float((c->b     = b));
    send_float((c->t     = t));
}

void recv_set_camera_image(void)
{
    struct camera *c = get_camera(recv_index());

    c->image = recv_index();
    c->l     = recv_float();
    c->r     = recv_float();
    c->b     = recv_float();
    c->t     = recv_float();
}

/*===========================================================================*/

static void test_color(int eye)
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

        draw_image(0);
        
        glBegin(GL_QUADS);
        {
            if (eye == 0)
                glColor3f(0.0f, 1.0f, 0.0f);
            else
                glColor3f(0.0f, 0.0f, 1.0f);

            glVertex2i(0, 0);
            glVertex2i(1, 0);
            glVertex2i(1, 1);
            glVertex2i(0, 1);
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

static void test_ghost(int eye)
{
    /* Map the tile onto the unit cube. */

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    /* Draw the bars. */

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    {
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LESS);

        draw_image(0);
        
        glTranslatef(0.5f, 0.5f, 0.0f);

        if (eye == 0)
            glRotatef(-45.0f, 0.0f, 0.0f, 1.0f);
        else
            glRotatef(+45.0f, 0.0f, 0.0f, 1.0f);

        glScalef(1.4142135f / 16.0f,
                 1.4142135f / 16.0f, 1.0f);

        glBegin(GL_QUADS);
        {
            int i, n = 16;

            glColor3f(1.0f, 1.0f, 1.0f);

            for (i = -n; i <= n; i += 4)
            {
                glVertex2i(i - 1, -n);
                glVertex2i(i + 1, -n);
                glVertex2i(i + 1, +n);
                glVertex2i(i - 1, +n);
            }
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

/*---------------------------------------------------------------------------*/

static int draw_tile(struct camera *c, int eye, int tile, const float d[3])
{
    if (c->type == CAMERA_PERSP)
        return draw_persp(tile, c->n, c->f, eye, d);

    if (c->type == CAMERA_ORTHO)
        return draw_ortho(tile, c->n, c->f);

    return 0;
}

static void get_eye_pos(float d[3], struct camera *c, int eye)
{
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
}

/*---------------------------------------------------------------------------*/

static void draw_color(int j, int eye)
{
    glPushMatrix();
    {
        transform_camera(j);
        test_color(eye);
    }
    glPopMatrix();
}

static void draw_ghost(int j, int eye)
{
    glPushMatrix();
    {
        transform_camera(j);
        test_ghost(eye);
    }
    glPopMatrix();
}

static void draw_scene(int j, int flag, float a)
{
    glPushMatrix();
    {
        float M[16];

        /* Apply the view matrix. */

        transform_camera(j);

        /* Save the inverse view rotation for env map use. */

        glGetFloatv(GL_MODELVIEW_MATRIX, M);

        load_inv(camera_rot, M);

        camera_rot[12] = 0;
        camera_rot[13] = 0;
        camera_rot[14] = 0;

        get_entity_position(j, camera_pos);

        /* Draw the scene. */

        draw_entity_tree(j, flag, a * get_entity_alpha(j));
    }
    glPopMatrix();
}

static void draw_camera(int i, int j, int f, float a)
{
    struct camera *c = get_camera(i);

    init_camera(i);

    if (c->frame)
    {
        /* Apply a basic projection for offscreen rendering. */

        glMatrixMode(GL_PROJECTION);
        {
            glPushMatrix();
            glLoadIdentity();

            if (c->type == CAMERA_ORTHO)
                glOrtho  (c->l, c->r, c->b, c->t, c->n, c->f);
            else
                glFrustum(c->l, c->r, c->b, c->t, c->n, c->f);
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPushMatrix();
            glLoadIdentity();
        }

        /* Render the scene to the offscreen buffer. */

        glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
        {
            int w = get_image_w(c->image);
            int h = get_image_h(c->image);

            glViewport(0, 0, w, h);
            glScissor (0, 0, w, h);

            opengl_push_framebuffer(c->frame);
            {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                draw_scene(j, f, a);
            }
            opengl_pop_framebuffer();
        }
        glPopAttrib();

        /* Revert the projection. */

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    else
    {
        int eye;
        int tile;
        int pass;

        /* Iterate over all tiles of this host. */

        for (tile = 0; tile < get_tile_count(); ++tile)
        {
            float d[2][3];

            /* Iterate over the eyes. */

            get_eye_pos(d[0], c, 0);
            get_eye_pos(d[1], c, 1);

            for (eye = 0; eye < (c->mode ? 2 : 1); ++eye)
            {
                camera_eye = eye;
                
                if (draw_tile(c, eye, tile, d[eye]))
                {
                    pass = 0;

                    /* Iterate over all passes of this eye and tile. */
                    
                    while ((pass = draw_pass(c->mode, eye, tile, pass, d)))
                    {
                        if      (get_tile_flags(tile) & TILE_TEST_COLOR)
                            draw_color(j, eye);
                        else if (get_tile_flags(tile) & TILE_TEST_GHOST)
                            draw_ghost(j, eye);
                        else
                            draw_scene(j, f, a);
                    }
                }
            }
        }

        /* HACK */

        if (c->mode != STEREO_VARRIER_00)
            opengl_set_fence();
    }
}

/*---------------------------------------------------------------------------*/

static void init_camera(int i)
{
    struct camera *c = get_camera(i);

    if (c->state == 0 && c->image)
    {
        /* The camera needs an offscreen render target.  Initialize it. */

        GLenum T = get_image_target(c->image);
        GLuint O = get_image_buffer(c->image);

        int w = get_image_w(c->image);
        int h = get_image_h(c->image);

        if (GL_has_framebuffer_object)
        {
            init_image(c->image);

            glGenFramebuffersEXT(1, &c->frame);
            glGenTextures       (1, &c->depth);

            /* Initialize the depth render target. */

            glBindTexture(T, c->depth);
            glTexImage2D(T, 0, GL_DEPTH_COMPONENT24, w, h, 0,
                         GL_DEPTH_COMPONENT, GL_INT, NULL);

            glTexParameteri(T, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(T, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(T, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(T, GL_TEXTURE_WRAP_T, GL_CLAMP);

            glTexParameteri(T, GL_DEPTH_TEXTURE_MODE_ARB,
                               GL_INTENSITY);
            glTexParameteri(T, GL_TEXTURE_COMPARE_MODE_ARB,
                               GL_COMPARE_R_TO_TEXTURE_ARB);

            /* Attach the framebuffer render targets. */

            opengl_push_framebuffer(c->frame);
            {
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                          GL_COLOR_ATTACHMENT0_EXT, T, O, 0);
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                          GL_DEPTH_ATTACHMENT_EXT, T,
                                          c->depth, 0);
            }
            opengl_pop_framebuffer();
        }
    }

    c->state = 1;
}

static void fini_camera(int i)
{
    struct camera *c = get_camera(i);

    if (c->state == 1)
    {
        if (c->frame && GL_has_framebuffer_object)
            glDeleteFramebuffersEXT(1, &c->frame);

        if (c->depth)
            glDeleteTextures(1, &c->depth);

        c->frame = 0;
        c->depth = 0;
        c->state = 0;
    }
}

/*---------------------------------------------------------------------------*/

static void dupe_camera(int i)
{
    get_camera(i)->count++;
}

static void free_camera(int i)
{
    struct camera *c = get_camera(i);

    if (c->count > 0)
    {
        c->count--;
        
        if (c->count == 0)
            memset(c, 0, sizeof (struct camera));
    }
}

/*===========================================================================*/

static struct entity_func camera_func = {
    "camera",
    init_camera,
    fini_camera,
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

