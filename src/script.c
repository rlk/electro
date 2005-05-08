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

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <SDL.h>
#include <lua.h>
#include <lualib.h>
#include <stdlib.h>
#include <string.h>

#include "joystick.h"
#include "display.h"
#include "console.h"
#include "server.h"
#include "camera.h"
#include "stereo.h"
#include "sprite.h"
#include "object.h"
#include "galaxy.h"
#include "light.h"
#include "pivot.h"
#include "utility.h"
#include "entity.h"
#include "sound.h"
#include "script.h"

/*---------------------------------------------------------------------------*/

static lua_State *L;

/*---------------------------------------------------------------------------*/
/* Script readers                                                            */

static const char *filereader(lua_State *L, void *data, size_t *size)
{
    static char buffer[MAXSTR];

    memset(buffer, 0, MAXSTR);

    if ((*size = fread(buffer, 1, MAXSTR - 1, (FILE *) data)) > 0)
        return buffer;
    else
        return NULL;
}

static const char *charreader(lua_State *L, void *data, size_t *size)
{
    static char buffer[MAXSTR];

    memset(buffer, 0, MAXSTR);

    if ((*size = strlen((char *) data)))
    {
        strncpy(buffer, data, MAXSTR);
        memset(data, 0, *size);
        return buffer;
    }
    else
        return NULL;
}

/*---------------------------------------------------------------------------*/
/* Generic userdata handlers                                                 */

#define USERDATA_ENTITY 0
#define USERDATA_SOUND  1
#define USERDATA_IMAGE  2

static int lua_touserdata_type(lua_State *L, int i)
{
    return ((int *) lua_touserdata(L, i))[0];
}

static int lua_touserdata_data(lua_State *L, int i)
{
    return ((int *) lua_touserdata(L, i))[1];
}

static void lua_pushuserdata(lua_State *L, int type, int data)
{
    int *p = (int *) lua_newuserdata(L, 2 * sizeof (int));

    p[0] = type;
    p[1] = data;
}

/*---------------------------------------------------------------------------*/
/* Entity userdata handlers                                                  */

static int lua_toentity(lua_State *L, int i)
{
    return lua_touserdata_data(L, i);
}

static int lua_isentity(lua_State *L, int i)
{
    return ((lua_isuserdata(L, i)) &&
            (lua_touserdata_type(L, i) == USERDATA_ENTITY));
}

static void lua_pushentity(lua_State *L, int id)
{
    if (id <= 0)
        lua_pushnil(L);
    else
        lua_pushuserdata(L, USERDATA_ENTITY, id);
}

/*---------------------------------------------------------------------------*/
/* Sound userdata handlers                                                   */

static int lua_tosound(lua_State *L, int i)
{
    return lua_touserdata_data(L, i);
}

static int lua_issound(lua_State *L, int i)
{
    return ((lua_isuserdata(L, i)) &&
            (lua_touserdata_type(L, i) == USERDATA_SOUND));
}

static void lua_pushsound(lua_State *L, int id)
{
    if (id < 0)
        lua_pushnil(L);
    else
        lua_pushuserdata(L, USERDATA_SOUND, id);
}

/*---------------------------------------------------------------------------*/
/* Function argument error reporters                                         */

static void script_type_error(const char *type, lua_State *L, int i)
{
    const char *name = lua_tostring(L, lua_upvalueindex(1));
    const char *got  = "unknown";

    if (lua_isuserdata(L, i))
        got = get_entity_type_name(lua_toentity(L, i));
    else
        got = lua_typename(L, lua_type(L, i));

    error("'%s' expected %s, got %s", name, type, got);
}

static void script_arity_error(lua_State *L, int i)
{
    const char *name = lua_tostring(L, lua_upvalueindex(1));

    error("'%s' expected %d parameters, got %d", name, -i, lua_gettop(L));
}

/*---------------------------------------------------------------------------*/
/* Type checking functions                                                   */

static int lua_iscamera(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(lua_toentity(L, i)) == TYPE_CAMERA);
}

static int lua_isgalaxy(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(lua_toentity(L, i)) == TYPE_GALAXY);
}

static int lua_issprite(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(lua_toentity(L, i)) == TYPE_SPRITE);
}

static int lua_islight(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(lua_toentity(L, i)) == TYPE_LIGHT);
}

/*---------------------------------------------------------------------------*/
/* Function argument type and arity checkers                                 */

