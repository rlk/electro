#include <mpi.h>
#include <SDL.h>
#include <stdio.h>
#include <unistd.h>

#include "opengl.h"
#include "shared.h"
#include "status.h"
#include "server.h"
#include "star.h"

/*---------------------------------------------------------------------------*/

static void server_send_event(struct event *e)
{
    size_t sz = sizeof (struct event);
    int  err;

    if ((err = MPI_Bcast(e, sz, MPI_BYTE, 0, MPI_COMM_WORLD)) != MPI_SUCCESS)
        mpi_error(err);
}

void server_send_draw(void)
{
    struct event e = { EVENT_DRAW, 0.0f, 0.0f, 0.0f };

    server_send_event(&e);
}

void server_send_move(void)
{
    struct event e = { EVENT_MOVE, 0.0f, 0.0f, 0.0f };

    status_get_camera_pos(&e.x, &e.y, &e.z);
    server_send_event(&e);
}

void server_send_turn(void)
{
    struct event e = { EVENT_TURN, 0.0f, 0.0f, 0.0f };

    status_get_camera_rot(&e.x, &e.y, &e.z);
    server_send_event(&e);
}

void server_send_dist(void)
{
    struct event e = { EVENT_DIST, 0.0f, 0.0f, 0.0f };

    e.x = status_get_camera_dist();
    server_send_event(&e);
}

void server_send_magn(void)
{
    struct event e = { EVENT_MAGN, 0.0f, 0.0f, 0.0f };

    e.x = status_get_camera_magn();
    server_send_event(&e);
}

void server_send_zoom(void)
{
    struct event e = { EVENT_ZOOM, 0.0f, 0.0f, 0.0f };

    e.x = status_get_camera_zoom();
    server_send_event(&e);
}

void server_send_exit(void)
{
    struct event e = { EVENT_EXIT, 0.0f, 0.0f, 0.0f };

    server_send_event(&e);
    MPI_Barrier(MPI_COMM_WORLD);
}

/*---------------------------------------------------------------------------*/

static void server_init(void)
{
    status_init();
    galaxy_init();
    star_init();

    server_send_draw();
}

static void server_draw(void)
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

static int server_loop(void)
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_MOUSEMOTION:     script_point(e.motion.x, e.motion.y); break;
        case SDL_MOUSEBUTTONDOWN: script_click(e.button.button,  1);    break;
        case SDL_MOUSEBUTTONUP:   script_click(e.button.button,  0);    break;
        case SDL_KEYDOWN:         script_keybd(e.key.keysym.sym, 1);    break;
        case SDL_KEYUP:           script_keybd(e.key.keysym.sym, 0);    break;

        case SDL_QUIT:
            server_send_exit();
            return 0;
        }

    server_draw();

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
                SDL_WM_SetCaption(TITLE, TITLE);

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
