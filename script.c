#include <lua.h>
#include <lualib.h>

#include "server.h"
#include "script.h"

/*---------------------------------------------------------------------------*/

static lua_State *L;

/*---------------------------------------------------------------------------*/

static float script_getnumber(const char * name, lua_State *L, int i)
{
    int n = lua_gettop(L);

    if (1 <= -i && -i <= n)
    {
        if (lua_isnumber(L, i))
            return (float) lua_tonumber(L, i);
        else
            lua_pushfstring(L, "'%s' expected number, got %s", name,
                            lua_typename(L, lua_type(L, i)));
    }
    else
        lua_pushfstring(L, "'%s' expected %d parameters, got %d", name, -i, n);

    lua_error(L);

    return 0.0;
}

/*---------------------------------------------------------------------------*/

int script_send_draw(lua_State *L)
{
    server_send_draw();
}

int script_send_move(lua_State *L)
{
    const char *name = "scene_move";
    server_send_move(script_getnumber(name, L, -3),
                     script_getnumber(name, L, -2),
                     script_getnumber(name, L, -1));
}

int script_send_turn(lua_State *L)
{
    const char *name = "camera_turn";
    server_send_turn(script_getnumber(name, L, -3),
                     script_getnumber(name, L, -2),
                     script_getnumber(name, L, -1));
}

int script_send_zoom(lua_State *L)
{
    const char *name = "camera_zoom";
    server_send_zoom(script_getnumber(name, L, -1));
}

int script_send_dist(lua_State *L)
{
    const char *name = "camera_dist";
    server_send_dist(script_getnumber(name, L, -1));
}

int script_send_magn(lua_State *L)
{
    const char *name = "camera_magn";
    server_send_magn(script_getnumber(name, L, -1));
}

/*---------------------------------------------------------------------------*/

int script_init(void)
{
    if ((L = lua_open()))
    {
        luaopen_io(L);
        luaopen_base(L);
        luaopen_math(L);

        lua_register(L, "scene_draw",  script_send_draw);
        lua_register(L, "camera_move", script_send_move);
        lua_register(L, "camera_turn", script_send_turn);
        lua_register(L, "camera_zoom", script_send_zoom);
        lua_register(L, "camera_dist", script_send_dist);
        lua_register(L, "camera_magn", script_send_magn);

        return 1;
    }
    else
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

void script_point(int x, int y)
{
    lua_getglobal(L, "do_point");

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber(L, (lua_Number) x);
        lua_pushnumber(L, (lua_Number) y);

        lua_call(L, 2, 0);
    }
}

void script_click(int b, int s)
{
    lua_getglobal(L, "do_click");

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber(L, (lua_Number) b);
        lua_pushnumber(L, (lua_Number) s);

        lua_call(L, 2, 0);
    }
}

void script_keybd(int k, int s)
{
    lua_getglobal(L, "do_keybd");

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber(L, (lua_Number) k);
        lua_pushnumber(L, (lua_Number) s);

        lua_call(L, 2, 0);
    }
}

/*---------------------------------------------------------------------------*/