static const char *script_getstring(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isstring(L, i))
            return lua_tostring(L, i);
        else
            script_type_error("string", L, i);
    }
    else script_arity_error(L, i);

    return "";
}

static int script_getboolean(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isboolean(L, i))
            return lua_toboolean(L, i);
        else
            script_type_error("boolean", L, i);
    }
    else script_arity_error(L, i);

    return 0;
}

static float script_getnumber(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isnumber(L, i))
            return (float) lua_tonumber(L, i);
        else
            script_type_error("number", L, i);
    }
    else script_arity_error(L, i);

    return 0.0;
}

static void script_getvector(lua_State *L, int i, float *v, int n)
{
    int j;

    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_istable(L, i))
        {
            for (j = 0; j < n; j++)
            {
                lua_pushnumber(L, j + 1);
                lua_gettable  (L, i - 1);

                if (lua_isnumber(L, -1))
                    v[j] = (float) lua_tonumber(L, -1);
                else
                {
                    script_type_error("number", L, -1);
                    break;
                }

                lua_pop(L, 1);
            }
        }
        else script_type_error("vector", L, i);
    }
    else script_arity_error(L, i);
}

/*---------------------------------------------------------------------------*/

static int script_getentity(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isnil   (L, i)) return 0;
        if (lua_isentity(L, i)) return lua_toentity(L, i);

        script_type_error("entity", L, i);
    }
    else script_arity_error(L, i);

    return 0;
}

static int script_getcamera(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_iscamera(L, i))
            return entity_data(lua_toentity(L, i));
        else
            script_type_error("camera", L, i);
    }
    else script_arity_error(L, i);

    return 0;
}

static int script_getgalaxy(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isgalaxy(L, i))
            return entity_data(lua_toentity(L, i));
        else
            script_type_error("galaxy", L, i);
    }
    else script_arity_error(L, i);

    return 0;
}

static int script_getsprite(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_issprite(L, i))
            return entity_data(lua_toentity(L, i));
        else
            script_type_error("sprite", L, i);
    }
    else script_arity_error(L, i);

    return 0;
}

static int script_getlight(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_islight(L, i))
            return entity_data(lua_toentity(L, i));
        else
            script_type_error("light", L, i);
    }
    else script_arity_error(L, i);

    return 0;
}

static int script_getsound(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_issound(L, i))
            return lua_tosound(L, i);
        else
            script_type_error("sound", L, i);
    }
    else script_arity_error(L, i);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* Display configuration                                                     */

static int script_add_host(lua_State *L)
{
    add_host(script_getstring(L, -5),
       (int) script_getnumber(L, -4),
       (int) script_getnumber(L, -3),
       (int) script_getnumber(L, -2),
       (int) script_getnumber(L, -1));

    return 0;
}

