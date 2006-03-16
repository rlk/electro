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

#ifndef OPENGL_H
#define OPENGL_H

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#define GL_GLEXT_PROTOTYPES 1
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glext.h"

/*---------------------------------------------------------------------------*/

extern GLboolean GL_has_fragment_program;
extern GLboolean GL_has_vertex_program;
extern GLboolean GL_has_vertex_buffer_object;
extern GLboolean GL_has_framebuffer_object;
extern GLboolean GL_has_point_sprite;
extern GLboolean GL_has_texture_rectangle;
extern GLboolean GL_has_texture_compression;
extern GLboolean GL_has_shader_objects;
extern GLboolean GL_has_vertex_shader;
extern GLboolean GL_has_fragment_shader;
extern GLboolean GL_has_multitexture;
extern GLenum    GL_max_multitexture;

#ifndef __APPLE__
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArrayARB;
extern PFNGLBINDATTRIBLOCATIONARBPROC       glBindAttribLocationARB;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB;
extern PFNGLPROGRAMENVPARAMETER4FARBPROC    glProgramEnvParameter4fARB;
extern PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointerARB;
extern PFNGLPROGRAMSTRINGARBPROC            glProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC              glBindProgramARB;
extern PFNGLGENPROGRAMSARBPROC              glGenProgramsARB;
extern PFNGLISPROGRAMARBPROC                glIsProgramARB;
extern PFNGLDELETEPROGRAMSARBPROC           glDeleteProgramsARB;
extern PFNGLBINDBUFFERARBPROC               glBindBufferARB;
extern PFNGLGENBUFFERSARBPROC               glGenBuffersARB;
extern PFNGLBUFFERDATAARBPROC               glBufferDataARB;
extern PFNGLISBUFFERARBPROC                 glIsBufferARB;
extern PFNGLDELETEBUFFERSARBPROC            glDeleteBuffersARB;
extern PFNGLACTIVETEXTUREARBPROC            glActiveTextureARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC         glUseProgramObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC       glCreateShaderObjectARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC      glCreateProgramObjectARB;
extern PFNGLSHADERSOURCEARBPROC             glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC            glCompileShaderARB;
extern PFNGLATTACHOBJECTARBPROC             glAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC              glLinkProgramARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC     glGetObjectParameterivARB;
extern PFNGLGETINFOLOGARBPROC               glGetInfoLogARB;
extern PFNGLDELETEOBJECTARBPROC             glDeleteObjectARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC       glGetUniformLocationARB;
extern PFNGLUNIFORM1IARBPROC                glUniform1iARB;
extern PFNGLUNIFORM1FVARBPROC               glUniform1fvARB;
extern PFNGLUNIFORM2FVARBPROC               glUniform2fvARB;
extern PFNGLUNIFORM3FVARBPROC               glUniform3fvARB;
extern PFNGLUNIFORM4FVARBPROC               glUniform4fvARB;
extern PFNGLUNIFORMMATRIX2FVARBPROC         glUniformMatrix2fvARB;
extern PFNGLUNIFORMMATRIX3FVARBPROC         glUniformMatrix3fvARB;
extern PFNGLUNIFORMMATRIX4FVARBPROC         glUniformMatrix4fvARB;
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC     glCompressedTexImage2DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC  glCompressedTexSubImage2DARB;
extern PFNGLGENFRAMEBUFFERSEXTPROC          glGenFramebuffersEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC       glDeleteFramebuffersEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC          glBindFramebufferEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC     glFramebufferTexture2DEXT;
#endif

#ifndef GL_POINT_SPRITE_ARB
#define GL_POINT_SPRITE_ARB GL_POINT_SPRITE_NV
#endif

#ifndef GL_COORD_REPLACE_ARB
#define GL_COORD_REPLACE_ARB GL_COORD_REPLACE_NV
#endif

#ifdef __APPLE__
typedef GLvoid (*_GLUfuncptr)(void);
#endif

#ifdef _WIN32
typedef GLvoid *_GLUfuncptr;
#endif

/*---------------------------------------------------------------------------*/

void      init_opengl(void);
void      fini_opengl(void);

void     *opengl_proc(const char *);
GLboolean opengl_need(const char *);
GLfloat   opengl_perf(GLfloat *);

void        opengl_basis_mult(float[3][3]); 
void        opengl_basis_invt(float[3][3]); 
GLhandleARB opengl_shader_object(GLenum, const char *);
GLhandleARB opengl_program_object(GLhandleARB, GLhandleARB);
GLuint      opengl_frag_prog(const char *);
GLuint      opengl_vert_prog(const char *);

void      opengl_draw_xyz(float, float, float);
void      opengl_draw_vec(float, float, float, float, float, float);
void      opengl_draw_grd(float, float, float, float);
void      opengl_draw_box(float, float, float);
void      opengl_draw_cap(float, float);
void      opengl_draw_sph(float);

/*---------------------------------------------------------------------------*/

void opengl_check(const char *, ...);

/*---------------------------------------------------------------------------*/

#endif
