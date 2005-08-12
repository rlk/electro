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

    float aabb_cache[6];
    int   aabb_state;
};

static vector_t string;

/*---------------------------------------------------------------------------*/

static struct string *get_string(int i)
{
    return (struct string *) vecget(string, i);
}

static int new_string(void)
{
    int i, n = vecnum(string);

    for (i = 0; i < n; ++i)
        if (get_string(i)->count == 0)
            return i;

    return vecadd(string);
}

/*===========================================================================*/

int send_create_string(const char *text)
{
    int i, n = strlen(text) + 1;

    if ((i = new_string()) >= 0)
    {
        struct string *s = get_string(i);

        send_event(EVENT_CREATE_STRING);

        s->text  = memdup(text, n, 1);
        s->font  = get_font();
        s->count = 1;
        s->fill  = 0;
        s->line  = 0;

        send_index(n);
        send_array(text, n, 1);
        send_index(s->font);

        return send_create_entity(TYPE_STRING, i);
    }
    return -1;
}

void recv_create_string(void)
{
    int i = new_string();
    int n = recv_index();

    struct string *s = get_string(i);

    s->text = (char *) malloc(n);
    recv_array(s->text, n, 1);

    s->font  = recv_index();
    s->count = 1;
    s->fill  = 0;
    s->line  = 0;

    recv_create_entity();
}

/*---------------------------------------------------------------------------*/

void send_set_string_fill(int i, int j)
{
    struct string *s = get_string(i);

    dupe_create_brush(j);
    send_delete_brush(s->fill);

    send_event(EVENT_SET_STRING_FILL);
    send_index(i);
    send_index((s->fill = j));
}

void recv_set_string_fill(void)
{
    int i = recv_index();
    int j = recv_index();

    get_string(i)->fill = j;
}

/*---------------------------------------------------------------------------*/

void send_set_string_line(int i, int j)
{
    struct string *s = get_string(i);

    dupe_create_brush(j);
    send_delete_brush(s->line);

    send_event(EVENT_SET_STRING_LINE);
    send_index(i);
    send_index((s->line = j));
}

void recv_set_string_line(void)
{
    int i = recv_index();
    int j = recv_index();

    get_string(i)->line = j;
}

/*---------------------------------------------------------------------------*/

void send_set_string_text(int i, const char *text)
{
    struct string *s = get_string(i);
    int n = strlen(text) + 1;

    send_event(EVENT_SET_STRING_TEXT);
    send_index(i);
    send_index(n);
    send_array(text, n, 1);

    if (s->text)
        free(s->text);

    if ((s->text = (char *) malloc(n)))
        strcpy(s->text, text);
}

void recv_set_string_text(void)
{
    int i = recv_index();
    int n = recv_index();

    struct string *s = get_string(i);

    if (s->text)
        free(s->text);

    if ((s->text = (char *) malloc(n)))
        recv_array(s->text, n, 1);
}

/*===========================================================================*/

static void aabb_string(int i, float aabb[6])
{
    struct string *s = get_string(i);

    if (s->aabb_state == 0)
    {
        aabb_font(s->font, s->text, s->aabb_cache);
        s->aabb_state = 1;
    }

    aabb[0] = s->aabb_cache[0];
    aabb[1] = s->aabb_cache[1];
    aabb[2] = s->aabb_cache[2];
    aabb[3] = s->aabb_cache[3];
    aabb[4] = s->aabb_cache[4];
    aabb[5] = s->aabb_cache[5];
}

static void draw_string(int j, int i, int f, float a)
{
    struct string *s = get_string(i);

    glPushMatrix();
    {
        transform_entity(j);

        glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
        {
            glDepthMask(GL_FALSE);

            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);

            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

            draw_brush(s->fill, a * get_entity_alpha(j));
            draw_font (s->font, s->text, 0);

            draw_brush(s->line, a * get_entity_alpha(j));
            draw_font (s->font, s->text, 1);
        }
        glPopAttrib();

        draw_entity_tree(j, f, a * get_entity_alpha(j));
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

static void dupe_string(int i)
{
    get_string(i)->count++;
}

static void free_string(int i)
{
    struct string *s = get_string(i);

    if (--s->count <= 0)
    {
        send_delete_brush(s->fill);
        send_delete_brush(s->line);

        memset(s, 0, sizeof (struct string));
    }
}

/*---------------------------------------------------------------------------*/

static struct entity_func string_func = {
    "string",
    NULL,
    NULL,
    aabb_string,
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
