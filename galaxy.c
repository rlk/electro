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

        if (mag < 6.0)
        {
            plx =    1000.0 / plx;
            ra  = M_PI * ra / 180.0;
            de  = M_PI * de / 180.0;

            S[n].position[0] =  sin(ra) * cos(de) * plx;
            S[n].position[1] =            sin(de) * plx;
            S[n].position[2] =  cos(ra) * cos(de) * plx;
        
            S[n].temperature =  0;
            S[n].magnitude   =  mag - 5.0 * log(plx / 10.0);

            n++;
        }
    }
}

void galaxy_init(void)
{
    glEnable(GL_POINT_SPRITE_ARB);
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

void galaxy_draw(const double p[3])
{
    int i;

    glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

    glColor4d(1.0, 1.0, 1.0, 1.0);

    for (i = 0; i < n; i++)
    {
        double dx = S[i].position[0] - p[0];
        double dy = S[i].position[1] - p[1];
        double dz = S[i].position[2] - p[2];
        double d  = sqrt(dx * dx + dy * dy + dz * dz);
        double m  = S[i].magnitude + 5.0 * log(d / 10.0);
        double L  = pow(10.0, -m / 2.5);

        glPointSize(sqrt(L) * 25.0);
        glBegin(GL_POINTS);
        glVertex3dv(S[i].position);
        glEnd();
    }
}

/*---------------------------------------------------------------------------*/
