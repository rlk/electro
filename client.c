#include <mpi.h>
#include <SDL.h>
#include <stdio.h>

#include "opengl.h"
#include "shared.h"
#include "status.h"
#include "client.h"

/*---------------------------------------------------------------------------*/

static void client_recv_draw(void)
{
    SDL_Event e;

    e.type      = SDL_USEREVENT;
    e.user.code = EVENT_DRAW;

    SDL_PushEvent(&e);
}

static void client_recv_exit(void)
{
    SDL_Event e;

    e.type      = SDL_USEREVENT;
    e.user.code = EVENT_EXIT;

    SDL_PushEvent(&e);

    MPI_Barrier(MPI_COMM_WORLD);
}

static void client_recv_event(void)
{
    size_t sz = sizeof (struct event);
    struct event e;
    int err;

    if ((err = MPI_Bcast(&e, sz, MPI_BYTE, 0, MPI_COMM_WORLD)) == MPI_SUCCESS)
    {
        switch (e.type)
        {
        case EVENT_DRAW: client_recv_draw();                   break;
        case EVENT_MOVE: status_set_camera_pos(e.x, e.y, e.z); break;
        case EVENT_TURN: status_set_camera_rot(e.x, e.y, e.z); break;
        case EVENT_DIST: status_set_camera_dist(e.x);          break;
        case EVENT_MAGN: status_set_camera_magn(e.x);          break;
        case EVENT_ZOOM: status_set_camera_zoom(e.x);          break;
        case EVENT_EXIT: client_recv_exit();                   break;
        }
    }
    else mpi_error(err);
}

/*---------------------------------------------------------------------------*/

static void client_init(int id)
{
    star_read_catalog_bin("hip_main.bin");

    status_init();
    galaxy_init();
    star_init();
}

static void client_draw(void)
{
    GLdouble a = (GLdouble) WIN_W / (GLdouble) WIN_H;
    GLdouble z = (GLdouble) status_get_camera_zoom();

    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        glFrustum(-a * z, +a * z, -z, +z, 1.0, 1000000.0);
    }
    glMatrixMode(GL_MODELVIEW);

    status_draw_camera();
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
    char buf[32];

    sprintf(buf, "%d, %d", (id - 1) * WIN_W, 0);
    setenv("SDL_VIDEO_WINDOW_POS", buf, 1);

    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        if (SDL_SetVideoMode(WIN_W, WIN_H, 0, SDL_OPENGL | SDL_NOFRAME))
        {
            client_init(id);

            while (client_loop())
                client_recv_event();
        }
        SDL_Quit();
    }
}

/*---------------------------------------------------------------------------*/
