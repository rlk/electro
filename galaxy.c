#include "opengl.h"
#include "viewer.h"
#include "png.h"

/*---------------------------------------------------------------------------*/

static GLuint o;

void galaxy_bill(double x, double y, double z)
{
    glPushMatrix();
    {
        glTranslated(x, y, z);

        viewer_bill();

        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 0); glVertex3d(+1.0, +1.0, 0.0);
            glTexCoord2i(0, 1); glVertex3d(+1.0, -1.0, 0.0);
            glTexCoord2i(1, 1); glVertex3d(-1.0, -1.0, 0.0);
            glTexCoord2i(1, 0); glVertex3d(-1.0, +1.0, 0.0);
        }
        glEnd();
    }
    glPopMatrix();
}

void galaxy_init(void)
{
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    o = png_load("puff.png");
}

void galaxy_draw(void)
{
    glColor4d(1.0, 1.0, 1.0, 1.0);

    galaxy_bill(+1.0, 0.0, +1.0);
    galaxy_bill(+1.0, 0.0, -1.0);
    galaxy_bill(-1.0, 0.0, -1.0);
    galaxy_bill(-1.0, 0.0, +1.0);
}

/*---------------------------------------------------------------------------*/
