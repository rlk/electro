#ifndef GALAXY_H
#define GALAXY_H

/*---------------------------------------------------------------------------*/

#define RADIUS 15000.0f

struct node
{
    int seed;
    int type;

    int star_first;
    int star_count;

    float position[3];
    float radius;
};

/*---------------------------------------------------------------------------*/

void galaxy_init(void);
void galaxy_draw(const double[3]);

/*---------------------------------------------------------------------------*/

#endif
