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

#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "opengl.h"
#include "matrix.h"
#include "utility.h"
#include "console.h"

#ifdef __linux__
#include <GL/glx.h>
#endif

/*---------------------------------------------------------------------------*/

GLboolean GL_has_fence                = GL_FALSE;
GLboolean GL_has_fragment_program     = GL_FALSE;
GLboolean GL_has_vertex_program       = GL_FALSE;
GLboolean GL_has_vertex_buffer_object = GL_FALSE;
GLboolean GL_has_framebuffer_object   = GL_FALSE;
GLboolean GL_has_point_sprite         = GL_FALSE;
GLboolean GL_has_texture_rectangle    = GL_FALSE;
GLboolean GL_has_texture_compression  = GL_FALSE;
GLboolean GL_has_shader_objects       = GL_FALSE;
GLboolean GL_has_multitexture         = GL_FALSE;
GLenum    GL_max_multitexture         = 0;

/*---------------------------------------------------------------------------*/

/* Confirm that the named OpenGL extension is supported by the current       */
/* implementation.                                                           */

GLboolean opengl_need(const char *extension)
{
    const char *string = (const char *) glGetString(GL_EXTENSIONS);
    const char *start  = string;

    char *where;
    char *space;

    while (start)
    {
        if ((where = strstr(start, extension)) == NULL)
            return 0;

        space = where + strlen(extension);

        if (where == start || *(where - 1) == ' ')
            if (*space == ' ' || *space == '\0')
                return GL_TRUE;

        start = space;
    }

    return GL_FALSE;
}

/* Acquire a pointer to the named OpenGL function.  Print an error if this   */
/* function is not supported by the current implementation.  Note that SDL's */
/* SDL_GL_GetProcAddress function doesn't work correctly on older Linux      */
/* systems, so we call GLX directly.                                         */

void *opengl_proc(const char *name)
{
#ifdef __linux__
    void *p = glXGetProcAddressARB((const GLubyte *) name);
#else
    void *p = SDL_GL_GetProcAddress(name);
#endif

    if (p == NULL)
        error("OpenGL procedure '%s' not found", name);

    return p;
}

/*===========================================================================*/

#define SEGMENTS 24

static GLuint box_list;
static GLuint cyl_list;
static GLuint cap_list;
static GLuint grd_list;
static GLuint xyz_list;

void opengl_draw_xyz(float x, float y, float z)
{
    glPushMatrix();
    {
        glTranslatef(x, y, z);
        glCallList(xyz_list);
    }
    glPopMatrix();
}

void opengl_draw_vec(float px, float py, float pz,
                     float qx, float qy, float qz)
{
    glBegin(GL_LINES);
    {
        glVertex3f(px, py, pz);
        glVertex3f(qx, qy, qz);
    }
    glEnd();
}

void opengl_draw_grd(float a, float b, float c, float d)
{
    const float da = (float) fabs(a);
    const float db = (float) fabs(b);
    const float dc = (float) fabs(c);

    float x[3] = { 1, 0, 0 };
    float y[3] = { 0, 1, 0 };
    float z[3];
    float M[16];

    /* Compute a well-defined basis for the plane's space. */

    if (da > db && da > dc)
    {
        x[0] = 0;
        x[1] = 0;
        x[2] = 1;
    }
    if (db > da && db > dc)
    {
        y[0] = 0;
        y[1] = 0;
        y[2] = 1;
    }

    z[0] = a;
    z[1] = b;
    z[2] = c;

    cross(x, y, z);
    cross(y, z, x);

    normalize(x);
    normalize(y);

    /* Use this basis as plane transformation. */

    M[0] = x[0]; M[4] = y[0]; M[8]  = z[0]; M[12] = 0;
    M[1] = x[1]; M[5] = y[1]; M[9]  = z[1]; M[13] = 0;
    M[2] = x[2]; M[6] = y[2]; M[10] = z[2]; M[14] = 0;
    M[3] =    0; M[7] =    0; M[11] =    0; M[15] = 1;

    glPushMatrix();
    {
        glMultMatrixf(M);
        glTranslatef(0, 0, d);
        glCallList(grd_list);
    }
    glPopMatrix();
    
}

