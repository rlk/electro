#ifndef SHARED_H
#define SHARED_H

/*---------------------------------------------------------------------------*/

#define TITLE "Total Perspective Vortex"

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

#define NAMELEN 32

struct viewport
{
    char name[NAMELEN];
    int gx;
    int gy;
    int lx;
    int ly;
    int w;
    int h;
};

void viewport_init(int);
void viewport_tile(const char *, int, int, int, int, int, int);
void viewport_sync(int, int);

/*---------------------------------------------------------------------------*/

void mpi_error(int);

/*---------------------------------------------------------------------------*/

#endif
