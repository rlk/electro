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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "opengl.h"
#include "stereo.h"
#include "matrix.h"
#include "display.h"
#include "video.h"

#define EXTENT 500

/*---------------------------------------------------------------------------*/

static void get_varrier_tile(int tile, float M[16],
                                       float c[3],
                                       float n[3], float *w, float *h)
{
    float o[3];
    float r[3];
    float u[3];

    /* Find the center and extent of the tile. */

    get_tile_o(tile, o);
    get_tile_r(tile, r);
    get_tile_u(tile, u);
    get_tile_n(tile, n);

    c[0] = o[0] + (r[0] + u[0]) / 2;
    c[1] = o[1] + (r[1] + u[1]) / 2;
    c[2] = o[2] + (r[2] + u[2]) / 2;

    *w = (float) sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
    *h = (float) sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);

    /* Compute the basis and transform for the tile coordinate system. */

    normalize(r);
    normalize(u);

    M[0] = r[0]; M[4] = u[0]; M[8]  = n[0]; M[12] = 0.0f;
    M[1] = r[1]; M[5] = u[1]; M[9]  = n[1]; M[13] = 0.0f;
    M[2] = r[2]; M[6] = u[2]; M[10] = n[2]; M[14] = 0.0f;
    M[3] = 0.0f; M[7] = 0.0f; M[11] = 0.0f; M[15] = 1.0f;
}

/*---------------------------------------------------------------------------*/

static char *vert_00 = \

    "uniform vec3 offset;                                        \n"\
    "varying vec3 L_phase;                                       \n"\
    "varying vec3 R_phase;                                       \n"\

    "void main()                                                 \n"\
    "{                                                           \n"\
    "    vec4 dr = vec4(offset.r, 0.0, 0.0, 0.0);                \n"\
    "    vec4 dg = vec4(offset.g, 0.0, 0.0, 0.0);                \n"\
    "    vec4 db = vec4(offset.b, 0.0, 0.0, 0.0);                \n"\

    "    L_phase.r = (gl_TextureMatrix[0] * (gl_Vertex + dr)).x; \n"\
    "    L_phase.g = (gl_TextureMatrix[0] * (gl_Vertex + dg)).x; \n"\
    "    L_phase.b = (gl_TextureMatrix[0] * (gl_Vertex + db)).x; \n"\
    "    R_phase.r = (gl_TextureMatrix[1] * (gl_Vertex + dr)).x; \n"\
    "    R_phase.g = (gl_TextureMatrix[1] * (gl_Vertex + dg)).x; \n"\
    "    R_phase.b = (gl_TextureMatrix[1] * (gl_Vertex + db)).x; \n"\

    "    gl_Position = ftransform();                             \n"\
    "}                                                           \n";

static char *frag_00 = \

    "uniform samplerRect L_map;                                      \n"\
    "uniform samplerRect R_map;                                      \n"\
    "uniform float       cycle;                                      \n"\
    "uniform vec2        scale;                                      \n"\

    "varying vec3 L_phase;                                           \n"\
    "varying vec3 R_phase;                                           \n"\

    "void main()                                                     \n"\
    "{                                                               \n"\
    "    const vec4 L = textureRect(L_map, gl_FragCoord.xy * scale); \n"\
    "    const vec4 R = textureRect(R_map, gl_FragCoord.xy * scale); \n"\

    "    vec3 Lk = step(vec3(cycle), fract(L_phase));                \n"\
    "    vec3 Rk = step(vec3(cycle), fract(R_phase));                \n"\

    "    gl_FragColor = vec4(max(L.rgb * Lk, R.rgb * Rk), 1.0);      \n"\
    "}                                                               \n";

static GLhandleARB vert_obj;
static GLhandleARB frag_obj;
static GLhandleARB prog_obj;

static GLuint frame_buf[2];
static GLuint color_buf[2];
static GLuint depth_buf[2];

