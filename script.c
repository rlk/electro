#include <lua.h>
#include <lualib.h>

#include "server.h"
#include "script.h"

/*---------------------------------------------------------------------------*/

static lua_State *L;

/*---------------------------------------------------------------------------*/

int script_init(void)
{
    if ((L = lua_open()))
    {
        luaopen_io(L);
        luaopen_base(L);
        luaopen_math(L);
        luaopen_table(L);
        luaopen_string(L);

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
}

/*---------------------------------------------------------------------------*/

void script_point(int x, int y)
{
}

void script_click(int b, int s)
{
}

void script_keybd(int k, int s)
{
}

/*---------------------------------------------------------------------------*/
