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
#include <something.h>
#else
#include <unistd.h>
#endif

#include <SDL.h>
#include <lua.h>
#include <lualib.h>

#include "joystick.h"
#include "viewport.h"
#include "server.h"
#include "camera.h"
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

static void script_type_error(const char *name,
                              const char *type, lua_State *L, int i)
{
    const char *got = "unknown";

    if (lua_isuserdata(L, i))
        got = get_entity_type_name(lua_toentity(L, i));
    else
        got = lua_typename(L, lua_type(L, i));

    lua_pushfstring(L, "'%s' expected %s, got %s", name, type, got);
    lua_error(L);
}

static void script_arity_error(const char *name, lua_State *L, int i, int n)
{
    lua_pushfstring(L, "'%s' expected %d parameters, got %d", name, -i, n);
    lua_error(L);
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

static const char *script_getstring(const char *name, lua_State *L, int i)
{
    int n = lua_gettop(L);

    /* Check the argument count, check for a string, and return it. */

    if (1 <= -i && -i <= n)
    {
        if (lua_isstring(L, i))
            return lua_tostring(L, i);
        else
            script_type_error(name, "string", L, i);
    }
    else script_arity_error(name, L, i, n);

    return "";
}

static float script_getnumber(const char *name, lua_State *L, int i)
{
    int n = lua_gettop(L);

    /* Check the argument count, check for a number, and return it. */

    if (1 <= -i && -i <= n)
    {
        if (lua_isnumber(L, i))
            return (float) lua_tonumber(L, i);
        else
            script_type_error(name, "number", L, i);
    }
    else script_arity_error(name, L, i, n);

    return 0.0;
}

static int script_getboolean(const char *name, lua_State *L, int i)
{
    int n = lua_gettop(L);

    /* Check the argument count, check for a boolean, and return it. */

    if (1 <= -i && -i <= n)
    {
        if (lua_isboolean(L, i))
            return lua_toboolean(L, i);
        else
            script_type_error(name, "boolean", L, i);
    }
    else script_arity_error(name, L, i, n);

    return 0;
}

static int script_getentity(const char *name, lua_State *L, int i)
{
    int n = lua_gettop(L);

    /* Check the argument count, check for a entity, and return it. */

    if (1 <= -i && -i <= n)
    {
        if (lua_isnil(L, i))    return 0;
        if (lua_isentity(L, i)) return lua_toentity(L, i);

        script_type_error(name, "entity", L, i);
    }
    else script_arity_error(name, L, i, n);

    return 0;
}

static int script_getcamera(const char *name, lua_State *L, int i)
{
    int n = lua_gettop(L);

    /* Check the argument count, check for a camera, and return it. */

    if (1 <= -i && -i <= n)
    {
        if (lua_iscamera(L, i))
            return entity_data(lua_toentity(L, i));
        else
            script_type_error(name, "camera", L, i);
    }
    else script_arity_error(name, L, i, n);

    return 0;
}

static int script_getgalaxy(const char *name, lua_State *L, int i)
{
    int n = lua_gettop(L);

    /* Check the argument count, check for a galaxy, and return it. */

    if (1 <= -i && -i <= n)
    {
        if (lua_isgalaxy(L, i))
            return entity_data(lua_toentity(L, i));
        else
            script_type_error(name, "galaxy", L, i);
    }
    else script_arity_error(name, L, i, n);

    return 0;
}

static int script_getsprite(const char *name, lua_State *L, int i)
{
    int n = lua_gettop(L);

    /* Check the argument count, check for a sprite, and return it. */

    if (1 <= -i && -i <= n)
    {
        if (lua_issprite(L, i))
            return entity_data(lua_toentity(L, i));
        else
            script_type_error(name, "sprite", L, i);
    }
    else script_arity_error(name, L, i, n);

    return 0;
}

static int script_getlight(const char *name, lua_State *L, int i)
{
    int n = lua_gettop(L);

    /* Check the argument count, check for a light, and return it. */

    if (1 <= -i && -i <= n)
    {
        if (lua_islight(L, i))
            return entity_data(lua_toentity(L, i));
        else
            script_type_error(name, "light", L, i);
    }
    else script_arity_error(name, L, i, n);

    return 0;
}

static int script_getsound(const char *name, lua_State *L, int i)
{
    int n = lua_gettop(L);

    /* Check the argument count, check for a sound, and return it. */

    if (1 <= -i && -i <= n)
    {
        if (lua_issound(L, i))
            return lua_tosound(L, i);
        else
            script_type_error(name, "sound", L, i);
    }
    else script_arity_error(name, L, i, n);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* Miscellaneous functions                                                   */

static int script_add_tile(lua_State *L)
{
    const char *name = "add_tile";

    add_tile(script_getstring(name, L, -7),
             script_getnumber(name, L, -6),
             script_getnumber(name, L, -5),
             script_getnumber(name, L, -4),
             script_getnumber(name, L, -3),
             script_getnumber(name, L, -2),
             script_getnumber(name, L, -1));
    return 0;
}

static int script_enable_timer(lua_State *L)
{
    const char *name = "enable_timer";

    enable_timer(script_getboolean(name, L, -1));
    return 0;
}

static int script_get_joystick(lua_State *L)
{
    const char *name = "get_joystick";
    float a[2];

    get_joystick((int) script_getnumber(name, L, -1), a);

    lua_pushnumber(L, (double) a[0]);
    lua_pushnumber(L, (double) a[1]);
    return 2;
}

static int script_get_viewport(lua_State *L)
{
    float x = get_total_viewport_x();
    float y = get_total_viewport_y();
    float w = get_total_viewport_w();
    float h = get_total_viewport_h();

    lua_pushnumber(L, x);
    lua_pushnumber(L, x + w);
    lua_pushnumber(L, y);
    lua_pushnumber(L, y + h);

    return 4;
}

static int script_get_modifier(lua_State *L)
{
    int i = (int) script_getnumber("get_modifier", L, -1);

    lua_pushboolean(L, (SDL_GetModState() & i) ? 1 : 0);

    return 1;
}

static int script_set_background(lua_State *L)
{
    const char *name = "set_background";

    int n = lua_gettop(L);
    float c0[3];
    float c1[3];

    if (n == 6)
    {
        c0[0] = script_getnumber(name, L, -6);
        c0[1] = script_getnumber(name, L, -5);
        c0[2] = script_getnumber(name, L, -4);
        c1[0] = script_getnumber(name, L, -3);
        c1[1] = script_getnumber(name, L, -2);
        c1[2] = script_getnumber(name, L, -1);

        send_set_background(c0, c1);
    }
    else
    {
        c0[0] = script_getnumber(name, L, -3);
        c0[1] = script_getnumber(name, L, -2);
        c0[2] = script_getnumber(name, L, -1);

        send_set_background(c0, c0);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Entity hierarchy functions                                                */

static int script_parent_entity(lua_State *L)
{
    const char *name = "parent_entity";

    send_parent_entity(script_getentity(name, L, -2),
                       script_getentity(name, L, -1));
    return 0;
}

static int script_delete_entity(lua_State *L)
{
    const char *name = "delete_entity";

    send_delete_entity(script_getentity(name, L, -1));
    return 0;
}

static int script_create_clone(lua_State *L)
{
    int id = send_create_clone(script_getentity("create_clone", L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_get_entity_parent(lua_State *L)
{
    int id = get_entity_parent(script_getentity("get_entity_parent", L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_get_entity_child(lua_State *L)
{
    const char *name = "get_entity_child";

    int id = get_entity_child(script_getentity(name, L, -2),
                        (int) script_getnumber(name, L, -1));

    lua_pushentity(L, id);
    return 1;
}

/*---------------------------------------------------------------------------*/
/* Entity transform functions                                                */

static int script_set_entity_position(lua_State *L)
{
    const char *name = "set_entity_position";

    send_set_entity_position(script_getentity(name, L, -4),
                             script_getnumber(name, L, -3),
                             script_getnumber(name, L, -2),
                             script_getnumber(name, L, -1));
    return 0;
}

static int script_set_entity_rotation(lua_State *L)
{
    const char *name = "set_entity_rotation";

    send_set_entity_rotation(script_getentity(name, L, -4),
                             script_getnumber(name, L, -3),
                             script_getnumber(name, L, -2),
                             script_getnumber(name, L, -1));
    return 0;
}

static int script_set_entity_scale(lua_State *L)
{
    const char *name = "set_entity_scale";

    send_set_entity_scale(script_getentity(name, L, -4),
                          script_getnumber(name, L, -3),
                          script_getnumber(name, L, -2),
                          script_getnumber(name, L, -1));
    return 0;
}

static int script_set_entity_alpha(lua_State *L)
{
    const char *name = "set_entity_alpha";

    send_set_entity_alpha(script_getentity(name, L, -2),
                          script_getnumber(name, L, -1));
    return 0;
}

static int script_set_entity_flag(lua_State *L)
{
    const char *name = "set_entity_flag";

    send_set_entity_flag(script_getentity(name, L, -3),
                   (int) script_getnumber(name, L, -2),
                         script_getboolean(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/

static int script_move_entity(lua_State *L)
{
    const char *name = "move_entity";

    send_move_entity(script_getentity(name, L, -4),
                     script_getnumber(name, L, -3),
                     script_getnumber(name, L, -2),
                     script_getnumber(name, L, -1));
    return 0;
}

static int script_turn_entity(lua_State *L)
{
    const char *name = "turn_entity";

    send_turn_entity(script_getentity(name, L, -4),
                     script_getnumber(name, L, -3),
                     script_getnumber(name, L, -2),
                     script_getnumber(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/

static int script_get_entity_position(lua_State *L)
{
    int id = script_getentity("get_entity_position", L, -1);
    float x, y, z;

    get_entity_position(id, &x, &y, &z);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);

    return 3;
}

static int script_get_entity_rotation(lua_State *L)
{
    int id = script_getentity("get_entity_rotation", L, -1);
    float x, y, z;

    get_entity_rotation(id, &x, &y, &z);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);

    return 3;
}

static int script_get_entity_scale(lua_State *L)
{
    int id = script_getentity("get_entity_scale", L, -1);
    float x, y, z;

    get_entity_scale(id, &x, &y, &z);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);

    return 3;
}

static int script_get_entity_alpha(lua_State *L)
{
    int  id = script_getentity("get_entity_alpha", L, -1);
    float a = get_entity_alpha(id);

    lua_pushnumber(L, a);

    return 1;
}

/*---------------------------------------------------------------------------*/
/* Entity constructors.                                                      */

static int script_create_camera(lua_State *L)
{
	int fl = (int) script_getnumber("create_camera", L, -1);
    int id = send_create_camera(fl);

    lua_pushentity(L, id);
    return 1;
}

static int script_create_sprite(lua_State *L)
{
    int id = send_create_sprite(script_getstring("create_sprite", L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_create_object(lua_State *L)
{
    int id = send_create_object(script_getstring("create_object", L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_create_galaxy(lua_State *L)
{
    int id = send_create_galaxy(script_getstring("create_galaxy", L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_create_light(lua_State *L)
{
	int fl = (int) script_getnumber("create_light", L, -1);
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
    const char *name = "set_galaxy_magnitude";

    send_set_galaxy_magnitude(script_getgalaxy(name, L, -2),
                              script_getnumber(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Camera controls                                                           */

static int script_set_camera_distance(lua_State *L)
{
    const char *name = "set_camera_distance";

    send_set_camera_distance(script_getcamera(name, L, -2),
                             script_getnumber(name, L, -1));
    return 0;
}

static int script_set_camera_zoom(lua_State *L)
{
    const char *name = "set_camera_zoom";

    send_set_camera_zoom(script_getcamera(name, L, -2),
                         script_getnumber(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Light controls                                                            */

static int script_set_light_color(lua_State *L)
{
    const char *name = "set_light_color";

    send_set_light_color(script_getlight(name, L, -4),
                         script_getnumber(name, L, -3),
                         script_getnumber(name, L, -2),
                         script_getnumber(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Sprite controls                                                           */

static int script_set_sprite_bounds(lua_State *L)
{
    const char *name = "set_sprite_bounds";

    send_set_sprite_bounds(script_getsprite(name, L, -5),
                           script_getnumber(name, L, -4),
                           script_getnumber(name, L, -3),
                           script_getnumber(name, L, -2),
                           script_getnumber(name, L, -1));
    return 0;
}

static int script_get_sprite_pixel(lua_State *L)
{
    const char *name = "get_sprite_pixel";

    unsigned char p[4];

    get_sprite_p(script_getsprite(name, L, -3),
                 (int) script_getnumber(name, L, -2),
                 (int) script_getnumber(name, L, -1), p);

    lua_pushnumber(L, (double) p[0] / 255.0);
    lua_pushnumber(L, (double) p[1] / 255.0);
    lua_pushnumber(L, (double) p[2] / 255.0);
    lua_pushnumber(L, (double) p[3] / 255.0);

    return 4;
}

static int script_get_sprite_size(lua_State *L)
{
    int id = script_getsprite("get_sprite_size", L, -1);

    lua_pushnumber(L, (double) get_sprite_w(id));
    lua_pushnumber(L, (double) get_sprite_h(id));

    return 2;
}

/*---------------------------------------------------------------------------*/
/* Sound functions                                                           */

static int script_load_sound(lua_State *L)
{
    lua_pushsound(L, create_sound(script_getstring("load_sound", L, -1)));
    return 1;
}

static int script_free_sound(lua_State *L)
{
    delete_sound(script_getsound("free_sound", L, -1));
    return 0;
}

static int script_stop_sound(lua_State *L)
{
    stop_sound(script_getsound("stop_sound", L, -1));
    return 0;
}

static int script_play_sound(lua_State *L)
{
    play_sound(script_getsound("play_sound", L, -1));
    return 0;
}

static int script_loop_sound(lua_State *L)
{
    loop_sound(script_getsound("loop_sound", L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/

static int script_get_entity_debug_id(lua_State *L)
{
    int id = script_getentity("get_entity_debug_id", L, -1);

    lua_pushstring(L, get_entity_debug_id(id));
    return 1;
}

/*---------------------------------------------------------------------------*/
/* Script callback backcallers                                               */

static int lua_callassert(lua_State *L, int nin, int nout, const char *name)
{
    int r = 0;

    if (lua_pcall(L, nin, nout, 0) == LUA_ERRRUN)
        fprintf(stderr, "%s: %s\n", name, lua_tostring(L, -1));
    else
        r = lua_toboolean(L, -1);

    lua_pop(L, 1);

    return r;
}

int do_start_script(void)
{
    const char *name = "do_start";

    lua_getglobal(L, name);

    if (lua_isfunction(L, -1))
        return lua_callassert(L, 0, 1, name);
    else
        lua_pop(L, 1);

    return 0;
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

/*---------------------------------------------------------------------------*/
/* Script setup/shutdown                                                     */

#define lua_function(L, n, f) (lua_pushstring(L, n),    \
                               lua_pushcfunction(L, f), \
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

    lua_function(L, "get_entity_position",  script_get_entity_position);
    lua_function(L, "get_entity_rotation",  script_get_entity_rotation);
    lua_function(L, "get_entity_scale",     script_get_entity_scale);
    lua_function(L, "get_entity_alpha",     script_get_entity_alpha);

    lua_function(L, "move_entity",          script_move_entity);
    lua_function(L, "turn_entity",          script_turn_entity);

    /* Galaxy control. */

    lua_function(L, "set_galaxy_magnitude", script_set_galaxy_magnitude);

    /* Light control. */

    lua_function(L, "set_light_color",      script_set_light_color);

    /* Camera control. */

    lua_function(L, "set_camera_distance",  script_set_camera_distance);
    lua_function(L, "set_camera_zoom",      script_set_camera_zoom);

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

    /* Misc. */

    lua_function(L, "add_tile",             script_add_tile);
    lua_function(L, "enable_timer",         script_enable_timer);
    lua_function(L, "get_joystick",         script_get_joystick);
    lua_function(L, "get_viewport",         script_get_viewport);
    lua_function(L, "get_modifier",         script_get_modifier);
    lua_function(L, "set_background",       script_set_background);
    lua_function(L, "get_entity_debug_id",  script_get_entity_debug_id);

    /* Constants. */

    lua_constant(L, "entity_flag_hidden",      FLAG_HIDDEN);
    lua_constant(L, "entity_flag_wireframe",   FLAG_WIREFRAME);
    lua_constant(L, "entity_flag_billboard",   FLAG_BILLBOARD);
    lua_constant(L, "entity_flag_unlit",       FLAG_UNLIT);

    lua_constant(L, "camera_type_orthogonal",  CAMERA_ORTHO);
    lua_constant(L, "camera_type_perspective", CAMERA_PERSP);

    lua_constant(L, "light_type_positional",   LIGHT_POSITIONAL);
    lua_constant(L, "light_type_directional",  LIGHT_DIRECTIONAL);

    lua_constant(L, "key_modifier_shift",      KMOD_SHIFT);
    lua_constant(L, "key_modifier_control",    KMOD_CTRL);
    lua_constant(L, "key_modifier_alt",        KMOD_ALT);

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

    /* Change the CWD to the directory of the named script. */

    const char *path = get_file_path(file);
    const char *name = get_file_name(file);

    if (push) getcwd(cwd, MAXSTR);

    chdir(path);

    /* Execute the script. */

    lua_getglobal(L, "dofile");

    if (lua_isfunction(L, -1))
    {
        lua_pushstring(L, name);

        if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN)
        {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    else lua_pop(L, 1);

    if (push) chdir(cwd);
}

/*---------------------------------------------------------------------------*/