static void push_quality(const float q[2])
{
    float v[4];

    /* Scale and shift the viewport in order to reduce rendering quality. */

    glGetFloatv(GL_VIEWPORT, v);

    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport((GLint) (v[0] * q[0]), (GLint) (v[1] * q[1]),
			   (GLint) (v[2] * q[0]), (GLint) (v[3] * q[1]));
}

static void pop_quality(void)
{
    /* Undo the previous viewport scale and shift. */

    glPopAttrib();
}

static void init_frame_buf(GLuint *frame,
                           GLuint *color,
                           GLuint *depth, int w, int h)
{
    /* Generate frame buffer and render buffer objects. */

    glGenFramebuffersEXT(1, frame);
    glGenTextures       (1, color);
    glGenTextures       (1, depth);

    /* Initialize the color render buffer. */

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, *color);
    glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA8, w, h, 0,
                  GL_RGBA, GL_INT, NULL);

    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Initialize the depth render buffer. */

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, *depth);
    glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT24, w, h, 0,
                  GL_DEPTH_COMPONENT, GL_INT, NULL);

    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_WRAP_T, GL_CLAMP);

    /* Initialize the frame buffer object. */

    opengl_push_framebuffer();
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, *frame);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                  GL_COLOR_ATTACHMENT0_EXT,
                                  GL_TEXTURE_RECTANGLE_ARB, *color, 0);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                  GL_DEPTH_ATTACHMENT_EXT,
                                  GL_TEXTURE_RECTANGLE_ARB, *depth, 0);
    }
    opengl_pop_framebuffer();

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
}

static void init_varrier_00(const float q[2])
{
    static int state = 0;

    if (state == 0)
    {
        int w = (int) (get_window_w() * q[0]);
        int h = (int) (get_window_h() * q[1]);

        /* Initialize a frame buffer object for each eye. */

        if (GL_has_texture_rectangle && GL_has_framebuffer_object)
        {
            init_frame_buf(frame_buf + 0, color_buf + 0, depth_buf + 0, w, h);
            init_frame_buf(frame_buf + 1, color_buf + 1, depth_buf + 1, w, h);
        }

        /* Initialize the Varrier algorithm fragment shader. */

        if (GL_has_shader_objects)
        {
            vert_obj = opengl_shader_object(GL_VERTEX_SHADER_ARB,   vert_00);
            frag_obj = opengl_shader_object(GL_FRAGMENT_SHADER_ARB, frag_00);
            prog_obj = opengl_program_object(vert_obj, frag_obj);
        }

        state = 1;
    }
}

static void set_line_transform(int tile, const float v[3], float *w, float *h)
{
    float p = get_varrier_pitch(tile);
    float a = get_varrier_angle(tile);
    float t = get_varrier_thick(tile);
    float s = get_varrier_shift(tile);

    float M[16];
    float C[3];
    float n[3];
    float u[3];
    float r[3];
    float e[3];
    float x, y, z;
    float nn, pp, ss;
    float dx, dy;

    get_varrier_tile(tile, M, C, n, w, h);
    get_tile_r(tile, r);
    get_tile_u(tile, u);

    /* Find the distance to the display. */

    nn = ((v[0] - C[0]) * n[0] +
          (v[1] - C[1]) * n[1] +
          (v[2] - C[2]) * n[2]);

    /* Compute the parallax offset due to optical thickness. */

    e[0] = v[0] - C[0];
    e[1] = v[1] - C[1];
    e[2] = v[2] - C[2];

    normalize(r);
    normalize(u);

    x = e[0] * r[0] + e[1] * r[1] + e[2] * r[2];
    y = e[0] * u[0] + e[1] * u[1] + e[2] * u[2];
    z = e[0] * n[0] + e[1] * n[1] + e[2] * n[2];

    dx = t * x / z;
    dy = t * y / z;

    /* Compute the pitch reduction due to optical thickness. */

    pp = p * (nn - t) / nn;
    ss = s;

    /* Transform the line screen texture into position. */

    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();

        glScalef(pp, pp, 1.0);               /* Pitch in feet.    */
        glRotatef(-a, 0, 0, 1);              /* Angle.            */
        glTranslatef(dx - ss, dy, 0);        /* Shift in feet.    */
    }
    glMatrixMode(GL_MODELVIEW);
}

