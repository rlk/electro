#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "opengl.h"
#include "galaxy.h"
#include "viewer.h"
#include "png.h"

/*---------------------------------------------------------------------------*/

#define N 118218
static int n = 0;

static GLuint texture;
static struct star *S;

/*---------------------------------------------------------------------------*/

void galaxy_star(FILE *fp, int i)
{
    char buf[512];

    if (fgets(buf, 512, fp))
    {
        double mag, ra, de, plx;

        sscanf(buf + 41, "%lf", &mag);
        sscanf(buf + 51, "%lf", &ra);
        sscanf(buf + 64, "%lf", &de);
        sscanf(buf + 79, "%lf", &plx);

        if (mag < 15.0)
        {
            ra  = M_PI * ra / 180.0;
            de  = M_PI * de / 180.0;

            if (plx > 0.0001)
                plx = 1000.0 / plx;
            else
                plx = 1000.0;

            S[n].position[0] =  sin(ra) * cos(de) * 10;
            S[n].position[1] =            sin(de) * 10;
            S[n].position[2] =  cos(ra) * cos(de) * 10;
            
            S[n].temperature =  1;
            S[n].brightness  =  sqrt(pow(10, -mag / 2.5)) * 0.25;

            n++;
        }
    }
}

void galaxy_init(void)
{
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    texture = png_load("blob.png");

    if ((S = (struct star *) calloc(sizeof (struct star), N)))
    {
        FILE *fp;

        if ((fp = fopen("hip_main.dat", "r")))
        {
            int i;

            for (i = 0; i < N; i++)
                galaxy_star(fp, i);
       
            fclose(fp);
        }
    }
}

/*---------------------------------------------------------------------------*/

void galaxy_bill(double x, double y, double z, double k)
{
    glPushMatrix();
    {
        glTranslated(x, y, z);

        viewer_bill();

        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 0); glVertex3d(+k, +k, 0.0);
            glTexCoord2i(0, 1); glVertex3d(+k, -k, 0.0);
            glTexCoord2i(1, 1); glVertex3d(-k, -k, 0.0);
            glTexCoord2i(1, 0); glVertex3d(-k, +k, 0.0);
        }
        glEnd();
    }
    glPopMatrix();
}

void galaxy_draw(void)
{
    int i;

    glColor4d(1.0, 1.0, 1.0, 1.0);

    for (i = 0; i < n; i++)
        galaxy_bill(S[i].position[0],
                    S[i].position[1],
                    S[i].position[2], S[i].brightness);
}

/*---------------------------------------------------------------------------*/
