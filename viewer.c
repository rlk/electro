#include <SDL.h>
#include <math.h>

#include "opengl.h"

/*---------------------------------------------------------------------------*/

static double position[3];
static double rotation[2];
static double distance;

static int button[3];

/*---------------------------------------------------------------------------*/

int viewer_point(int x, int y)
{
    static int last_x = 0;
    static int last_y = 0;

    int dx = last_x - x;
    int dy = last_y - y;

    last_x = x;
    last_y = y;

    if (button[0])
    {
        rotation[0] -= 180.0 * dy / 500.0;
        rotation[1] -= 180.0 * dx / 500.0;

        if (rotation[0] >  +90.0) rotation[0]  = +90.0;
        if (rotation[0] <  -90.0) rotation[0]  = -90.0;
        if (rotation[1] > +180.0) rotation[1] -= 360.0;
        if (rotation[1] < -180.0) rotation[1] += 360.0;

        return 1;
    }
    if (button[2])
    {
        distance += dy / 10.0;

        if (distance < 0.0)
            distance = 0.0;

        return 1;
    }

    return 0;
}

int viewer_click(int b, int s)
{
    switch (b)
    {
    case 1: button[0] = s;  break;
    case 2: button[1] = s;  break;
    case 3: button[2] = s;  break;

    case 4: if (s) distance += 1.0; break;
    case 5: if (s) distance -= 1.0; break;
    }

    if (distance < 0.0)
        distance = 0.0;

    return 1;
}

/*---------------------------------------------------------------------------*/

void viewer_init(void)
{
    button[0]   = 0;
    button[1]   = 0;
    button[2]   = 0;

    position[0] = 0;
    position[1] = 0;
    position[2] = 0;
    rotation[0] = 0;
    rotation[1] = 0;
    distance    = 0;
}

void viewer_draw(void)
{
    GLdouble a = (GLdouble) WIN_W / (GLdouble) WIN_H;
    GLdouble z = 0.5;

    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        glFrustum(-a * z, +a * z, -z, +z, 1.0, 10000.0);
    }

    glMatrixMode(GL_MODELVIEW);
    {
        glLoadIdentity();
        glTranslated(0, 0, -distance);
        glRotated(-rotation[0], 1, 0, 0);
        glRotated(-rotation[1], 0, 1, 0);
    }
}

void viewer_get_pos(double p[3])
{
    double T = M_PI * rotation[1] / 180.0;
    double P = M_PI * rotation[0] / 180.0;

    p[0] = position[0] + sin(T) * cos(P) * distance;
    p[1] = position[1] -          sin(P) * distance;
    p[2] = position[2] + cos(T) * cos(P) * distance;
}

/*---------------------------------------------------------------------------*/