void opengl_draw_box(float x, float y, float z)
{
    glPushMatrix();
    {
        glScalef(x, y, z);
        glCallList(box_list);
    }
    glPopMatrix();
}

void opengl_draw_cap(float r, float l)
{
    glPushMatrix();
    {
        glTranslatef(0, 0, +l / 2);
        glScalef(+r, +r, +r);
        glCallList(cap_list);
    }
    glPopMatrix();

    glPushMatrix();
    {
        glScalef(+r, +r, +l);
        glCallList(cyl_list);
    }
    glPopMatrix();

    glPushMatrix();
    {
        glTranslatef(0, 0, -l / 2);
        glScalef(+r, +r, -r);
        glCallList(cap_list);
    }
    glPopMatrix();
}

void opengl_draw_sph(float r)
{
    glPushMatrix();
    {
        glScalef(+r, +r, +r);
        glCallList(cap_list);
    }
    glPopMatrix();

    glPushMatrix();
    {
        glScalef(+r, +r, -r);
        glCallList(cap_list);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

static void init_opengl_box(void)
{
    const float x = 0.5f;
    const float y = 0.5f;
    const float z = 0.5f;

    glBegin(GL_LINES);
    {
        glVertex3f(-x, -y, -z);  glVertex3f(-x, -y, +z);
        glVertex3f(-x, +y, -z);  glVertex3f(-x, +y, +z);
        glVertex3f(+x, -y, -z);  glVertex3f(+x, -y, +z);
        glVertex3f(+x, +y, -z);  glVertex3f(+x, +y, +z);

        glVertex3f(-x, -y, -z);  glVertex3f(-x, +y, -z);
        glVertex3f(-x, -y, +z);  glVertex3f(-x, +y, +z);
        glVertex3f(+x, -y, -z);  glVertex3f(+x, +y, -z);
        glVertex3f(+x, -y, +z);  glVertex3f(+x, +y, +z);

        glVertex3f(-x, -y, -z);  glVertex3f(+x, -y, -z);
        glVertex3f(-x, -y, +z);  glVertex3f(+x, -y, +z);
        glVertex3f(-x, +y, -z);  glVertex3f(+x, +y, -z);
        glVertex3f(-x, +y, +z);  glVertex3f(+x, +y, +z);
    }
    glEnd();
}

static void init_opengl_cyl(void)
{
    int d = 360 / SEGMENTS;
    int i;

    glBegin(GL_LINES);
    {
        for (i = 0; i < 360; i += d)
        {
            float x = (float) cos(RAD(i));
            float y = (float) sin(RAD(i));

            glVertex3f(x, y, -0.5f);
            glVertex3f(x, y, +0.5f);
        }
    }
    glEnd();
}

static void init_opengl_cap(void)
{
    int d = 360 / SEGMENTS;
    int i;
    int j;

    /* Longitude */

    for (i = 0; i < 360; i += d)
    {
        glBegin(GL_LINE_STRIP);
        {
            for (j = 0; j <= 90; j += d)
            {
                float x = (float) (cos(RAD(i)) * cos(RAD(j)));
                float y = (float) (sin(RAD(i)) * cos(RAD(j)));
                float z = (float) (              sin(RAD(j)));

                glVertex3f(x, y, z);
            }
        }
        glEnd();
    }

    /* Latitude */

    for (j = 0; j < 90; j += d)
    {
        glBegin(GL_LINE_LOOP);
        {
            for (i = 0; i < 360; i += d)
            {
                float x = (float) (cos(RAD(i)) * cos(RAD(j)));
                float y = (float) (sin(RAD(i)) * cos(RAD(j)));
                float z = (float) (              sin(RAD(j)));

                glVertex3f(x, y, z);
            }
        }
        glEnd();
    }
}

static void init_opengl_grd(void)
{
    int d = SEGMENTS / 2;
    int i;

    glBegin(GL_LINES);
    {
        for (i = -d; i <= d; ++i)
        {
            glVertex2i(-d, i);
            glVertex2i(+d, i);
            glVertex2i(i, -d);
            glVertex2i(i, +d);
        }
    }
    glEnd();
}

static void init_opengl_xyz(void)
{
    float d = 1.0f / 8.0f;

    glBegin(GL_LINES);
    {
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0, 0, 0);
        glVertex3f(d, 0, 0);

        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0, 0, 0);
        glVertex3f(0, d, 0);

        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, d);
    }
    glEnd();
}

