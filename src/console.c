/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    ELECTRO is free software;  you can redistribute it and/or modify it    */
/*    under the terms of the  GNU General Public License  as published by    */
/*    the  Free Software Foundation;  either version 2 of the License, or    */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "opengl.h"
#include "version.h"
#include "utility.h"
#include "display.h"
#include "console.h"
#include "glyph.h"
#include "script.h"
#include "vector.h"

/*---------------------------------------------------------------------------*/

#define IMG_W 512
#define IMG_H 512

#define MAXHST 128

#define AUTO_DELAY  500
#define AUTO_REPEAT  50

static vector_t history;

static char command[MAXSTR];
static int  history_i = 0;
static int  command_i = 0;

static int console_enable = 0;
static int image_dirty    = 0;

static int console_w;
static int console_h;
static int console_x;
static int console_y;

static int image_w;
static int image_h;

static unsigned char console_r = 0x00;
static unsigned char console_g = 0xFF;
static unsigned char console_b = 0x00;

static unsigned char *console;
static unsigned char *image;

#define CONS_C(i, j) console[(console_w * i + j) * 4]
#define CONS_R(i, j) console[(console_w * i + j) * 4 + 1]
#define CONS_G(i, j) console[(console_w * i + j) * 4 + 2]
#define CONS_B(i, j) console[(console_w * i + j) * 4 + 3]

/*---------------------------------------------------------------------------*/

static GLubyte back_image[16] = {
    0x00, 0x10, 0x00, 0xC0, 0x00, 0x10, 0x00, 0xC0, 
    0x00, 0x10, 0x00, 0xB0, 0x00, 0x10, 0x00, 0xB0, 
};

static GLuint  fore_texture;
static GLuint  back_texture;

/*---------------------------------------------------------------------------*/

static void scroll_console(void)
{
    memmove(console + console_w * 4, console, console_w * (console_h - 1) * 4);
    memset(console, 0, console_w * 4);
}

static void write_console(const char *str)
{
    int i, l = strlen(str);

    for (i = 0; i < l; i++)
    {
        /* Scroll on newline. */

        if (str[i] == '\n')
        {
            scroll_console();
            console_x = 0;
        }

        /* Write an printable character to the buffer. */

        if (str[i] >= 32)
        {
            if (0 <= console_x && console_x < console_w &&
                0 <= console_y && console_y < console_h)
            {
                CONS_C(console_y, console_x) = str[i];
                CONS_R(console_y, console_x) = console_r;
                CONS_G(console_y, console_x) = console_g;
                CONS_B(console_y, console_x) = console_b;
            }
            console_x = console_x + 1;
        }

        /* Scroll on line wrap. */

        if (console_x >= console_w)
        {
            scroll_console();
            console_x = 0;
        }
    }
}

/*---------------------------------------------------------------------------*/

int set_console_enable(int b)
{
    /*
    SDL_EnableKeyRepeat(b ? SDL_DEFAULT_REPEAT_DELAY    : 0,
                        b ? SDL_DEFAULT_REPEAT_INTERVAL : 0);
    */
    console_enable = b;
    return 1;
}

int console_is_enabled(void)
{
    return console_enable;
}

/*---------------------------------------------------------------------------*/

static void init_fore_image(void)
{
    /* Create the console foreground texture, if necessary. */

    if (glIsTexture(fore_texture))
        glBindTexture(GL_TEXTURE_2D, fore_texture);
    else
    {
        glGenTextures(1, &fore_texture);
        glBindTexture(GL_TEXTURE_2D, fore_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, 4, IMG_W, IMG_H, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, image);
    }
}

static void init_back_image(void)
{
    /* Create the console background texture, if necessary. */

    if (glIsTexture(back_texture))
        glBindTexture(GL_TEXTURE_2D, back_texture);
    else
    {
        glGenTextures(1, &back_texture);
        glBindTexture(GL_TEXTURE_2D, back_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, 4, 2, 2, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, back_image);
    }
}

/*---------------------------------------------------------------------------*/

