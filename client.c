#include <mpi.h>
#include <SDL.h>
#include <stdio.h>

#include "opengl.h"
#include "shared.h"
#include "client.h"

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
}

void client_recv_turn(const struct event *e)
{
}

void client_recv_zoom(const struct event *e)
{
}

void client_recv_dist(const struct event *e)
{
}

void client_recv_magn(const struct event *e)
{
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
        printf("client recv %d\n", e.type);

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
}

static void client_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

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
