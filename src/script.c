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

#include <lua.h>
#include <lualib.h>
#include <stdio.h>

#include "joystick.h"
#include "viewport.h"
#include "shared.h"
#include "server.h"
#include "camera.h"
#include "sprite.h"
#include "object.h"
#include "galaxy.h"
#include "light.h"
#include "pivot.h"
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
    if (id < 0)
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
        got = entity_typename(lua_toentity(L, i));
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
        && entity_istype(lua_toentity(L, i), TYPE_CAMERA);
}

static int lua_isgalaxy(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && entity_istype(lua_toentity(L, i), TYPE_GALAXY);
}

static int lua_issprite(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && entity_istype(lua_toentity(L, i), TYPE_SPRITE);
}

static int lua_islight(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && entity_istype(lua_toentity(L, i), TYPE_LIGHT);
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
        if (lua_isentity(L, i))
            return lua_toentity(L, i);
        else
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
            return entity_todata(lua_toentity(L, i));
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
            return entity_todata(lua_toentity(L, i));
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
            return entity_todata(lua_toentity(L, i));
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
            return entity_todata(lua_toentity(L, i));
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

    viewport_tile(script_getstring(name, L, -7),
                  script_getnumber(name, L, -6),
                  script_getnumber(name, L, -5),
                  script_getnumber(name, L, -4),
                  script_getnumber(name, L, -3),
                  script_getnumber(name, L, -2),
                  script_getnumber(name, L, -1));
    return 0;
}

static int script_enable_idle(lua_State *L)
{
    const char *name = "enable_idle";

    enable_idle(script_getboolean(name, L, -1));
    return 0;
}

static int script_joystick_axis(lua_State *L)
{
    const char *name = "joystick_axis";

    lua_pushnumber(L, joystick_axis(script_getnumber(name, L, -2),
                                    script_getnumber(name, L, -1)));
    return 1;
}

/*---------------------------------------------------------------------------*/
/* Entity hierarchy functions                                                */

static int script_entity_parent(lua_State *L)
{
    const char *name = "entity_parent";

    entity_send_parent(script_getentity(name, L, -2),
                       script_getentity(name, L, -1));
    return 0;
}

static int script_entity_delete(lua_State *L)
{
    const char *name = "entity_delete";

    entity_send_delete(script_getentity(name, L, -1));
    return 0;
}

static int script_entity_clone(lua_State *L)
{
    int id = entity_send_clone(script_getentity("entity_clone", L, -1));

    lua_pushentity(L, id);
    return 1;
}

/*---------------------------------------------------------------------------*/
/* Entity transform functions                                                */

static int script_entity_position(lua_State *L)
{
    const char *name = "entity_position";

    entity_send_position(script_getentity(name, L, -4),
                         script_getnumber(name, L, -3),
                         script_getnumber(name, L, -2),
                         script_getnumber(name, L, -1));
    return 0;
}

static int script_entity_rotation(lua_State *L)
{
    const char *name = "entity_rotation";

    entity_send_rotation(script_getentity(name, L, -4),
                         script_getnumber(name, L, -3),
                         script_getnumber(name, L, -2),
                         script_getnumber(name, L, -1));
    return 0;
}

static int script_entity_scale(lua_State *L)
{
    const char *name = "entity_scale";

    entity_send_scale(script_getentity(name, L, -4),
                      script_getnumber(name, L, -3),
                      script_getnumber(name, L, -2),
                      script_getnumber(name, L, -1));
    return 0;
}

static int script_entity_alpha(lua_State *L)
{
    const char *name = "entity_alpha";

    entity_send_alpha(script_getentity(name, L, -2),
                      script_getnumber(name, L, -1));
    return 0;
}

