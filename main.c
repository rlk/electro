#include <SDL.h>

#include "opengl.h"
#include "viewer.h"
#include "galaxy.h"

/*---------------------------------------------------------------------------*/

static void init(void)
{
    viewer_init();
    galaxy_init();
}

static void draw(void)
{
    double p[3];

    viewer_get_pos(p);

    glClear(GL_COLOR_BUFFER_BIT);

    viewer_draw();
    galaxy_draw(p);

    SDL_GL_SwapBuffers();
}

/*---------------------------------------------------------------------------*/

static int loop(void)
{
    SDL_Event e;
    int c = 0;

    while (SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_MOUSEBUTTONDOWN:
            c += viewer_click(e.button.button, 1);
            break;

        case SDL_MOUSEBUTTONUP:
            c += viewer_click(e.button.button, 0);
            break;

        case SDL_MOUSEMOTION:
            c += viewer_point(e.motion.x, e.motion.y);
            break;

        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_ESCAPE) return 1;
            break;

        case SDL_QUIT:
            return 1;
        }

    if (c) draw();

    return 0;
}

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        if (SDL_SetVideoMode(WIN_W, WIN_H, 0, SDL_OPENGL))
        {
            init();
            draw();

            while (SDL_WaitEvent(NULL))
                if (loop())
                    break;
        }
        SDL_Quit();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