/*---------------------------------------------------------------------------*/

static void init_opengl_obj(void)
{
    box_list = glGenLists(1);
    cyl_list = glGenLists(1);
    cap_list = glGenLists(1);
    grd_list = glGenLists(1);
    xyz_list = glGenLists(1);

    glNewList(box_list, GL_COMPILE);
    init_opengl_box();
    glEndList();

    glNewList(cyl_list, GL_COMPILE);
    init_opengl_cyl();
    glEndList();

    glNewList(cap_list, GL_COMPILE);
    init_opengl_cap();
    glEndList();

    glNewList(grd_list, GL_COMPILE);
    init_opengl_grd();
    glEndList();

    glNewList(xyz_list, GL_COMPILE);
    init_opengl_xyz();
    glEndList();
}

static void fini_opengl_obj(void)
{
    if (xyz_list && glIsList(xyz_list)) glDeleteLists(xyz_list, 1);
    if (grd_list && glIsList(grd_list)) glDeleteLists(grd_list, 1);
    if (cap_list && glIsList(cap_list)) glDeleteLists(cap_list, 1);
    if (cyl_list && glIsList(cyl_list)) glDeleteLists(cyl_list, 1);
    if (box_list && glIsList(box_list)) glDeleteLists(box_list, 1);

    box_list = 0;
    cyl_list = 0;
    cap_list = 0;
    grd_list = 0;
    xyz_list = 0;
}

/*===========================================================================*/

static GLuint fence = 0;

#ifdef __APPLE__

void init_opengl(void)
{
    GLint TUs;

    GL_has_fragment_program     = opengl_need("GL_ARB_fragment_program");
    GL_has_vertex_program       = opengl_need("GL_ARB_vertex_program");
    GL_has_vertex_buffer_object = opengl_need("GL_ARB_vertex_buffer_object");
    GL_has_framebuffer_object   = opengl_need("GL_EXT_framebuffer_object");
    GL_has_point_sprite         = opengl_need("GL_ARB_point_sprite");
    GL_has_texture_compression  = opengl_need("GL_ARB_texture_compression");
    GL_has_texture_rectangle    = opengl_need("GL_ARB_texture_rectangle");
    GL_has_multitexture         = opengl_need("GL_ARB_multitexture");
    GL_has_shader_objects       = opengl_need("GL_ARB_shader_objects");

    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &TUs);
    GL_max_multitexture = GL_TEXTURE0_ARB +  TUs;

    init_opengl_obj();
}

#else

