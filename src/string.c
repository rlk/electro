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
#include "font.h"
#include "string.h"

/*---------------------------------------------------------------------------*/

struct string
{
    int   count;
    int   font;
    char *text;
    float color[4];
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
    int i;

    if ((i = new_string()) >= 0)
    {
        send_event(EVENT_CREATE_STRING);

        S(i)->text     = memdup(text, strlen(text) + 1, 1);
        S(i)->font     = get_font();
        S(i)->count    = 1;
        S(i)->color[0] = 1.0f;
        S(i)->color[1] = 1.0f;
        S(i)->color[2] = 1.0f;
        S(i)->color[3] = 1.0f;

        return send_create_entity(TYPE_STRING, i);
    }
    return -1;
}

void recv_create_string(void)
{
    int i = new_string();

    S(i)->count    = 1;
    S(i)->color[0] = 1.0f;
    S(i)->color[1] = 1.0f;
    S(i)->color[2] = 1.0f;
    S(i)->color[3] = 1.0f;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_string_color(int i, float r, float g, float b)
{
    send_event(EVENT_SET_STRING_COLOR);
    send_index(i);

    send_float((S(i)->color[0] = r));
    send_float((S(i)->color[1] = g));
    send_float((S(i)->color[2] = b));
}

void recv_set_string_color(void)
{
    int i = recv_index();

    S(i)->color[0] = recv_float();
    S(i)->color[1] = recv_float();
    S(i)->color[2] = recv_float();
}

void send_set_string_value(int i, const char *text)
{
    int n = strlen(text) + 1;

    send_event(EVENT_SET_STRING_VALUE);
    send_index(i);
    send_index(n);
    send_array(text, n, 1);

    if (S(i)->text)
        free(S(i)->text);

    if ((S(i)->text = (char *) malloc(n)))
        strcpy(S(i)->text, text);
}

void recv_set_string_value(void)
{
    int i = recv_index();
    int n = recv_index();

    if (S(i)->text)
        free(S(i)->text);

    if ((S(i)->text = (char *) malloc(n)))
        recv_array(S(i)->text, n, 1);
}

/*===========================================================================*/

static void init_string(int i)
{
    init_font(S(i)->font);
}

static void fini_string(int i)
{
    fini_font(S(i)->font);
}

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

        glColor4f(S(i)->color[0],
                  S(i)->color[1],
                  S(i)->color[2], a * get_entity_alpha(j));
        draw_font(S(i)->font, S(i)->text);

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
    init_string,
    fini_string,
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