static int script_entity_flag(lua_State *L)
{
    const char *name = "entity_flag";

    entity_send_flag(script_getentity(name, L, -3),
                     script_getnumber(name, L, -2),
                     script_getboolean(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/

static int script_entity_get_position(lua_State *L)
{
    float x, y, z;
    int id = script_getentity("entity_get_position", L, -1);

    entity_get_position(id, &x, &y, &z);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);

    return 3;
}

static int script_entity_get_rotation(lua_State *L)
{
    float x, y, z;
    int id = script_getentity("entity_get_rotation", L, -1);

    entity_get_rotation(id, &x, &y, &z);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);

    return 3;
}

static int script_entity_get_scale(lua_State *L)
{
    float x, y, z;
    int id = script_getentity("entity_get_scale", L, -1);

    entity_get_scale(id, &x, &y, &z);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);

    return 3;
}

static int script_entity_get_alpha(lua_State *L)
{
    int  id = script_getentity("entity_get_alpha", L, -1);
    float a = entity_get_alpha(id);

    lua_pushnumber(L, a);

    return 1;
}

static int script_viewport_get(lua_State *L)
{
    float x = viewport_total_x();
    float y = viewport_total_y();
    float w = viewport_total_w();
    float h = viewport_total_h();

    lua_pushnumber(L, x);
    lua_pushnumber(L, x + w);
    lua_pushnumber(L, y);
    lua_pushnumber(L, y + h);

    return 4;
}

/*---------------------------------------------------------------------------*/
/* Entity constructors.                                                      */

