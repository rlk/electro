#include <SDL.h>
#include <iostream>

#include "opengl.hpp"

//-----------------------------------------------------------------------------

static bool loop()
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_QUIT: return false;
        }

    return true;
}

static void draw()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

static void flip()
{
    SDL_GL_SwapBuffers();
}

//-----------------------------------------------------------------------------

void server(int w, int h)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0)
    {
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        if (SDL_SetVideoMode(w, h, 0, SDL_OPENGL))
            while (loop())
            {
                draw();
                flip();
            }

        SDL_Quit();
    }
}

//-----------------------------------------------------------------------------