static int stereo_varrier_00(int eye, int tile, int pass, float v[2][3])
{
    float q[2];

    get_tile_quality(tile, q);

    init_varrier_00(q);

    /* Render each eye's view to the appropriate off-screen frame buffer. */

    if (pass == 0)
    {
        if (GL_has_framebuffer_object)
        {
            opengl_push_framebuffer();
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buf[eye]);
        }

        push_quality(q);

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        return 1;
    }
    
    /* Combine both eye views in the on-screen buffer. */

    if (pass == 1)
    {
        pop_quality();
        opengl_pop_framebuffer();

        if (eye == 1)
        {
            float c = get_varrier_cycle(tile);
            float w;
            float h;

            /* Bind the eye view off-screen buffers as textures. */

            glActiveTextureARB(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, color_buf[1]);

            glActiveTextureARB(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, color_buf[0]);

            /* Enable the Varrier algorithm fragment shader. */

            if (GL_has_shader_objects)
            {
                glUseProgramObjectARB(prog_obj);

                glUniformLoc1i(prog_obj, "L_map", 0);
                glUniformLoc1i(prog_obj, "R_map", 1);
                glUniformLoc1f(prog_obj, "cycle", c);
                glUniformLoc2f(prog_obj, "scale", q[0], q[1]);
                glUniformLoc3f(prog_obj, "offset", -0.00027398f,
                                                    0.00000000f,
                                                   +0.00027398f);
            }

            /* Apply the linescreen transforms to the texture matrices. */

            glActiveTextureARB(GL_TEXTURE1);
            set_line_transform(tile, v[1], &w, &h);
            glActiveTextureARB(GL_TEXTURE0);
            set_line_transform(tile, v[0], &w, &h);

            /* Set up a transform mapping units to pixels. */

            glMatrixMode(GL_PROJECTION);
            {
                glPushMatrix();
                glLoadIdentity();
                glOrtho(-w / 2, +w / 2, -h / 2, +h / 2, -1, +1);
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glPushMatrix();
                glLoadIdentity();
            }

            /* Draw a screen-filling quad. */

            glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT | GL_ENABLE_BIT);
            {
                glDisable(GL_BLEND);

                glBegin(GL_POLYGON);
                {
                    glVertex2f(-w / 2, -h / 2);
                    glVertex2f(+w / 2, -h / 2);
                    glVertex2f(+w / 2, +h / 2);
                    glVertex2f(-w / 2, +h / 2);
                }
                glEnd();
            }
            glPopAttrib();

            /* Revert the state. */

            glMatrixMode(GL_PROJECTION);
            {
                glPopMatrix();
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glPopMatrix();
            }

            if (GL_has_shader_objects)
                glUseProgramObjectARB(0);

            glActiveTextureARB(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

            glActiveTextureARB(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static void lines(float k)
{
    int i;

    glBegin(GL_QUADS);
    {
        for (i = -EXTENT; i < EXTENT; ++i)
        {
            glVertex2f((float) i,     (float) -EXTENT);
            glVertex2f((float) i + k, (float) -EXTENT);
            glVertex2f((float) i + k, (float)  EXTENT);
            glVertex2f((float) i,     (float)  EXTENT);
        }
    }
    glEnd();
}

static void draw_varrier_lines_new(int tile, const float M[16],
                                             const float c[3],
                                             float w, float h, float d)
{
    float p = get_varrier_pitch(tile);
    float a = get_varrier_angle(tile);
    float t = get_varrier_thick(tile);
    float s = get_varrier_shift(tile) + d;
    float k = get_varrier_cycle(tile);

    glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glColor3f(0.0f, 0.0f, 0.0f);

        glDepthRange(0, 0);

        /* Transform the line screen into position. */

        glTranslatef(c[0], c[1], c[2]);
        glMultMatrixf(M);

        /* Apply the line screen configuration. */

        glRotatef(a, 0, 0, 1);
        glTranslatef(s, 0, t);
        glScalef(1 / p, 1 / p, 1 / p);

        /* Draw the line screen. */

        lines(k);
    }
    glPopMatrix();
    glPopAttrib();
}

static void draw_varrier_lines(int tile, const float M[16],
                                         const float C[3],
                                         float w, float h, float d)
{
    float p = get_varrier_pitch(tile);
    float a = get_varrier_angle(tile);
    float t = get_varrier_thick(tile);
    float s = get_varrier_shift(tile) + d;
    float c = get_varrier_cycle(tile);

    float f = 1 / p;
    int i;
    int n = (int) ceil(w / f);

    /* TODO: Compute out the proper range of the line screen. */

    int dx = (int) ceil(n);
    int dy = (int) ceil(h);

    glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glDepthRange(0, 0);

        /* Transform the line screen into position. */

        glTranslatef(C[0], C[1], C[2]);
        glMultMatrixf(M);

        /* Draw the line screen. */

        glTranslatef(s, 0, t);
        glRotatef(a, 0, 0, 1);

        glBegin(GL_QUADS);
        {
            glColor3f(0.0f, 0.0f, 0.0f);

            for (i = -n / 2 - dx; i < n / 2 + dx; ++i)
            {
                glVertex2f(f * i,         -h / 2 - dy);
                glVertex2f(f * i + f * c, -h / 2 - dy);
                glVertex2f(f * i + f * c,  h / 2 + dy);
                glVertex2f(f * i,          h / 2 + dy);
            }
        }
        glEnd();
    }
    glPopMatrix();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

#define LINESZ 256

static GLuint line_object[3] = { 0, 0, 0 };

static void init_line_texture(int tile, int chan)
{
    static const GLubyte color[3][4] = {
        { 0x00, 0xFF, 0xFF, 0x00 },
        { 0xFF, 0x00, 0xFF, 0x00 },
        { 0xFF, 0xFF, 0x00, 0x00 },
    };

    GLubyte *p;

    /* If the requested texture object already exists, bind it. */

    if (glIsTexture(line_object[chan]))
        glBindTexture(GL_TEXTURE_2D, line_object[chan]);

    else if ((p = (GLubyte *) calloc(LINESZ * 2, 4)))
    {
        float c = get_varrier_cycle(tile);
        int i;
        int j;

        /* Generate a new texture object */

        glGenTextures(1, line_object + chan);
        glBindTexture(GL_TEXTURE_2D, line_object[chan]);

        /* Fill it with the line screen pattern for the given channel. */

        memset(p, 0xFF, LINESZ * 8);

        for (i = 0; i < LINESZ * c - 1; ++i)
            for (j = 0; j < 2; ++j)
            {
                p[(j * LINESZ + i) * 4 + 0] = color[chan][0];
                p[(j * LINESZ + i) * 4 + 1] = color[chan][1];
                p[(j * LINESZ + i) * 4 + 2] = color[chan][2];
                p[(j * LINESZ + i) * 4 + 3] = color[chan][3];
            }

        /* Configure the texture and specify the pixel buffer. */

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, LINESZ, 2, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, p);
        free(p);
    }
}

static void move_line_texture(int tile, const float v[3], float px)
{
    float p = get_varrier_pitch(tile);
    float a = get_varrier_angle(tile);
    float t = get_varrier_thick(tile);
    float s = get_varrier_shift(tile);

    float M[16];
    float C[3];
    float n[3];
    float u[3];
    float r[3];
    float e[3];
    float w, h;
    float x, y, z;
    float nn, pp, ss;
    float dx, dy;

    get_varrier_tile(tile, M, C, n, &w, &h);
    get_tile_r(tile, r);
    get_tile_u(tile, u);

    /* Find the distance to the display. */

    nn = ((v[0] - C[0]) * n[0] +
          (v[1] - C[1]) * n[1] +
          (v[2] - C[2]) * n[2]);

    /* Compute the parallax offset due to optical thickness. */

    e[0] = v[0] - C[0];
    e[1] = v[1] - C[1];
    e[2] = v[2] - C[2];

    normalize(r);
    normalize(u);

    x = e[0] * r[0] + e[1] * r[1] + e[2] * r[2];
    y = e[0] * u[0] + e[1] * u[1] + e[2] * u[2];
    z = e[0] * n[0] + e[1] * n[1] + e[2] * n[2];

    dx = t * x / z;
    dy = t * y / z;

    /* Compute the pitch reduction due to optical thickness. */

    pp = p * (nn - t) / nn;
    ss = s;

    /* Transform the line screen texture into position. */

    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();

        glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0,
                                   1.0f / 800.0f, 1.0f / 600.0f, 0.0f, 0.0f);

        glScalef(pp, pp, 1.0);               /* Pitch in feet.    */
        glTranslatef(-ss + dx + px, dy, 0);  /* Shift in feet.    */
        glRotatef(-a, 0, 0, 1);              /* Angle.            */
        glScalef(0.5f * w, 0.5f * h, 1.0f);  /* Scale to feet.    */
    }
    glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------------*/