static void draw_image(void)
{
    int r, c;
    int i, j;

    init_fore_image();

    /* This would be a lot more efficient using glBitmap to draw right   */
    /* to the framebuffer, or glTexSubImage2D with pixel transfer bias   */
    /* to transfer glyphs straight to texture memory, but both of those  */
    /* are commonly buggy on crappy hardware.  Lame.                     */

    for (r = 0; r < console_h; r++)
        for (c = 0; c < console_w; c++)
        {
            const GLubyte *p = glyph[CONS_C(r, c) ? CONS_C(r, c) - 32 : 0];

            for (i = 0; i < GLYPH_H; i++)
                for (j = 0; j < GLYPH_W; j++)
                {
                    const int x = GLYPH_W * c + j;
                    const int y = GLYPH_H * r + i;
                    const int s = GLYPH_W * i + j;
                    const int d = image_w * y + x;

                    image[d * 4 + 0] = CONS_R(r, c);
                    image[d * 4 + 1] = CONS_G(r, c);
                    image[d * 4 + 2] = CONS_B(r, c);
                    image[d * 4 + 3] = p[s];
                }
        }

    /* Write the contents of the console image to the console texture. */

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_w, image_h,
                    GL_RGBA, GL_UNSIGNED_BYTE, image);

    image_dirty = 0;
}

/*---------------------------------------------------------------------------*/

int startup_console(int w, int h)
{
    int W = IMG_W;
    int H = IMG_H;

    if ((console = (unsigned char *) calloc(w * h * 4, 1)) &&
        (image   = (unsigned char *) calloc(W * H * 4, 1)))
    {
        history = vecnew(256, sizeof (char *));

        console_w = w;
        console_h = h;
        console_x = 0;
        console_y = 0;

        image_w = W;
        image_h = H;

        color_console(1.0f, 1.0f, 0.0f);

        print("  |||  ELECTRO %s\n", version());
        print("  O o  Copyright (C) 2005  Robert Kooima \n");
        print("   -   http://www.evl.uic.edu/rlk/electro\n");

        color_console(0.0f, 1.0f, 0.0f);

        console_enable = 0;
        return 1;
    }
    return 0;
}

void draw_console(void)
{
    if (console_enable)
    {
        if (image_dirty)
            draw_image();

        glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);
        {
            int x = CONSOLE_X;
            int y = CONSOLE_Y;
            int w = CONSOLE_COLS * GLYPH_W;
            int h = CONSOLE_ROWS * GLYPH_H;

            float s = (float) w / IMG_W;
            float t = (float) h / IMG_H;

            /* Set the GL state for rendering the console. */

            glEnable(GL_COLOR_MATERIAL);
            glDisable(GL_SCISSOR_TEST);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glEnable(GL_TEXTURE_2D);

            /* Apply a pixel-for-pixel transformation. */

            glMatrixMode(GL_PROJECTION);
            {
                glLoadIdentity();
                glOrtho(0, get_window_w(),
                        0, get_window_h(), -1, +1);
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glLoadIdentity();
            }

            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            /* Draw the console background. */

            init_back_image();

            glBegin(GL_QUADS);
            {
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

                glTexCoord2i(0,     0);     glVertex2i(x,     y);
                glTexCoord2i(w / 2, 0);     glVertex2i(x + w, y);
                glTexCoord2i(w / 2, h / 2); glVertex2i(x + w, y + h);
                glTexCoord2i(0,     h / 2); glVertex2i(x,     y + h);
            }
            glEnd();

            /* Draw the console cursor. */

            glBindTexture(GL_TEXTURE_2D, 0);

            glBegin(GL_QUADS);
            {
                glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

                glVertex2i(CONSOLE_X + GLYPH_W *  console_x,
                           CONSOLE_Y + GLYPH_H *  console_y);
                glVertex2i(CONSOLE_X + GLYPH_W * (console_x + 1),
                           CONSOLE_Y + GLYPH_H *  console_y);
                glVertex2i(CONSOLE_X + GLYPH_W * (console_x + 1),
                           CONSOLE_Y + GLYPH_H * (console_y + 1));
                glVertex2i(CONSOLE_X + GLYPH_W *  console_x,
                           CONSOLE_Y + GLYPH_H * (console_y + 1));
            }
            glEnd();

            /* Draw the console text. */

            init_fore_image();

            glBegin(GL_QUADS);
            {
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

                glTexCoord2f(0, 0); glVertex2i(x,     y);
                glTexCoord2f(s, 0); glVertex2i(x + w, y);
                glTexCoord2f(s, t); glVertex2i(x + w, y + h);
                glTexCoord2f(0, t); glVertex2i(x,     y + h);
            }
            glEnd();
        }
        glPopAttrib();
    }
}

