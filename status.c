#include "opengl.h"

/*---------------------------------------------------------------------------*/

static float camera_pos[4];
static float camera_rot[3];

static float camera_dist;
static float camera_magn;
static float camera_zoom;

/*---------------------------------------------------------------------------*/

void status_init(void)
{
    camera_pos[0] =    0.0f;
    camera_pos[1] =   15.5f;
    camera_pos[2] = 9200.0f;
    camera_pos[3] =    1.0f;

    camera_rot[0] =    0.0f;
    camera_rot[1] =    0.0f;
    camera_rot[2] =    0.0f;

    camera_dist   = 1000.0f;
    camera_magn   =  128.0f;
    camera_zoom   =    0.5f;
}

void status_draw_camera(void)
{
    glLoadIdentity();

    glTranslatef(0, 0, -camera_dist);

    glRotatef(-camera_rot[0], 1, 0, 0);
    glRotatef(-camera_rot[1], 0, 1, 0);
    glRotatef(-camera_rot[2], 0, 0, 1);
    
    glTranslatef(-camera_pos[0], -camera_pos[1], -camera_pos[2]);

    glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, 0, camera_pos);
    glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, 1, camera_magn);
}

/*---------------------------------------------------------------------------*/

void status_set_camera_pos(float x, float y, float z)
{
    camera_pos[0] = x;
    camera_pos[1] = y;
    camera_pos[2] = z;
}

void status_set_camera_rot(float x, float y, float z)
{
    camera_rot[0] = x;
    camera_rot[1] = y;
    camera_rot[2] = z;
}

void status_set_camera_dist(float d)
{
    camera_dist = d;
}

void status_set_camera_magn(float d)
{
    camera_magn = d;
}

void status_set_camera_zoom(float d)
{
    camera_zoom = d;
}

/*---------------------------------------------------------------------------*/

void status_get_camera_pos(float *x, float *y, float *z)
{
    *x = camera_pos[0];
    *y = camera_pos[1];
    *z = camera_pos[2];
}

void status_get_camera_rot(float *x, float *y, float *z)
{
    *x = camera_rot[0];
    *y = camera_rot[1];
    *z = camera_rot[2];
}

float status_get_camera_dist(void)
{
    return camera_dist;
}

float status_get_camera_magn(void)
{
    return camera_magn;
}

float status_get_camera_zoom(void)
{
    return camera_zoom;
}

/*---------------------------------------------------------------------------*/