PFNGLGENFENCESNVPROC                 glGenFencesNV;
PFNGLSETFENCENVPROC                  glSetFenceNV;
PFNGLFINISHFENCENVPROC               glFinishFenceNV;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArrayARB;
PFNGLBINDATTRIBLOCATIONARBPROC       glBindAttribLocationARB;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB;
PFNGLPROGRAMENVPARAMETER4FARBPROC    glProgramEnvParameter4fARB;
PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointerARB;
PFNGLPROGRAMSTRINGARBPROC            glProgramStringARB;
PFNGLBINDPROGRAMARBPROC              glBindProgramARB;
PFNGLGENPROGRAMSARBPROC              glGenProgramsARB;
PFNGLISPROGRAMARBPROC                glIsProgramARB;
PFNGLDELETEPROGRAMSARBPROC           glDeleteProgramsARB;
PFNGLBINDBUFFERARBPROC               glBindBufferARB;
PFNGLGENBUFFERSARBPROC               glGenBuffersARB;
PFNGLBUFFERDATAARBPROC               glBufferDataARB;
PFNGLMAPBUFFERARBPROC                glMapBufferARB;
PFNGLUNMAPBUFFERARBPROC              glUnmapBufferARB;
PFNGLISBUFFERARBPROC                 glIsBufferARB;
PFNGLDELETEBUFFERSARBPROC            glDeleteBuffersARB;
PFNGLACTIVETEXTUREARBPROC            glActiveTextureARB;
PFNGLUSEPROGRAMOBJECTARBPROC         glUseProgramObjectARB;
PFNGLCREATESHADEROBJECTARBPROC       glCreateShaderObjectARB;
PFNGLCREATEPROGRAMOBJECTARBPROC      glCreateProgramObjectARB;
PFNGLSHADERSOURCEARBPROC             glShaderSourceARB;
PFNGLCOMPILESHADERARBPROC            glCompileShaderARB;
PFNGLATTACHOBJECTARBPROC             glAttachObjectARB;
PFNGLLINKPROGRAMARBPROC              glLinkProgramARB;
PFNGLGETOBJECTPARAMETERIVARBPROC     glGetObjectParameterivARB;
PFNGLGETINFOLOGARBPROC               glGetInfoLogARB;
PFNGLDELETEOBJECTARBPROC             glDeleteObjectARB;
PFNGLGETUNIFORMLOCATIONARBPROC       glGetUniformLocationARB;
PFNGLUNIFORM1IARBPROC                glUniform1iARB;
PFNGLUNIFORM1FARBPROC                glUniform1fARB;
PFNGLUNIFORM2FARBPROC                glUniform2fARB;
PFNGLUNIFORM3FARBPROC                glUniform3fARB;
PFNGLUNIFORM4FARBPROC                glUniform4fARB;
PFNGLUNIFORMMATRIX2FVARBPROC         glUniformMatrix2fvARB;
PFNGLUNIFORMMATRIX3FVARBPROC         glUniformMatrix3fvARB;
PFNGLUNIFORMMATRIX4FVARBPROC         glUniformMatrix4fvARB;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC     glCompressedTexImage2DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC  glCompressedTexSubImage2DARB;
PFNGLGENFRAMEBUFFERSEXTPROC          glGenFramebuffersEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC       glDeleteFramebuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC          glBindFramebufferEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC     glFramebufferTexture2DEXT;

