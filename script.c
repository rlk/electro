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

#include "server.h"
#include "status.h"
#include "shared.h"
#include "script.h"

/*---------------------------------------------------------------------------*/

static lua_State *L;

/*---------------------------------------------------------------------------*/

static const char *script_getstring(const char *str, lua_State *L, int i)
{
    int n = lua_gettop(L);

    if (1 <= -i && -i <= n)
    {
        if (lua_isstring(L, i))
            return lua_tostring(L, i);
        else
            lua_pushfstring(L, "'%s' expected string, got %s", str,
                            lua_typename(L, lua_type(L, i)));
    }
    else lua_pushfstring(L, "'%s' expected %d parameters, got %d", str, -i, n);

    lua_error(L);

    return "";
}

static float script_getnumber(const char *str, lua_State *L, int i)
{
    int n = lua_gettop(L);

    if (1 <= -i && -i <= n)
    {
        if (lua_isnumber(L, i))
            return (float) lua_tonumber(L, i);
        else
            lua_pushfstring(L, "'%s' expected number, got %s", str,
                            lua_typename(L, lua_type(L, i)));
    }
    else lua_pushfstring(L, "'%s' expected %d parameters, got %d", str, -i, n);

    lua_error(L);

    return 0.0;
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

static int script_camera_move(lua_State *L)
{
    const char *name = "camera_move";
    status_set_camera_org(script_getnumber(name, L, -3),
                          script_getnumber(name, L, -2),
                          script_getnumber(name, L, -1));
    server_send_move();
    return 0;
}

static int script_camera_turn(lua_State *L)
{
    const char *name = "camera_turn";
    status_set_camera_rot(script_getnumber(name, L, -3),
                          script_getnumber(name, L, -2),
                          script_getnumber(name, L, -1));
    server_send_turn();
    return 0;
}

static int script_camera_dist(lua_State *L)
{
    const char *name = "camera_dist";
    status_set_camera_dist(script_getnumber(name, L, -1));
    server_send_dist();
    return 0;
}

static int script_camera_magn(lua_State *L)
{
    const char *name = "camera_magn";
    status_set_camera_magn(script_getnumber(name, L, -1));
    server_send_magn();
    return 0;
}

static int script_camera_zoom(lua_State *L)
{
    const char *name = "camera_zoom";
    status_set_camera_zoom(script_getnumber(name, L, -1));
    server_send_zoom();
    return 0;
}

/*---------------------------------------------------------------------------*/

int script_init(void)
{
    if ((L = lua_open()))
    {
        luaopen_io(L);
        luaopen_base(L);
        luaopen_math(L);

        lua_register(L, "add_tile",    script_add_tile);

        lua_register(L, "camera_move", script_camera_move);
        lua_register(L, "camera_turn", script_camera_turn);
        lua_register(L, "camera_dist", script_camera_dist);
        lua_register(L, "camera_magn", script_camera_magn);
        lua_register(L, "camera_zoom", script_camera_zoom);

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
    lua_dofile(L, filename);
}

/*---------------------------------------------------------------------------*/

int script_point(int x, int y)
{
    int r = 0;

    lua_getglobal(L, "do_point");

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber(L, (lua_Number) x);
        lua_pushnumber(L, (lua_Number) y);

        lua_call(L, 2, 1);

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

        lua_call(L, 2, 1);

        r = lua_toboolean(L, -1);
    }
    else lua_pop(L, 1);

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

        lua_call(L, 2, 1);

        r = lua_toboolean(L, -1);
    }
    else lua_pop(L, 1);

    return r;
}

/*---------------------------------------------------------------------------*/
