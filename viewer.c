#include <SDL.h>
#include "opengl.h"

/*---------------------------------------------------------------------------*/

static double position[3];
static double rotation[3];

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
        rotation[0] += 180.0 * dy / 500.0;
        rotation[1] += 180.0 * dx / 500.0;

        if (rotation[0] >  +90.0) rotation[0]  = +90.0;
        if (rotation[0] <  -90.0) rotation[0]  = -90.0;
        if (rotation[1] > +180.0) rotation[1] -= 360.0;
        if (rotation[1] < -180.0) rotation[1] += 360.0;

        return 1;
    }

    return 0;
}

int viewer_click(int b, int s)
{
    switch (b)
    {
    case 1: button[0] = ((s == SDL_PRESSED) ? 1 : 0);  break;
    case 2: button[1] = ((s == SDL_PRESSED) ? 1 : 0);  break;
    case 3: button[2] = ((s == SDL_PRESSED) ? 1 : 0);  break;

    case 4: if (s == SDL_PRESSED) position[2] += 0.25; break;
    case 5: if (s == SDL_PRESSED) position[2] -= 0.25; break;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

void viewer_init(void)
{
    position[0] =   0;
    position[1] =   0;
    position[2] =   5;
    rotation[0] = -30;
    rotation[1] =   0;
    rotation[2] =   0;
    button[0]   =   0;
    button[1]   =   0;
    button[2]   =   0;
}

void viewer_draw(void)
{
    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        glFrustum(-1.0, +1.0, -1.0, +1.0, +1.0, +1000.0);
    }

    glMatrixMode(GL_MODELVIEW);
    {
        glLoadIdentity();
        glTranslated(-position[0], -position[1], -position[2]);
        glRotated(-rotation[0], 1, 0, 0);
        glRotated(-rotation[1], 0, 1, 0);
        glRotated(-rotation[2], 0, 0, 1);
    }
}

/*---------------------------------------------------------------------------*/