static int script_add_tile(lua_State *L)
{
    const char *n = script_getstring(L, -12);
    int   x = (int) script_getnumber(L, -11);
    int   y = (int) script_getnumber(L, -10);
    int   w = (int) script_getnumber(L, -9);
    int   h = (int) script_getnumber(L, -8);
    int   X = (int) script_getnumber(L, -7);
    int   Y = (int) script_getnumber(L, -6);
    int   W = (int) script_getnumber(L, -5);
    int   H = (int) script_getnumber(L, -4);

    float p[3][3];

    script_getvector(L, -3, p[0], 3);
    script_getvector(L, -2, p[1], 3);
    script_getvector(L, -1, p[2], 3);

    add_tile(n, x, y, w, h,
                X, Y, W, H, p);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* Miscellaneous functions                                                   */

static int script_enable_timer(lua_State *L)
{
    enable_timer(script_getboolean(L, -1));
    return 0;
}

static int script_get_joystick(lua_State *L)
{
    float a[2];

    get_joystick((int) script_getnumber(L, -1), a);

    lua_pushnumber(L, (double) a[0]);
    lua_pushnumber(L, (double) a[1]);
    return 2;
}

static int script_get_viewport(lua_State *L)
{
    lua_pushnumber(L, (float) get_viewport_x());
    lua_pushnumber(L, (float) get_viewport_y());
    lua_pushnumber(L, (float) get_viewport_w());
    lua_pushnumber(L, (float) get_viewport_h());

    return 4;
}

static int script_get_modifier(lua_State *L)
{
    int i = (int) script_getnumber(L, -1);

    lua_pushboolean(L, (SDL_GetModState() & i) ? 1 : 0);

    return 1;
}

static int script_set_background(lua_State *L)
{
    int n = lua_gettop(L);
    float c0[3];
    float c1[3];

    if (n == 6)
    {
        c0[0] = script_getnumber(L, -6);
        c0[1] = script_getnumber(L, -5);
        c0[2] = script_getnumber(L, -4);
        c1[0] = script_getnumber(L, -3);
        c1[1] = script_getnumber(L, -2);
        c1[2] = script_getnumber(L, -1);

        send_set_background(c0, c1);
    }
    else
    {
        c0[0] = script_getnumber(L, -3);
        c0[1] = script_getnumber(L, -2);
        c0[2] = script_getnumber(L, -1);

        send_set_background(c0, c0);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Entity hierarchy functions                                                */

static int script_parent_entity(lua_State *L)
{
    send_parent_entity(script_getentity(L, -2),
                       script_getentity(L, -1));
    return 0;
}

static int script_delete_entity(lua_State *L)
{
    send_delete_entity(script_getentity(L, -1));
    return 0;
}

static int script_create_clone(lua_State *L)
{
    int id = send_create_clone(script_getentity(L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_get_entity_parent(lua_State *L)
{
    int id = get_entity_parent(script_getentity(L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_get_entity_child(lua_State *L)
{
    int id = get_entity_child(script_getentity(L, -2),
                        (int) script_getnumber(L, -1));

    lua_pushentity(L, id);
    return 1;
}

/*---------------------------------------------------------------------------*/
/* Entity transform functions                                                */

static int script_set_entity_position(lua_State *L)
{
    float p[3];

    p[0] = script_getnumber(L, -3);
    p[1] = script_getnumber(L, -2);
    p[2] = script_getnumber(L, -1);

    send_set_entity_position(script_getentity(L, -4), p);
    return 0;
}

static int script_set_entity_rotation(lua_State *L)
{
    float r[3];

    r[0] = script_getnumber(L, -3);
    r[1] = script_getnumber(L, -2);
    r[2] = script_getnumber(L, -1);

    send_set_entity_rotation(script_getentity(L, -4), r);
    return 0;
}

static int script_set_entity_scale(lua_State *L)
{
    float s[3];

    s[0] = script_getnumber(L, -3);
    s[1] = script_getnumber(L, -2);
    s[2] = script_getnumber(L, -1);

    send_set_entity_scale(script_getentity(L, -4), s);
    return 0;
}

static int script_set_entity_alpha(lua_State *L)
{
    send_set_entity_alpha(script_getentity(L, -2),
                          script_getnumber(L, -1));
    return 0;
}

static int script_set_entity_flag(lua_State *L)
{
    send_set_entity_flag(script_getentity(L, -3),
                   (int) script_getnumber(L, -2),
                         script_getboolean(L, -1));
    return 0;
}

static int script_set_entity_frag_prog(lua_State *L)
{
    int id           = script_getentity(L, -2);
    const char *file = script_getstring(L, -1);

    char *text = alloc_text(file);

    send_set_entity_frag_prog(id, text);

    free(text);
    return 0;
}

static int script_set_entity_vert_prog(lua_State *L)
{
    int id           = script_getentity(L, -2);
    const char *file = script_getstring(L, -1);

    char *text = alloc_text(file);

    send_set_entity_vert_prog(id, text);

    free(text);
    return 0;
}

/*---------------------------------------------------------------------------*/

static int script_move_entity(lua_State *L)
{
    float v[3];

    v[0] = script_getnumber(L, -3);
    v[1] = script_getnumber(L, -2);
    v[2] = script_getnumber(L, -1);

    send_move_entity(script_getentity(L, -4), v);
    return 0;
}

static int script_turn_entity(lua_State *L)
{
    float r[3];

    r[0] = script_getnumber(L, -3);
    r[1] = script_getnumber(L, -2);
    r[2] = script_getnumber(L, -1);

    send_turn_entity(script_getentity(L, -4), r);
    return 0;
}

/*---------------------------------------------------------------------------*/

static int script_get_entity_position(lua_State *L)
{
    int id = script_getentity(L, -1);
    float p[3];

    get_entity_position(id, p);

    lua_pushnumber(L, p[0]);
    lua_pushnumber(L, p[1]);
    lua_pushnumber(L, p[2]);

    return 3;
}

static int script_get_entity_x_vector(lua_State *L)
{
    int id = script_getentity(L, -1);
    float v[3];

    get_entity_x_vector(id, v);

    lua_pushnumber(L, v[0]);
    lua_pushnumber(L, v[1]);
    lua_pushnumber(L, v[2]);

    return 3;
}

static int script_get_entity_y_vector(lua_State *L)
{
    int id = script_getentity(L, -1);
    float v[3];

    get_entity_y_vector(id, v);

    lua_pushnumber(L, v[0]);
    lua_pushnumber(L, v[1]);
    lua_pushnumber(L, v[2]);

    return 3;
}

static int script_get_entity_z_vector(lua_State *L)
{
    int id = script_getentity(L, -1);
    float v[3];

    get_entity_z_vector(id, v);

    lua_pushnumber(L, v[0]);
    lua_pushnumber(L, v[1]);
    lua_pushnumber(L, v[2]);

    return 3;
}

static int script_get_entity_scale(lua_State *L)
{
    int id = script_getentity(L, -1);
    float s[3];

    get_entity_scale(id, s);

    lua_pushnumber(L, s[0]);
    lua_pushnumber(L, s[1]);
    lua_pushnumber(L, s[2]);

    return 3;
}

static int script_get_entity_alpha(lua_State *L)
{
    int  id = script_getentity(L, -1);
    float a = get_entity_alpha(id);

    lua_pushnumber(L, a);

    return 1;
}

/*---------------------------------------------------------------------------*/
/* Entity constructors.                                                      */

static int script_create_camera(lua_State *L)
{
    int fl = (int) script_getnumber(L, -1);
    int id = send_create_camera(fl);

    lua_pushentity(L, id);
    return 1;
}

static int script_create_sprite(lua_State *L)
{
    int id = send_create_sprite(script_getstring(L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_create_object(lua_State *L)
{
    int id = send_create_object(script_getstring(L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_create_galaxy(lua_State *L)
{
    int id = send_create_galaxy(script_getstring(L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_create_light(lua_State *L)
{
    int fl = (int) script_getnumber(L, -1);
    int id = send_create_light(fl);

    lua_pushentity(L, id);
    return 1;
}

static int script_create_pivot(lua_State *L)
{
    int id = send_create_pivot();

    lua_pushentity(L, id);
    return 1;
}

/*---------------------------------------------------------------------------*/
/* Galaxy controls                                                           */

static int script_set_galaxy_magnitude(lua_State *L)
{
    send_set_galaxy_magnitude(script_getgalaxy(L, -2),
                              script_getnumber(L, -1));
    return 0;
}

static int script_get_star_index(lua_State *L)
{
    int id = script_getentity(L, -2);
    int gd = script_getgalaxy(L, -2);
    int jd = script_getentity(L, -1);

    float p[3];
    float v[3];

    get_entity_position(jd, p);
    get_entity_z_vector(jd, v);

    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];

    lua_pushnumber(L, pick_galaxy(id, gd, p, v));
    return 1;
}

static int script_get_star_position(lua_State *L)
{
    float p[3];

    get_star_position(script_getgalaxy(L, -2),
                (int) script_getnumber(L, -1), p);

    lua_pushnumber(L, p[0]);
    lua_pushnumber(L, p[1]);
    lua_pushnumber(L, p[2]);

    return 3;
}

/*---------------------------------------------------------------------------*/
/* Camera controls                                                           */

static int script_set_camera_offset(lua_State *L)
{
    float v[3];
    float e[3][3] = {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f }
    };

    v[0] = script_getnumber(L, -3);
    v[1] = script_getnumber(L, -2);
    v[2] = script_getnumber(L, -1);

    send_set_camera_offset(script_getcamera(L, -4), v, e);
    return 0;
}

static int script_set_camera_stereo(lua_State *L)
{
    float v[3];

    v[0] = script_getnumber(L, -3);
    v[1] = script_getnumber(L, -2);
    v[2] = script_getnumber(L, -1);

    send_set_camera_stereo(script_getcamera(L, -5), v,
                     (int) script_getnumber(L, -4));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Light controls                                                            */

static int script_set_light_color(lua_State *L)
{
    send_set_light_color(script_getlight(L, -4),
                         script_getnumber(L, -3),
                         script_getnumber(L, -2),
                         script_getnumber(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Sprite controls                                                           */

static int script_set_sprite_bounds(lua_State *L)
{
    send_set_sprite_bounds(script_getsprite(L, -5),
                           script_getnumber(L, -4),
                           script_getnumber(L, -3),
                           script_getnumber(L, -2),
                           script_getnumber(L, -1));
    return 0;
}

static int script_get_sprite_pixel(lua_State *L)
{
    unsigned char p[4];

    get_sprite_p(script_getsprite(L, -3),
                 (int) script_getnumber(L, -2),
                 (int) script_getnumber(L, -1), p);

    lua_pushnumber(L, (double) p[0] / 255.0);
    lua_pushnumber(L, (double) p[1] / 255.0);
    lua_pushnumber(L, (double) p[2] / 255.0);
    lua_pushnumber(L, (double) p[3] / 255.0);

    return 4;
}

static int script_get_sprite_size(lua_State *L)
{
    int id = script_getsprite(L, -1);

    lua_pushnumber(L, (double) get_sprite_w(id));
    lua_pushnumber(L, (double) get_sprite_h(id));

    return 2;
}

/*---------------------------------------------------------------------------*/
/* Sound functions                                                           */

static int script_load_sound(lua_State *L)
{
    lua_pushsound(L, create_sound(script_getstring(L, -1)));
    return 1;
}

static int script_free_sound(lua_State *L)
{
    delete_sound(script_getsound(L, -1));
    return 0;
}

static int script_stop_sound(lua_State *L)
{
    stop_sound(script_getsound(L, -1));
    return 0;
}

static int script_play_sound(lua_State *L)
{
    play_sound(script_getsound(L, -1));
    return 0;
}

static int script_loop_sound(lua_State *L)
{
    loop_sound(script_getsound(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Console functions                                                         */

static int script_clear_console(lua_State *L)
{
    clear_console();
    return 0;
}

static int script_color_console(lua_State *L)
{
    color_console(script_getnumber(L, -3),
                  script_getnumber(L, -2),
                  script_getnumber(L, -1));
    return 0;
}

static int script_print_console(lua_State *L)
{
    int i, n = lua_gettop(L);
    const char *str;

    for (i = n; i > 0; --i)
        if ((str = script_getstring(L, -i)))
            print_console(str);

    return 1;
}

/*---------------------------------------------------------------------------*/

static int script_exit(lua_State *L)
{
    SDL_Event e;

    e.type = SDL_QUIT;
    SDL_PushEvent(&e);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* Script callback backcallers                                               */

static int lua_callassert(lua_State *L, int nin, int nout, const char *name)
{
    int r = 0;

    if (lua_pcall(L, nin, nout, 0) == LUA_ERRRUN)
        error("%s: %s\n", name, lua_tostring(L, -1));
    else
        r = lua_toboolean(L, -1);

    lua_pop(L, 1);

    return r;
}

int do_point_script(int x, int y)
{
    const char *name = "do_point";

    lua_getglobal(L, name);

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber(L, (lua_Number) x);
        lua_pushnumber(L, (lua_Number) y);

        return lua_callassert(L, 2, 1, name);
    }
    else lua_pop(L, 1);

    return 0;
}

int do_click_script(int b, int s)
{
    const char *name = "do_click";

    lua_getglobal(L, name);

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber (L, (lua_Number) b);
        lua_pushboolean(L, s);

        return lua_callassert(L, 2, 1, name);
    }
    else lua_pop(L, 1);

    return 0;
}

int do_frame_script(void)
{
    const char *name = "do_frame";

    lua_getglobal(L, name);

    if (lua_isfunction(L, -1))
        return lua_callassert(L, 0, 1, name);
    else
        lua_pop(L, 1);

    return 0;
}

int do_timer_script(int t)
{
    const char *name = "do_timer";

    lua_getglobal(L, name);

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber(L, (lua_Number) (t / 1000.0f));

        return lua_callassert(L, 1, 1, name);
    }
    else lua_pop(L, 1);

    return 0;
}

int do_keyboard_script(int k, int s)
{
    const char *name = "do_keyboard";

    lua_getglobal(L, name);

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber (L, (lua_Number) k);
        lua_pushboolean(L, s);

        return lua_callassert(L, 2, 1, name);
    }
    else lua_pop(L, 1);

    return 0;
}

int do_joystick_script(int n, int b, int s)
{
    const char *name = "do_joystick";

    lua_getglobal(L, name);

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber (L, (lua_Number) n);
        lua_pushnumber (L, (lua_Number) b);
        lua_pushboolean(L, s);

        return lua_callassert(L, 3, 1, name);
    }
    else lua_pop(L, 1);

    return 0;
}

void do_command(const char *command)
{
    char buffer[MAXSTR];
    int err;
    int top = lua_gettop(L);

    memset(buffer, 0, MAXSTR);
    strncpy(buffer, command, MAXSTR);

    if ((err = lua_load(L, charreader, buffer, "Console")))
        error("Syntax: %s", lua_tostring(L, -1));
    else
    {
        if (lua_pcall(L, 0, 0, 0))
            error("Command: %s", lua_tostring(L, -1));
    }

    while (lua_gettop(L) > top)
        lua_pop(L, 1);
}

/*---------------------------------------------------------------------------*/

void load_args(int c, char *v[])
{
    int j;

    lua_getglobal(L, "E");

    if (lua_istable(L, -1))
    {
        lua_pushstring(L, "argument");
        lua_gettable(L, -2);

        if (lua_istable(L, -1))
        {
            for (j = 0; j < c; j++)
            {
                lua_pushnumber(L, j + 1);
                lua_pushstring(L, v[j]);
                lua_settable(L, -3);
            }
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

/*---------------------------------------------------------------------------*/
/* Script setup/shutdown                                                     */

#define lua_function(L, n, f) (lua_pushstring(L, n),      \
                               lua_pushstring(L, n),      \
                               lua_pushcclosure(L, f, 1), \
                               lua_settable(L, -3))
#define lua_constant(L, n, v) (lua_pushstring(L, n), \
                               lua_pushnumber(L, v), \
                               lua_settable(L, -3))

void luaopen_electro(lua_State *L)
{
    /* Create the Electro environment table. */

    lua_pushstring(L, "E");
    lua_newtable(L);

    /* Entity contructors and destructor. */

    lua_function(L, "create_camera",        script_create_camera);
    lua_function(L, "create_sprite",        script_create_sprite);
    lua_function(L, "create_object",        script_create_object);
    lua_function(L, "create_galaxy",        script_create_galaxy);
    lua_function(L, "create_light",         script_create_light);
    lua_function(L, "create_pivot",         script_create_pivot);
    lua_function(L, "create_clone",         script_create_clone);

    /* Entity control. */

    lua_function(L, "parent_entity",        script_parent_entity);
    lua_function(L, "delete_entity",        script_delete_entity);

    lua_function(L, "get_entity_parent",    script_get_entity_parent);
    lua_function(L, "get_entity_child",     script_get_entity_child);

    lua_function(L, "set_entity_position",  script_set_entity_position);
    lua_function(L, "set_entity_rotation",  script_set_entity_rotation);
    lua_function(L, "set_entity_scale",     script_set_entity_scale);
    lua_function(L, "set_entity_alpha",     script_set_entity_alpha);
    lua_function(L, "set_entity_flag",      script_set_entity_flag);
    lua_function(L, "set_entity_frag_prog", script_set_entity_frag_prog);
    lua_function(L, "set_entity_vert_prog", script_set_entity_vert_prog);

    lua_function(L, "get_entity_position",  script_get_entity_position);
    lua_function(L, "get_entity_x_vector",  script_get_entity_x_vector);
    lua_function(L, "get_entity_y_vector",  script_get_entity_y_vector);
    lua_function(L, "get_entity_z_vector",  script_get_entity_z_vector);
    lua_function(L, "get_entity_scale",     script_get_entity_scale);
    lua_function(L, "get_entity_alpha",     script_get_entity_alpha);

    lua_function(L, "move_entity",          script_move_entity);
    lua_function(L, "turn_entity",          script_turn_entity);

    /* Galaxy control. */

    lua_function(L, "set_galaxy_magnitude", script_set_galaxy_magnitude);
    lua_function(L, "get_star_index",       script_get_star_index);
    lua_function(L, "get_star_position",    script_get_star_position);

    /* Camera control. */

    lua_function(L, "set_camera_offset",    script_set_camera_offset);
    lua_function(L, "set_camera_stereo",    script_set_camera_stereo);

    /* Light control. */

    lua_function(L, "set_light_color",      script_set_light_color);

    /* Sprite control. */

    lua_function(L, "set_sprite_bounds",    script_set_sprite_bounds);
    lua_function(L, "get_sprite_pixel",     script_get_sprite_pixel);
    lua_function(L, "get_sprite_size",      script_get_sprite_size);

    /* Sound control. */

    lua_function(L, "load_sound",           script_load_sound);
    lua_function(L, "free_sound",           script_free_sound);
    lua_function(L, "stop_sound",           script_stop_sound);
    lua_function(L, "play_sound",           script_play_sound);
    lua_function(L, "loop_sound",           script_loop_sound);

    /* Console */

    lua_function(L, "clear_console",        script_clear_console);
    lua_function(L, "color_console",        script_color_console);
    lua_function(L, "print_console",        script_print_console);

    /* Display */

    lua_function(L, "add_host",             script_add_host);
    lua_function(L, "add_tile",             script_add_tile);
    lua_function(L, "get_viewport",         script_get_viewport);

    /* Misc. */

    lua_function(L, "enable_timer",         script_enable_timer);
    lua_function(L, "get_joystick",         script_get_joystick);
    lua_function(L, "get_modifier",         script_get_modifier);
    lua_function(L, "set_background",       script_set_background);
    lua_function(L, "exit",                 script_exit);

    /* Constants. */

    lua_constant(L, "entity_flag_hidden",        FLAG_HIDDEN);
    lua_constant(L, "entity_flag_wireframe",     FLAG_WIREFRAME);
    lua_constant(L, "entity_flag_billboard",     FLAG_BILLBOARD);
    lua_constant(L, "entity_flag_unlit",         FLAG_UNLIT);
    lua_constant(L, "entity_flag_line_smooth",   FLAG_LINE_SMOOTH);
    lua_constant(L, "entity_flag_pos_tracked_0", FLAG_POS_TRACKED_0);
    lua_constant(L, "entity_flag_rot_tracked_0", FLAG_ROT_TRACKED_0);
    lua_constant(L, "entity_flag_pos_tracked_1", FLAG_POS_TRACKED_1);
    lua_constant(L, "entity_flag_rot_tracked_1", FLAG_ROT_TRACKED_1);

    lua_constant(L, "camera_type_orthogonal",    CAMERA_ORTHO);
    lua_constant(L, "camera_type_perspective",   CAMERA_PERSP);

    lua_constant(L, "stereo_mode_none",          STEREO_NONE);
    lua_constant(L, "stereo_mode_quad",          STEREO_QUAD);
    lua_constant(L, "stereo_mode_red_blue",      STEREO_RED_BLUE);
    lua_constant(L, "stereo_mode_varrier",       STEREO_VARRIER);

    lua_constant(L, "light_type_positional",     LIGHT_POSITIONAL);
    lua_constant(L, "light_type_directional",    LIGHT_DIRECTIONAL);

    lua_constant(L, "key_modifier_shift",        KMOD_SHIFT);
    lua_constant(L, "key_modifier_control",      KMOD_CTRL);
    lua_constant(L, "key_modifier_alt",          KMOD_ALT);

    /* Add an empty table to hold command line arguments. */

    lua_pushstring(L, "argument");
    lua_newtable(L);
    lua_settable(L, -3);

    /* Register the "electro" environment table globally. */

    lua_settable(L, LUA_GLOBALSINDEX);
}

int init_script(void)
{
    if ((L = lua_open()))
    {
        luaopen_io(L);
        luaopen_base(L);
        luaopen_math(L);
        luaopen_table(L);
        luaopen_debug(L);
        luaopen_string(L);
        luaopen_electro(L);

        return 1;
    }
    return 0;
}

void free_script(void)
{
    lua_close(L);
}

void load_script(const char *file, int push)
{
    char cwd[MAXSTR];
    int  err;
    FILE *fp;

    /* Change the CWD to the directory of the named script. */

    const char *path = get_file_path(file);
    const char *name = get_file_name(file);

    if (push) getcwd(cwd, MAXSTR);

    chdir(path);

    /* Load and execute the script. */

    if ((fp = fopen(name, "r")))
    {
        int top = lua_gettop(L);

        if ((err = lua_load(L, filereader, fp, file)))
            error("Loading: %s", lua_tostring(L, -1));
        else
        {
            if (lua_pcall(L, 0, 0, 0))
                error("Executing: %s", lua_tostring(L, -1));
        }

        while (lua_gettop(L) > top)
            lua_pop(L, 1);

        fclose(fp);
    }

    if (push) chdir(cwd);
}

/*---------------------------------------------------------------------------*/