static int stereo_varrier_01(int eye, int tile, int pass, const float v[3])
{
    float px = 0.00025f;

    if (pass == 0)
    {
        if (eye == 0)
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        else
            glClear(GL_DEPTH_BUFFER_BIT);

        /* Set up the line screen texture environments. */

        if (GL_has_multitexture && GL_TEXTURE3_ARB < GL_max_multitexture)
        {
            /* TU0 modulates the material RGB against the base texture,      */
            /* giving the pixel RGB, and sums (and clamps) the red and       */
            /* green line screen alpha values.                               */

            glActiveTextureARB(GL_TEXTURE0_ARB);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,      GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,      GL_TEXTURE0);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,    GL_TEXTURE1);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,    GL_TEXTURE2);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_ADD);

            /* TU1 modulates the pixel RGB against the red line screen and   */
            /* sums (and clamps) the blue linescreen alpha value with the    */
            /* red and green alpha values.                                   */

            glActiveTextureARB(GL_TEXTURE1_ARB);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,      GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,      GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,    GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,    GL_TEXTURE3);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_ADD);
            init_line_texture(tile, 0);
            move_line_texture(tile, v, +px);

            /* TU2 modulates the pixel color against the green line screen   */
            /* and modulates the accumulated line screen alpha against the   */
            /* material alpha value.                                         */

            glActiveTextureARB(GL_TEXTURE2_ARB);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,      GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,      GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,    GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,    GL_PRIMARY_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_MODULATE);
            init_line_texture(tile, 1);
            move_line_texture(tile, v, 0);

            /* TU3 modulates the pixel color against the blue line screen    */
            /* and modulates the accumulated line screen alpha against the   */
            /* base texture alpha value.                                     */

            glActiveTextureARB(GL_TEXTURE3_ARB);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,      GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,      GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,    GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,    GL_TEXTURE0);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_MODULATE);
            init_line_texture(tile, 2);
            move_line_texture(tile, v, -px);

            glActiveTextureARB(GL_TEXTURE0_ARB);
        }

        draw_tile_background(tile, DRAW_VARRIER_TEXGEN);

        return 1;
    }
    else
    {
        if (GL_has_multitexture)
        {
            if (GL_TEXTURE3_ARB < GL_max_multitexture)
            {
                glActiveTextureARB(GL_TEXTURE3_ARB);
                glDisable(GL_TEXTURE_2D);
            }
            if (GL_TEXTURE2_ARB < GL_max_multitexture)
            {
                glActiveTextureARB(GL_TEXTURE2_ARB);
                glDisable(GL_TEXTURE_2D);
            }
            if (GL_TEXTURE1_ARB < GL_max_multitexture)
            {
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glDisable(GL_TEXTURE_2D);
            }
            glActiveTextureARB(GL_TEXTURE0_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
    }
    return 0;
}

static int stereo_varrier_11(int eye, int tile, int pass)
{
    if (pass == 0)
    {
        float M[16];
        float c[3];
        float n[3];
        float w, h;

        get_varrier_tile(tile, M, c, n, &w, &h);

        /* Draw the line screen into the depth buffer. */

        if (eye == 0)
        {
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            glColorMask(0, 0, 0, 0);
            draw_varrier_lines_new(tile, M, c, w, h, 0);
            glColorMask(1, 1, 1, 1);
            draw_tile_background(tile, 0);
        }
        else
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(0, 0, 0, 0);
            draw_varrier_lines_new(tile, M, c, w, h, 0);
            glColorMask(1, 1, 1, 1);
            draw_tile_background(tile, 0);
        }

        return 1;
    }
    return 0;
}

