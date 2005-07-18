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

#include <stdlib.h>
#include <string.h>

#include "opengl.h"
#include "utility.h"
#include "vector.h"
#include "buffer.h"
#include "entity.h"
#include "event.h"
#include "brush.h"
#include "font.h"
#include "string.h"

/*---------------------------------------------------------------------------*/

struct string
{
    int   count;
    int   font;
    char *text;
    int   fill;
    int   line;
};

static vector_t string;

/*---------------------------------------------------------------------------*/

#define S(i) ((struct string *) vecget(string, i))

static int new_string(void)
{
    int i, n = vecnum(string);

    for (i = 0; i < n; ++i)
        if (S(i)->count == 0)
            return i;

    return vecadd(string);
}

/*===========================================================================*/

int send_create_string(const char *text)
{
    int i, n = strlen(text) + 1;

    if ((i = new_string()) >= 0)
    {
        send_event(EVENT_CREATE_STRING);

        S(i)->text  = memdup(text, n, 1);
        S(i)->font  = get_font();
        S(i)->count = 1;
        S(i)->fill  = 0;
        S(i)->line  = 0;

        send_index(n);
        send_array(text, n, 1);
        send_index(S(i)->font);

        return send_create_entity(TYPE_STRING, i);
    }
    return -1;
}

void recv_create_string(void)
{
    int i = new_string();
    int n = recv_index();

    S(i)->text = (char *) malloc(n);
    recv_array(S(i)->text, n, 1);

    S(i)->font  = recv_index();
    S(i)->count = 1;
    S(i)->fill  = 0;
    S(i)->line  = 0;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_string_fill(int i, int j)
{
    send_event(EVENT_SET_STRING_FILL);
    send_index(i);
    send_index((S(i)->fill = j));
}

void recv_set_string_fill(void)
{
    int i      = recv_index();
    S(i)->fill = recv_index();
}

/*---------------------------------------------------------------------------*/

void send_set_string_line(int i, int j)
{
    send_event(EVENT_SET_STRING_LINE);
    send_index(i);
    send_index((S(i)->line = j));
}

void recv_set_string_line(void)
{
    int i      = recv_index();
    S(i)->line = recv_index();
}

/*---------------------------------------------------------------------------*/

void send_set_string_text(int i, const char *text)
{
    int n = strlen(text) + 1;

    send_event(EVENT_SET_STRING_TEXT);
    send_index(i);
    send_index(n);
    send_array(text, n, 1);

    if (S(i)->text)
        free(S(i)->text);

    if ((S(i)->text = (char *) malloc(n)))
        strcpy(S(i)->text, text);
}

void recv_set_string_text(void)
{
    int i = recv_index();
    int n = recv_index();

    if (S(i)->text)
        free(S(i)->text);

    if ((S(i)->text = (char *) malloc(n)))
        recv_array(S(i)->text, n, 1);
}

/*===========================================================================*/

static int bbox_string(int i, float bound[6])
{
    bbox_font(S(i)->font, S(i)->text, bound);
    return 1;
}

static void draw_string(int j, int i, int f, float a)
{
    glPushMatrix();
    {
        transform_entity(j);

        glPushAttrib(GL_DEPTH_BUFFER_BIT);
        {
            glDepthMask(GL_FALSE);

            draw_brush(S(i)->fill, a * get_entity_alpha(j));
            draw_font (S(i)->font, S(i)->text, 0);

            draw_brush(S(i)->line, a * get_entity_alpha(j));
            draw_font (S(i)->font, S(i)->text, 1);
        }
        glPopAttrib();

        draw_entity_tree(j, f, a * get_entity_alpha(j));
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

static void dupe_string(int i)
{
    S(i)->count++;
}

static void free_string(int i)
{
    if (--S(i)->count == 0)
        memset(S(i), 0, sizeof (struct string));
}

/*---------------------------------------------------------------------------*/

static struct entity_func string_func = {
    "string",
    NULL,
    NULL,
    bbox_string,
    draw_string,
    dupe_string,
    free_string,
};

struct entity_func *startup_string(void)
{
    if ((string = vecnew(MIN_STRINGS, sizeof (struct string))))
        return &string_func;
    else
        return NULL;
}