static int script_create_camera(lua_State *L)
{
    int id = camera_send_create(script_getnumber("create_camera", L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_create_sprite(lua_State *L)
{
    int id = sprite_send_create(script_getstring("create_sprite", L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_create_object(lua_State *L)
{
    int id = object_send_create(script_getstring("create_object", L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_create_galaxy(lua_State *L)
{
    const char *name = "create_galaxy";
    int id, n = lua_gettop(L);

    if (n == 2)
        id = galaxy_send_create(script_getstring(name, L, -2),
                                script_getstring(name, L, -1));
    else
        id = galaxy_send_create(script_getstring(name, L, -1), 0);
                                

    lua_pushentity(L, id);
    return 1;
}

static int script_create_light(lua_State *L)
{
    int id = light_send_create(script_getnumber("create_light", L, -1));

    lua_pushentity(L, id);
    return 1;
}

static int script_create_pivot(lua_State *L)
{
    int id = pivot_send_create();

    lua_pushentity(L, id);
    return 1;
}

/*---------------------------------------------------------------------------*/
/* Galaxf controls                                                           */

static int script_galaxy_magn(lua_State *L)
{
    const char *name = "galaxy_magn";

    galaxy_send_magn(script_getgalaxy(name, L, -2),
                     script_getnumber(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Camera controls                                                           */

static int script_camera_dist(lua_State *L)
{
    const char *name = "camera_dist";

    camera_send_dist(script_getcamera(name, L, -2),
                     script_getnumber(name, L, -1));
    return 0;
}

static int script_camera_zoom(lua_State *L)
{
    const char *name = "camera_zoom";

    camera_send_zoom(script_getcamera(name, L, -2),
                     script_getnumber(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Light controls                                                            */

static int script_light_color(lua_State *L)
{
    const char *name = "light_color";

    light_send_color(script_getlight(name, L, -4),
                     script_getnumber(name, L, -3),
                     script_getnumber(name, L, -2),
                     script_getnumber(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Sprite controls                                                           */

static int script_sprite_bounds(lua_State *L)
{
    const char *name = "sprite_bounds";

    sprite_send_bounds(script_getsprite(name, L, -5),
                       script_getnumber(name, L, -4),
                       script_getnumber(name, L, -3),
                       script_getnumber(name, L, -2),
                       script_getnumber(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Sound functions                                                           */

static int script_sound_load(lua_State *L)
{
    lua_pushsound(L, sound_create(script_getstring("sound_load", L, -1)));
    return 1;
}

static int script_sound_free(lua_State *L)
{
    sound_delete(script_getsound("sound_free", L, -1));
    return 0;
}

static int script_sound_stop(lua_State *L)
{
    sound_stop(script_getsound("sound_stop", L, -1));
    return 0;
}

static int script_sound_play(lua_State *L)
{
    sound_play(script_getsound("sound_play", L, -1));
    return 0;
}

static int script_sound_loop(lua_State *L)
{
    sound_loop(script_getsound("sound_loop", L, -1));
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

    lua_function(L, "create_camera",           script_create_camera);
    lua_function(L, "create_sprite",           script_create_sprite);
    lua_function(L, "create_object",           script_create_object);
    lua_function(L, "create_galaxy",           script_create_galaxy);
    lua_function(L, "create_light",            script_create_light);
    lua_function(L, "create_pivot",            script_create_pivot);

    /* Entity control. */

    lua_function(L, "entity_parent",           script_entity_parent);
    lua_function(L, "entity_clone",            script_entity_clone);
    lua_function(L, "entity_delete",           script_entity_delete);

    lua_function(L, "entity_position",         script_entity_position);
    lua_function(L, "entity_rotation",         script_entity_rotation);
    lua_function(L, "entity_scale",            script_entity_scale);
    lua_function(L, "entity_alpha",            script_entity_alpha);
    lua_function(L, "entity_flag",             script_entity_flag);

    lua_function(L, "entity_get_position",     script_entity_get_position);
    lua_function(L, "entity_get_rotation",     script_entity_get_rotation);
    lua_function(L, "entity_get_scale",        script_entity_get_scale);
    lua_function(L, "entity_get_alpha",        script_entity_get_alpha);

    lua_constant(L, "entity_flag_hidden",      FLAG_HIDDEN);
    lua_constant(L, "entity_flag_wireframe",   FLAG_WIREFRAME);
    lua_constant(L, "entity_flag_billboard",   FLAG_BILLBOARD);
    lua_constant(L, "entity_flag_unlit",       FLAG_UNLIT);

    /* Galaxy control. */

    lua_function(L, "galaxy_magn",             script_galaxy_magn);

    /* Light control. */

    lua_function(L, "light_color",             script_light_color);

    /* Camera control. */

    lua_function(L, "camera_dist",             script_camera_dist);
    lua_function(L, "camera_zoom",             script_camera_zoom);

    lua_constant(L, "camera_type_orthogonal",  CAMERA_ORTHO);
    lua_constant(L, "camera_type_perspective", CAMERA_PERSP);

    /* Sprite control. */

    lua_function(L, "sprite_bounds",           script_sprite_bounds);

    /* Light control. */

    lua_constant(L, "light_type_positional",   LIGHT_POSITIONAL);
    lua_constant(L, "light_type_directional",  LIGHT_DIRECTIONAL);

    /* Sound control. */

    lua_function(L, "sound_load",              script_sound_load);
    lua_function(L, "sound_free",              script_sound_free);
    lua_function(L, "sound_stop",              script_sound_stop);
    lua_function(L, "sound_play",              script_sound_play);
    lua_function(L, "sound_loop",              script_sound_loop);

    /* Misc. */

    lua_function(L, "add_tile",                script_add_tile);
    lua_function(L, "enable_idle",             script_enable_idle);
    lua_function(L, "joystick_axis",           script_joystick_axis);
    lua_function(L, "viewport_get",            script_viewport_get);

    /* Register the "electro" environment table globally. */

    lua_settable(L, LUA_GLOBALSINDEX);
}

int script_init(void)
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

void script_free(void)
{
    lua_close(L);
}

void script_file(const char *filename)
{
    lua_getglobal(L, "dofile");

    if (lua_isfunction(L, -1))
    {
        lua_pushstring(L, filename);

        if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN)
        {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    else lua_pop(L, 1);
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

int script_start(void)
{
    const char *name = "do_start";

    lua_getglobal(L, name);

    if (lua_isfunction(L, -1))
        return lua_callassert(L, 0, 1, name);
    else
        lua_pop(L, 1);

    return 0;
}

int script_point(int x, int y)
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

int script_click(int b, int s)
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

int script_timer(int t)
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

int script_keyboard(int k, int s)
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

int script_joystick(int n, int b, int s)
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
