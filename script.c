/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    TOTAL PERSPECTIVE VORTEX is free software;  you can redistribute it    */
/*    and/or modify it under the terms of the  GNU General Public License    */
/*    as published by the  Free Software Foundation;  either version 2 of    */
/*    the License, or (at your option) any later version.                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#include <lua.h>
#include <lualib.h>
#include <stdio.h>

#include "shared.h"
#include "server.h"
#include "camera.h"
#include "object.h"
#include "sprite.h"
#include "entity.h"
#include "script.h"

/*---------------------------------------------------------------------------*/

static lua_State *L;

/*---------------------------------------------------------------------------*/
/* Entity userdata handlers                                                  */

static int lua_toentity(lua_State *L, int i)
{
    return ((int *) lua_touserdata(L, i))[0];
}

static int lua_isentity(lua_State *L, int i)
{
    return lua_isuserdata(L, i);
}

static void lua_pushentity(lua_State *L, int id)
{
    int *p = (int *) lua_newuserdata(L, sizeof (int));
    *p = id;
}

/*
static int lua_issprite(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && entity_istype(lua_toentity(L, i), TYPE_SPRITE);
}
*/

/*---------------------------------------------------------------------------*/
/* Function argument error reporters                                         */

static void script_type_error(const char *name,
                              const char *type, lua_State *L, int i)
{
    const char *got = "unknown";

    if (lua_isuserdata(L, i))
    {
        switch (((int *) lua_touserdata(L, i))[0])
        {
        /*
        case TYPE_SPRITE: got = "sprite"; break;
        case TYPE_OBJECT: got = "object"; break;
        */
        }
    }
    else got = lua_typename(L, lua_type(L, i));

    lua_pushfstring(L, "'%s' expected %s, got %s", name, type, got);
    lua_error(L);
}

static void script_arity_error(const char *name, lua_State *L, int i, int n)
{
    lua_pushfstring(L, "'%s' expected %d parameters, got %d", name, -i, n);
    lua_error(L);
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

/*---------------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------------*/

static int script_camera_move(lua_State *L)
{
    const char *name = "camera_move";

    camera_set_org(script_getnumber(name, L, -3),
                   script_getnumber(name, L, -2),
                   script_getnumber(name, L, -1));
    return 0;
}

static int script_camera_turn(lua_State *L)
{
    const char *name = "camera_turn";

    camera_set_rot(script_getnumber(name, L, -3),
                   script_getnumber(name, L, -2),
                   script_getnumber(name, L, -1));
    return 0;
}

static int script_camera_dist(lua_State *L)
{
    const char *name = "camera_dist";

    camera_set_dist(script_getnumber(name, L, -1));
    return 0;
}

static int script_camera_magn(lua_State *L)
{
    const char *name = "camera_magn";

    camera_set_magn(script_getnumber(name, L, -1));
    return 0;
}

static int script_camera_zoom(lua_State *L)
{
    const char *name = "camera_zoom";

    camera_set_zoom(script_getnumber(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/

static int script_sprite_create(lua_State *L)
{
    const char *name = "sprite_create";

    lua_pushentity(L, sprite_create(script_getstring(name, L, -1)));
    return 1;
}

static int script_entity_parent(lua_State *L)
{
    const char *name = "entity_parent";

    entity_parent(script_getentity(name, L, -2),
                  script_getentity(name, L, -1));
    return 0;
}

static int script_entity_delete(lua_State *L)
{
    const char *name = "entity_delete";

    entity_delete(script_getentity(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/

static int script_entity_position(lua_State *L)
{
    const char *name = "entity_position";

    entity_position(script_getentity(name, L, -4),
                    script_getnumber(name, L, -3),
                    script_getnumber(name, L, -2),
                    script_getnumber(name, L, -1));
    return 0;
}

static int script_entity_rotation(lua_State *L)
{
    const char *name = "entity_rotation";

    entity_rotation(script_getentity(name, L, -4),
                    script_getnumber(name, L, -3),
                    script_getnumber(name, L, -2),
                    script_getnumber(name, L, -1));
    return 0;
}

static int script_entity_scale(lua_State *L)
{
    const char *name = "entity_scale";

    entity_scale(script_getentity(name, L, -4),
                 script_getnumber(name, L, -3),
                 script_getnumber(name, L, -2),
                 script_getnumber(name, L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Script setup/shutdown                                                     */

int script_init(void)
{
    if ((L = lua_open()))
    {
        luaopen_base(L);
        luaopen_math(L);
        luaopen_io(L);

        lua_register(L, "add_tile",    script_add_tile);

        lua_register(L, "enable_idle", script_enable_idle);

        lua_register(L, "camera_move", script_camera_move);
        lua_register(L, "camera_turn", script_camera_turn);
        lua_register(L, "camera_dist", script_camera_dist);
        lua_register(L, "camera_magn", script_camera_magn);
        lua_register(L, "camera_zoom", script_camera_zoom);

        lua_register(L, "sprite_create",   script_sprite_create);
        lua_register(L, "entity_parent",   script_entity_parent);
        lua_register(L, "entity_delete",   script_entity_delete);

        lua_register(L, "entity_position", script_entity_position);
        lua_register(L, "entity_rotation", script_entity_rotation);
        lua_register(L, "entity_scale",    script_entity_scale);

/*
        lua_register(L, "sprite_load", script_sprite_load);
        lua_register(L, "sprite_free", script_sprite_free);
        lua_register(L, "sprite_fade", script_sprite_fade);
*/
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

int script_point(int x, int y)
{
    int r = 0;

    lua_getglobal(L, "do_point");

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber(L, (lua_Number) x);
        lua_pushnumber(L, (lua_Number) y);

        if (lua_pcall(L, 2, 1, 0) == LUA_ERRRUN)
            fprintf(stderr, "(do_point): %s\n", lua_tostring(L, -1));
        else
            r = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    return r;
}

int script_click(int b, int s)
{
    int r = 0;

    lua_getglobal(L, "do_click");

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber (L, (lua_Number) b);
        lua_pushboolean(L, s);

        if (lua_pcall(L, 2, 1, 0) == LUA_ERRRUN)
            fprintf(stderr, "(do_click): %s\n", lua_tostring(L, -1));
        else
            r = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    return r;
}

int script_keybd(int k, int s)
{
    int r = 0;

    lua_getglobal(L, "do_keybd");

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber (L, (lua_Number) k);
        lua_pushboolean(L, s);

        if (lua_pcall(L, 2, 1, 0) == LUA_ERRRUN)
            fprintf(stderr, "(do_keybd): %s\n", lua_tostring(L, -1));
        else
            r = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    return r;
}

int script_timer(int t)
{
    int r = 0;

    lua_getglobal(L, "do_timer");

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber(L, (lua_Number) (t / 1000.0f));

        if (lua_pcall(L, 1, 1, 0) == LUA_ERRRUN)
            fprintf(stderr, "(do_timer): %s\n", lua_tostring(L, -1));
        else
            r = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    return r;
}

/*---------------------------------------------------------------------------*/
