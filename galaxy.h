#ifndef GALAXY_H
#define GALAXY_H

/*---------------------------------------------------------------------------*/

struct star
{
    float         position[3];
    float         magnitude;
    unsigned char color[3];
};

struct node
{
    int size;
    int seed;
    int type;

    int star_first;
    int star_count;
};

/*---------------------------------------------------------------------------*/

void galaxy_init(void);
void galaxy_draw(const double[3]);

/*---------------------------------------------------------------------------*/

#endif