/*---------------------------------------------------------------------------*/

static void refresh_command(void)
{
    unsigned char r = console_r;
    unsigned char g = console_g;
    unsigned char b = console_b;

    int i;

    color_console(1.0f, 1.0f, 1.0f);

    /* Print the command at the beginning of the line. */

    console_x = 0;

    write_console(command);

    /* Clear to the end of the line. */

    for (i = console_x; i < console_w; i++)
        CONS_C(console_y, i) = 0;

    console_x = command_i;
    console_r = r;
    console_g = g;
    console_b = b;
}

/*---------------------------------------------------------------------------*/

int input_console(int symbol, int unicode)
{
    if (symbol == SDLK_RETURN)                      /* Execute the command. */
    {
        refresh_command();
        scroll_console();
        console_x = 0;

        if (command_i > 0)
        {
            *((char **) vecget(history, vecadd(history))) =
              memdup(command, strlen(command) + 1, 1);

            command_i = 0;
            history_i = vecnum(history);

            do_command(command);

            memset(command, 0, MAXSTR);
        }
    }

    else if (symbol == SDLK_LEFT || unicode == 2)   /* Cursor moves left. */
    {
        if (command_i > 0)
        {
            command_i--;
            console_x--;
        }
    }

    else if (symbol == SDLK_RIGHT || unicode == 6)  /* Cursor moves right. */
    {
        if (command[command_i])
        {
            command_i++;
            console_x++;
        }
    }

    else if (symbol == SDLK_HOME || unicode == 1)   /* Home */
        console_x = command_i = 0;

    else if (symbol == SDLK_END || unicode == 5)    /* End */
        console_x = command_i = strlen(command);

    else if (unicode == 11)                         /* Kill */
    {
        memset(command + command_i, 0, MAXSTR - command_i - 1);
        refresh_command();
    }

    else if (symbol == SDLK_UP || unicode == 16)    /* Previous history. */
    {
        if (history_i > 0)
            history_i--;
        if (history_i < vecnum(history))
            strncpy(command, *((char **) vecget(history, history_i)), MAXSTR);

        command_i = strlen(command);
        refresh_command();
    }

    else if (symbol == SDLK_DOWN || unicode == 14)  /* Next history. */
    {
        if (history_i < vecnum(history) - 1)
            history_i++;
        if (history_i < vecnum(history))
            strncpy(command, *((char **) vecget(history, history_i)), MAXSTR);

        command_i = strlen(command);
        refresh_command();
    }

    else if (symbol == SDLK_DELETE ||
             symbol == SDLK_BACKSPACE)              /* Delete a character. */
    {
        if (command_i > 0)
        {
            memmove(command + command_i - 1,
                    command + command_i, MAXSTR - command_i - 1);

            command_i--;
            refresh_command();
        }
    }
    
    else if (isprint(unicode))                      /* Insert a character. */
    {
        if (command_i < MAXSTR && console_x < console_w)
        {
            memmove(command + command_i + 1,
                    command + command_i, MAXSTR - command_i - 1);
            command[command_i] = (char) unicode;

            command_i++;
            refresh_command();
        }
    }

    image_dirty = 1;

    return image_dirty;
}

/*---------------------------------------------------------------------------*/

void clear_console(void)
{
    memset(console, 0, console_w * console_h * 4);
    image_dirty = 1;
}

void color_console(float r, float g, float b)
{
    console_r = (unsigned char) (r * 255.0);
    console_g = (unsigned char) (g * 255.0);
    console_b = (unsigned char) (b * 255.0);
}

void print_console(const char *str)
{
    write_console(str);
    console_enable = 1;
    image_dirty    = 1;
}

void error_console(const char *str)
{
    unsigned char r = console_r;
    unsigned char g = console_g;
    unsigned char b = console_b;

    color_console(1.0f, 0.0f, 0.0f);

    write_console("Error: ");
    write_console(str);
    write_console("\n");
    
#ifndef NDEBUG
    fprintf(stderr, "Error: %s\n", str);
#endif

    console_r = r;
    console_g = g;
    console_b = b;
    console_enable = 1;
    image_dirty    = 1;
}

/*---------------------------------------------------------------------------*/
