#ifndef SHARED_H
#define SHARED_H

/*---------------------------------------------------------------------------*/

#define EVENT_DRAW 1
#define EVENT_MOVE 2
#define EVENT_TURN 3
#define EVENT_ZOOM 4
#define EVENT_DIST 5
#define EVENT_MAGN 6
#define EVENT_EXIT 7

struct event
{
    int   type;
    float x;
    float y;
    float z;
};

/*---------------------------------------------------------------------------*/

void mpi_error(int);

/*---------------------------------------------------------------------------*/

#endif