void init_opengl(void)
{
    /* Fragment and vertex program prerequisites. */

    glProgramStringARB             = (PFNGLPROGRAMSTRINGARBPROC)
                           opengl_proc("glProgramStringARB");
    glBindProgramARB              = (PFNGLBINDPROGRAMARBPROC)
                           opengl_proc("glBindProgramARB");
    glGenProgramsARB              = (PFNGLGENPROGRAMSARBPROC)
                           opengl_proc("glGenProgramsARB");
    glIsProgramARB                = (PFNGLISPROGRAMARBPROC)
                           opengl_proc("glIsProgramARB");
    glDeleteProgramsARB           = (PFNGLDELETEPROGRAMSARBPROC)
                           opengl_proc("glDeleteProgramsARB");
    glProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)
                           opengl_proc("glProgramLocalParameter4fvARB");
    glProgramEnvParameter4fARB    = (PFNGLPROGRAMENVPARAMETER4FARBPROC)
                           opengl_proc("glProgramEnvParameter4fARB");

    /* Fragment program support. */

    if (opengl_need("GL_ARB_fragment_program"))
    {
        GL_has_fragment_program = (GLboolean) (glProgramStringARB
                                            && glBindProgramARB
                                            && glGenProgramsARB);
    }

    /* Vertex program support. */

    if (opengl_need("GL_ARB_vertex_program"))
    {
        glDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)
                               opengl_proc("glDisableVertexAttribArrayARB");
        glEnableVertexAttribArrayARB  = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)
                               opengl_proc("glEnableVertexAttribArrayARB");
        glBindAttribLocationARB       = (PFNGLBINDATTRIBLOCATIONARBPROC)
                               opengl_proc("glBindAttribLocationARB");
        glVertexAttribPointerARB      = (PFNGLVERTEXATTRIBPOINTERARBPROC)
                              opengl_proc("glVertexAttribPointerARB");

        GL_has_vertex_program = (GLboolean) (glProgramEnvParameter4fARB
                                          && glDisableVertexAttribArrayARB
                                          && glEnableVertexAttribArrayARB
                                          && glBindAttribLocationARB
                                          && glVertexAttribPointerARB
                                          && glProgramStringARB
                                          && glBindProgramARB
                                          && glGenProgramsARB);
    }

    /* Vertex buffer object support. */

    if (opengl_need("GL_ARB_vertex_buffer_object"))
    {
        glBindBufferARB    = (PFNGLBINDBUFFERARBPROC)
                    opengl_proc("glBindBufferARB");
        glGenBuffersARB    = (PFNGLGENBUFFERSARBPROC)
                    opengl_proc("glGenBuffersARB");
        glBufferDataARB    = (PFNGLBUFFERDATAARBPROC)
                    opengl_proc("glBufferDataARB");
        glMapBufferARB     = (PFNGLMAPBUFFERARBPROC)
                    opengl_proc("glMapBufferARB");
        glUnmapBufferARB   = (PFNGLUNMAPBUFFERARBPROC)
                    opengl_proc("glUnmapBufferARB");
        glIsBufferARB      = (PFNGLISBUFFERARBPROC)
                    opengl_proc("glIsBufferARB");
        glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)
                    opengl_proc("glDeleteBuffersARB");

        GL_has_vertex_buffer_object = (GLboolean) (glBindBufferARB
                                               && glGenBuffersARB
                                               && glBufferDataARB
                                               && glIsBufferARB
                                               && glDeleteBuffersARB);
    }

    /* GLSL support. */

    if (opengl_need("GL_ARB_shader_objects"))
    {
        glUseProgramObjectARB     = (PFNGLUSEPROGRAMOBJECTARBPROC)
                           opengl_proc("glUseProgramObjectARB");
        glCreateShaderObjectARB   = (PFNGLCREATESHADEROBJECTARBPROC)
                           opengl_proc("glCreateShaderObjectARB");
        glCreateProgramObjectARB  = (PFNGLCREATEPROGRAMOBJECTARBPROC)
                           opengl_proc("glCreateProgramObjectARB");
        glShaderSourceARB         = (PFNGLSHADERSOURCEARBPROC)
                           opengl_proc("glShaderSourceARB");
        glCompileShaderARB        = (PFNGLCOMPILESHADERARBPROC)
                           opengl_proc("glCompileShaderARB");
        glAttachObjectARB         = (PFNGLATTACHOBJECTARBPROC)
                           opengl_proc("glAttachObjectARB");
        glLinkProgramARB          = (PFNGLLINKPROGRAMARBPROC)
                           opengl_proc("glLinkProgramARB");
        glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)
                           opengl_proc("glGetObjectParameterivARB");
        glGetInfoLogARB           = (PFNGLGETINFOLOGARBPROC)
                           opengl_proc("glGetInfoLogARB");
        glDeleteObjectARB         = (PFNGLDELETEOBJECTARBPROC)
                           opengl_proc("glDeleteObjectARB");

        glGetUniformLocationARB   = (PFNGLGETUNIFORMLOCATIONARBPROC)
                           opengl_proc("glGetUniformLocationARB");
        glUniform1iARB            = (PFNGLUNIFORM1IARBPROC)
                           opengl_proc("glUniform1iARB");
        glUniform1fARB            = (PFNGLUNIFORM1FARBPROC)
                           opengl_proc("glUniform1fARB");
        glUniform2fARB            = (PFNGLUNIFORM2FARBPROC)
                           opengl_proc("glUniform2fARB");
        glUniform3fARB            = (PFNGLUNIFORM3FARBPROC)
                           opengl_proc("glUniform3fARB");
        glUniform4fARB            = (PFNGLUNIFORM4FARBPROC)
                           opengl_proc("glUniform4fARB");
        glUniformMatrix2fvARB     = (PFNGLUNIFORMMATRIX2FVARBPROC)
                           opengl_proc("glUniformMatrix2fvARB");
        glUniformMatrix3fvARB     = (PFNGLUNIFORMMATRIX3FVARBPROC)
                           opengl_proc("glUniformMatrix3fvARB");
        glUniformMatrix4fvARB     = (PFNGLUNIFORMMATRIX4FVARBPROC)
                           opengl_proc("glUniformMatrix4fvARB");

        GL_has_shader_objects = (GLboolean) (glUseProgramObjectARB
                                          && glCreateShaderObjectARB
                                          && glCreateProgramObjectARB
                                          && glShaderSourceARB
                                          && glCompileShaderARB
                                          && glAttachObjectARB
                                          && glLinkProgramARB
                                          && glGetObjectParameterivARB
                                          && glGetInfoLogARB
                                          && glDeleteObjectARB
                                          && glGetUniformLocationARB
                                          && glUniform1iARB
                                          && glUniform1fARB
                                          && glUniform2fARB
                                          && glUniform3fARB
                                          && glUniform4fARB
                                          && glUniformMatrix2fvARB
                                          && glUniformMatrix3fvARB
                                          && glUniformMatrix4fvARB);
    }

    /* Multitexture support. */

    if (opengl_need("GL_ARB_multitexture"))
    {
        GLint TUs;

        glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)
                    opengl_proc("glActiveTextureARB");

        glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &TUs);
        GL_max_multitexture = GL_TEXTURE0_ARB +  TUs;
        GL_has_multitexture = (GLboolean) (glActiveTextureARB != NULL);
    }

    /* Compressed texture support. */

    if (opengl_need("GL_ARB_texture_compression"))
    {
        glCompressedTexImage2DARB    = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)
                              opengl_proc("glCompressedTexImage2DARB");
        glCompressedTexSubImage2DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)
                              opengl_proc("glCompressedTexSubImage2DARB");

        GL_has_texture_compression = (glCompressedTexImage2DARB
                                   && glCompressedTexSubImage2DARB);
    }

    if (opengl_need("GL_EXT_framebuffer_object"))
    {
        glGenFramebuffersEXT          = (PFNGLGENFRAMEBUFFERSEXTPROC)
                               opengl_proc("glGenFramebuffersEXT");
        glDeleteFramebuffersEXT       = (PFNGLDELETEFRAMEBUFFERSEXTPROC)
                               opengl_proc("glDeleteFramebuffersEXT");
        glBindFramebufferEXT          = (PFNGLBINDFRAMEBUFFEREXTPROC)
                               opengl_proc("glBindFramebufferEXT");
        glFramebufferTexture2DEXT     = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
                               opengl_proc("glFramebufferTexture2DEXT");

        GL_has_framebuffer_object = (glGenFramebuffersEXT
                                  && glDeleteFramebuffersEXT
                                  && glBindFramebufferEXT
                                  && glFramebufferTexture2DEXT);
    }

    /* Fence support */

    if (opengl_need("GL_NV_fence"))
    {
        glGenFencesNV                  = (PFNGLGENFENCESNVPROC)
                               opengl_proc("glGenFencesNV");
        glSetFenceNV                  = (PFNGLSETFENCENVPROC)
                               opengl_proc("glSetFenceNV");
        glFinishFenceNV               = (PFNGLFINISHFENCENVPROC)
                               opengl_proc("glFinishFenceNV");

        GL_has_fence = (glSetFenceNV
                     && glFinishFenceNV);
    }

    /* Miscellaneous procedureless extensions. */

    GL_has_point_sprite      = (opengl_need("GL_ARB_point_sprite"));
    GL_has_texture_rectangle = (opengl_need("GL_ARB_texture_rectangle"));

    init_opengl_obj();

    if (GL_has_fence)
        glGenFencesNV(1, &fence);
}

