#include "opengl.h"

/*---------------------------------------------------------------------------*/

void galaxy_init(void)
{
}

void galaxy_draw(void)
{
    glBegin(GL_QUADS);
    {
        glVertex3f(+1.0f, 0.0f, +1.0f);
        glVertex3f(+1.0f, 0.0f, -1.0f);
        glVertex3f(-1.0f, 0.0f, -1.0f);
        glVertex3f(-1.0f, 0.0f, +1.0f);
    }
    glEnd();
}

/*---------------------------------------------------------------------------*/