static int stereo_varrier_33(int eye, int tile, int pass)
{
    float M[16];
    float c[3];
    float n[3];
    float w, h, d = 0.00025f;

    int next = 0;

    get_varrier_tile(tile, M, c, n, &w, &h);

    switch (pass)
    {
    case 0:
        if (eye == 0)
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        else
            glClear(GL_DEPTH_BUFFER_BIT);

        glColorMask(0, 0, 0, 0);
        draw_varrier_lines(tile, M, c, w, h, +d);
        glColorMask(1, 0, 0, 0);
        draw_tile_background(tile, 0);
        next = 1;
        break;
        
    case 1:
        glClear(GL_DEPTH_BUFFER_BIT);
        glColorMask(0, 0, 0, 0);
        draw_varrier_lines(tile, M, c, w, h,  0);
        glColorMask(0, 1, 0, 0);
        draw_tile_background(tile, 0);
        next = 2;
        break;
        
    case 2:
        glClear(GL_DEPTH_BUFFER_BIT);
        glColorMask(0, 0, 0, 0);
        draw_varrier_lines(tile, M, c, w, h, -d);
        glColorMask(0, 0, 1, 0);
        draw_tile_background(tile, 0);
        next = 3;
        break;
        
    case 3:
        glColorMask(1, 1, 1, 1);
        next = 0;
    }

    return next;
}

