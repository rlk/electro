#include <mpi.h>
#include <SDL.h>
#include <stdio.h>

#include "opengl.h"
#include "shared.h"
#include "client.h"

/*---------------------------------------------------------------------------*/

static float pos[4] = { 0.0, 15.5, 9200.0, 1.0 };
static float rot[3] = { 0.0, 0.0, 0.0 };

static float dist   = 1000.0f;
static float magn   =  128.0f;
static float zoom   =    0.5f;

/*---------------------------------------------------------------------------*/

void client_recv_draw(void)
{
    SDL_Event e;

    e.type      = SDL_USEREVENT;
    e.user.code = EVENT_DRAW;

    SDL_PushEvent(&e);
}

void client_recv_move(const struct event *e)
{
    pos[0] = e->x;
    pos[1] = e->y;
    pos[2] = e->z;
}

void client_recv_turn(const struct event *e)
{
    rot[0] = e->x;
    rot[1] = e->y;
    rot[2] = e->z;
}

void client_recv_zoom(const struct event *e)
{
    zoom = e->x;
}

void client_recv_dist(const struct event *e)
{
    dist = e->x;
}

void client_recv_magn(const struct event *e)
{
    magn = e->x;
}

void client_recv_exit(void)
{
    SDL_Event e;

    e.type      = SDL_USEREVENT;
    e.user.code = EVENT_EXIT;

    SDL_PushEvent(&e);

    MPI_Barrier(MPI_COMM_WORLD);
}

void client_recv_event(void)
{
    size_t sz = sizeof (struct event);
    struct event e;
    int err;

    if ((err = MPI_Bcast(&e, sz, MPI_BYTE, 0, MPI_COMM_WORLD)) == MPI_SUCCESS)
    {
        switch (e.type)
        {
        case EVENT_DRAW: client_recv_draw();   break;
        case EVENT_MOVE: client_recv_move(&e); break;
        case EVENT_TURN: client_recv_turn(&e); break;
        case EVENT_ZOOM: client_recv_zoom(&e); break;
        case EVENT_DIST: client_recv_dist(&e); break;
        case EVENT_MAGN: client_recv_magn(&e); break;
        case EVENT_EXIT: client_recv_exit();   break;
        }
    }
    else mpi_error(err);
}

/*---------------------------------------------------------------------------*/

static void client_init(int id)
{
    char buf[32];

    sprintf(buf, "Client %d\n", id);

    SDL_WM_SetCaption(buf, buf);

    galaxy_init();
    star_read_catalog_bin("hip_main.bin");
}

static void client_draw(void)
{
    GLdouble a = (GLdouble) WIN_W / (GLdouble) WIN_H;

    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        glFrustum(-a * zoom, +a * zoom, -zoom, +zoom, 1.0, 1000000.0);
    }

    glMatrixMode(GL_MODELVIEW);
    {
        glLoadIdentity();
        glTranslatef(0, 0, -dist);

        glRotatef(-rot[0], 1, 0, 0);
        glRotatef(-rot[1], 0, 1, 0);
        glRotatef(-rot[2], 0, 0, 1);

        glTranslatef(-pos[0], -pos[1], -pos[2]);
    }

    glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, 0, pos);
    glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, 1, magn);

    galaxy_draw();

    SDL_GL_SwapBuffers();
}

static int client_loop(void)
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_USEREVENT:
            switch (e.user.code)
            {
            case EVENT_DRAW: client_draw(); return 1;
            case EVENT_EXIT:                return 0;
            }
            break;

        case SDL_QUIT:
            return 0;
        }

    return 1;
}

void client(int np, int id)
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        if (SDL_SetVideoMode(WIN_W, WIN_H, 0, WIN_M | SDL_OPENGL))
        {
            client_init(id);

            while (client_loop())
                client_recv_event();
        }
        SDL_Quit();
    }
}

/*---------------------------------------------------------------------------*/