#endif

void fini_opengl(void)
{
    fini_opengl_obj();
}

/*---------------------------------------------------------------------------*/

GLfloat opengl_perf(GLfloat *all)
{
    static GLfloat fps   = 0.0f;
    static GLint   then  = 0;
    static GLint   count = 0;
    static GLint   total = 0;
    static GLint   start = 0;

    GLint now = (GLint) SDL_GetTicks();

    /* Compute the average FPS over 1000 milliseconds. */

    count++;

    if (now - then > 1000)
    {
        fps   = 1000.0f * count / (now - then);
        then  = now;
        count = 0;
    }

    /* Compute the total average FPS. */

    if (start)
        total++;
    else
        start = SDL_GetTicks();

    if (all) *all = 1000.0f * total / (now - start);

    return fps;
}

/*---------------------------------------------------------------------------*/

static void opengl_object_error(const char *tag, GLhandleARB H)
{
    char *s;
    GLint l;

    glGetObjectParameterivARB(H, GL_OBJECT_INFO_LOG_LENGTH_ARB, &l);

    if ((s = (char *) calloc(l + 1, 1)))
    {
        glGetInfoLogARB(H, l, NULL, s);
        error("%s: %s\n", tag, s);
        free(s);
    }
}

GLhandleARB opengl_shader_object(GLenum type, const char *text)
{
    GLhandleARB H = glCreateShaderObjectARB(type);
    GLint p;

    glShaderSourceARB(H, 1, &text, NULL);
    glCompileShaderARB(H);

    glGetObjectParameterivARB(H, GL_OBJECT_COMPILE_STATUS_ARB,  &p);

    if (p == 0)
    {
        if (type == GL_VERTEX_SHADER_ARB)
            opengl_object_error("Vertex Shader", H);
        else
            opengl_object_error("Fragment Shader", H);
    }

    return H;
}

