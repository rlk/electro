#include <mpi.h>
#include <SDL.h>
#include <stdio.h>
#include <unistd.h>

#include "opengl.h"
#include "shared.h"
#include "server.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

void server_send_event(struct event *e)
{
    size_t sz = sizeof (struct event);
    int  err;

    if ((err = MPI_Bcast(e, sz, MPI_BYTE, 0, MPI_COMM_WORLD)) != MPI_SUCCESS)
        mpi_error(err);
}

void server_send_draw(void)
{
    struct event e;

    e.type = EVENT_DRAW;
    e.x    = 0.0f;
    e.y    = 0.0f;
    e.z    = 0.0f;

    server_send_event(&e);
}

void server_send_move(float x, float y, float z)
{
    struct event e;

    e.type = EVENT_MOVE;
    e.x    = x;
    e.y    = y;
    e.z    = z;

    server_send_event(&e);
}

void server_send_turn(float x, float y, float z)
{
    struct event e;

    e.type = EVENT_TURN;
    e.x    = x;
    e.y    = y;
    e.z    = z;

    server_send_event(&e);
}

void server_send_zoom(float k)
{
    struct event e;

    e.type = EVENT_ZOOM;
    e.x    = k;
    e.y    = 0.0f;
    e.z    = 0.0f;

    server_send_event(&e);
}

void server_send_dist(float k)
{
    struct event e;

    e.type = EVENT_DIST;
    e.x    = k;
    e.y    = 0.0f;
    e.z    = 0.0f;

    server_send_event(&e);
}

void server_send_magn(float k)
{
    struct event e;

    e.type = EVENT_MAGN;
    e.x    = k;
    e.y    = 0.0f;
    e.z    = 0.0f;

    server_send_event(&e);
}

void server_send_exit(void)
{
    struct event e;

    e.type = EVENT_EXIT;
    e.x    = 0.0f;
    e.y    = 0.0f;
    e.z    = 0.0f;

    server_send_event(&e);

    MPI_Barrier(MPI_COMM_WORLD);
}

/*---------------------------------------------------------------------------*/

static void server_init(void)
{
    SDL_WM_SetCaption("Server", "Server");
}

static void server_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapBuffers();
}

static int server_loop(void)
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_MOUSEMOTION:     script_point(e.motion.x, e.motion.y); break;
        case SDL_MOUSEBUTTONDOWN: script_click(e.button.button, 1);     break;
        case SDL_MOUSEBUTTONUP:   script_click(e.button.button, 0);     break;
        case SDL_KEYDOWN:         script_keybd(e.key.keysym.sym, 1);    break;
        case SDL_KEYUP:           script_keybd(e.key.keysym.sym, 0);    break;

        case SDL_QUIT:
            server_send_exit();
            return 0;
        }

    return 1;
}

/*---------------------------------------------------------------------------*/

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s [OPTION]...\n", name);
    fprintf(stderr, "\t-s <filename>   Source Lua script\n");
    fprintf(stderr, "\t-f <filename>   Read binary star catalog\n");
    fprintf(stderr, "\t-t <filename>   Read ascii star catalog\n");
    fprintf(stderr, "\t-o <filename>   Write binary star catalog\n");
    fprintf(stderr, "\t-h              Help\n");

    server_send_exit();
}

void server(int np, int argc, char *argv[])
{
    if (script_init())
    {
        int c;

        while ((c = getopt(argc, argv, "hs:f:t:o:")) > 0)
            switch (c)
            {
            case 's': script_file(optarg);           break;
            case 'f': star_read_catalog_bin(optarg); break;
            case 't': star_read_catalog_txt(optarg); break;
            case 'o': star_write_catalog(optarg);    break;
            case '?':
            case 'h': usage(argv[0]); return;
            }

        if (SDL_Init(SDL_INIT_VIDEO) == 0)
        {
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            if (SDL_SetVideoMode(WIN_W, WIN_H, 0, WIN_M | SDL_OPENGL))
            {
                server_init();

                while (SDL_WaitEvent(NULL))
                    if (server_loop() == 0)
                        break;
            }
            SDL_Quit();
        }

        script_free();
    }
}

/*---------------------------------------------------------------------------*/
