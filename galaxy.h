#ifndef GALAXY_H
#define GALAXY_H

/*---------------------------------------------------------------------------*/

struct star
{
    double position[3];
    double temperature;
    double brightness;
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
void galaxy_draw(void);

/*---------------------------------------------------------------------------*/

#endif