/*---------------------------------------------------------------------------*/

static int stereo_none(int pass)
{
    if (pass == 0)
        return 1;
    else
        return 0;
}

static int stereo_quad(int eye, int tile, int pass)
{
    if (pass == 0)
    {
        if (quad_stereo_status())
        {
            if (eye == 0)
                glDrawBuffer(GL_BACK_LEFT);
            else
                glDrawBuffer(GL_BACK_RIGHT);
        }

        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);
        draw_tile_background(tile, 0);

        return 1;
    }
    else
        return 0;
}

static int stereo_red_blue(int eye, int tile, int pass)
{
    if (pass == 0)
    {
        if (eye == 0)
        {
            glClear(GL_COLOR_BUFFER_BIT |
                    GL_DEPTH_BUFFER_BIT);
            draw_tile_background(tile, 0);
            glColorMask(1, 0, 0, 0);
        }
        else
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(0, 0, 1, 0);
        }
        return 1;
    }
    else
    {
        glColorMask(1, 1, 1, 1);
        return 0;
    }
}

/*---------------------------------------------------------------------------*/

int draw_pass(int mode, int eye, int tile, int pass, float v[2][3])
{
    /* If stereo rendering is enabled, handle it. */

    switch (mode)
    {
    case STEREO_NONE:       return stereo_none(pass);
    case STEREO_TILE:       return stereo_none(pass);

    case STEREO_QUAD:       return stereo_quad      (eye, tile, pass);
    case STEREO_RED_BLUE:   return stereo_red_blue  (eye, tile, pass);
    case STEREO_VARRIER_00: return stereo_varrier_00(eye, tile, pass, v);
    case STEREO_VARRIER_01: return stereo_varrier_01(eye, tile, pass, v[eye]);
    case STEREO_VARRIER_11: return stereo_varrier_11(eye, tile, pass);
    case STEREO_VARRIER_33: return stereo_varrier_33(eye, tile, pass);
    }

    return 0;
}