GLhandleARB opengl_program_object(GLhandleARB vert_shad, GLhandleARB frag_shad)
{
    GLhandleARB H = glCreateProgramObjectARB();
    GLint p;

    glBindAttribLocationARB(H, 6, "tangent");
    glBindAttribLocationARB(H, 7, "bitangent");

    if (vert_shad) glAttachObjectARB(H, vert_shad);
    if (frag_shad) glAttachObjectARB(H, frag_shad);

    glLinkProgramARB(H);

    glGetObjectParameterivARB(H, GL_OBJECT_LINK_STATUS_ARB, &p);
    if (p == 0) opengl_object_error("Program Linker", H);

    return H;
}

/*---------------------------------------------------------------------------*/

/* Generate and return a new vertex program object.  Bind the given program  */
/* text to it.  Print any program error message to the console.              */

GLuint opengl_vert_prog(const char *text)
{
    GLuint o;

    glGenProgramsARB(1, &o);
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, o);

    glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB, strlen(text), text);

    if (glGetError() == GL_INVALID_OPERATION)
        error("Vertex program: %s\n",
              glGetString(GL_PROGRAM_ERROR_STRING_ARB));

    return o;
}

/* Generate and return a new fragment program object.  Bind the given        */
/* program text to it.  Print any program error message to the console.      */

GLuint opengl_frag_prog(const char *text)
{
    GLuint o;

    glGenProgramsARB(1, &o);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, o);

    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
                       GL_PROGRAM_FORMAT_ASCII_ARB, strlen(text), text);

    if (glGetError() == GL_INVALID_OPERATION)
        error("Fragment program: %s\n",
              glGetString(GL_PROGRAM_ERROR_STRING_ARB));

    return o;
}

/*---------------------------------------------------------------------------*/

#define MAXFBO 8

static GLint fbo_v[MAXFBO];
static int   fbo_i = 0;

static void fix_point_sprite_origin(void)
{
    /* HACK: NVIDIA point sprite origin differs between FBO and window. */

#ifdef GL_POINT_SPRITE_COORD_ORIGIN
    GLint fbo;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fbo);

    if (fbo)
        glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
    else
        glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
#endif
}

void opengl_push_framebuffer(int fbo)
{
    if (GL_has_framebuffer_object)
    {
        if (fbo_i < MAXFBO)
            glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, fbo_v + fbo_i++);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

        fix_point_sprite_origin();
    }
}

void opengl_pop_framebuffer(void)
{
    if (GL_has_framebuffer_object)
    {
        if (fbo_i > 0)
        {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_v[--fbo_i]);

            fix_point_sprite_origin();
        }
    }
}

/*===========================================================================*/

void opengl_set_fence(void)
{
    if (GL_has_fence)
        glSetFenceNV(fence, GL_ALL_COMPLETED_NV);
}

void opengl_get_fence(void)
{
    if (GL_has_fence)
        glFinishFenceNV(fence);
    else
        glFinish();
}

/*---------------------------------------------------------------------------*/

void opengl_check(const char *format, ...)
{
#ifndef NDEBUG
    GLenum err;

    while ((err = glGetError()) != GL_NO_ERROR)
    {
        char string[MAXSTR];
        va_list args;

        va_start(args, format);
        vsprintf(string, format, args);
        va_end(args);

        error("OpenGL error: %s: %s", gluErrorString(err), string);
    }
#endif
}

/*---------------------------------------------------------------------------*/

