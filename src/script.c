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
#include <luasocket.h>
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
#include "string.h"
#include "galaxy.h"
#include "light.h"
#include "pivot.h"
#include "utility.h"
#include "entity.h"
#include "sound.h"
#include "font.h"
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

static int E_tousertype(lua_State *L, int i)
{
    return ((int *) lua_touserdata(L, i))[0];
}

static int E_touserdata(lua_State *L, int i)
{
    return ((int *) lua_touserdata(L, i))[1];
}

static void E_pushuserdata(lua_State *L, int type, int data)
{
    int *p = (int *) lua_newuserdata(L, 2 * sizeof (int));

    p[0] = type;
    p[1] = data;
}

/*---------------------------------------------------------------------------*/
/* Entity userdata handlers                                                  */

static int E_toentity(lua_State *L, int i)
{
    return E_touserdata(L, i);
}

static int E_isentity(lua_State *L, int i)
{
    return ((lua_isuserdata(L, i)) &&
              (E_tousertype(L, i) == USERDATA_ENTITY));
}

static void E_pushentity(lua_State *L, int id)
{
    if (id <= 0)
        lua_pushnil(L);
    else
        E_pushuserdata(L, USERDATA_ENTITY, id);
}

/*---------------------------------------------------------------------------*/
/* Sound userdata handlers                                                   */

#ifndef NAUDIO

static int E_tosound(lua_State *L, int i)
{
    return E_touserdata(L, i);
}

static int E_issound(lua_State *L, int i)
{
    return ((lua_isuserdata(L, i)) &&
              (E_tousertype(L, i) == USERDATA_SOUND));
}

static void E_pushsound(lua_State *L, int id)
{
    if (id < 0)
        lua_pushnil(L);
    else
        E_pushuserdata(L, USERDATA_SOUND, id);
}

#endif

/*---------------------------------------------------------------------------*/
/* Function argument error reporters                                         */

static void E_type_error(const char *type, lua_State *L, int i)
{
    const char *name = lua_tostring(L, lua_upvalueindex(1));
    const char *got;

    if (lua_isuserdata(L, i))
        got = entity_name(E_toentity(L, i));
    else
        got = lua_typename(L, lua_type(L, i));

    error("'%s' expected %s, got %s", name, type, got);
}

static void E_arity_error(lua_State *L, int i)
{
    const char *name = lua_tostring(L, lua_upvalueindex(1));

    error("'%s' expected %d parameters, got %d", name, -i, lua_gettop(L));
}

/*---------------------------------------------------------------------------*/
/* Type checking functions                                                   */

static int E_iscamera(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(E_toentity(L, i)) == TYPE_CAMERA);
}

static int E_isgalaxy(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(E_toentity(L, i)) == TYPE_GALAXY);
}

static int E_issprite(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(E_toentity(L, i)) == TYPE_SPRITE);
}

static int E_isstring(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(E_toentity(L, i)) == TYPE_STRING);
}

static int E_islight(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(E_toentity(L, i)) == TYPE_LIGHT);
}

/*---------------------------------------------------------------------------*/
/* Function argument type and arity checkers                                 */

static const char *L_getstring(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isnil(L, i))
            return NULL;

        if (lua_isstring(L, i))
            return lua_tostring(L, i);
        else
            E_type_error("string", L, i);
    }
    else E_arity_error(L, i);

    return "";
}

static int L_getboolean(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isboolean(L, i))
            return lua_toboolean(L, i);
        else
            E_type_error("boolean", L, i);
    }
    else E_arity_error(L, i);

    return 0;
}

static float L_getnumber(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isnumber(L, i))
            return (float) lua_tonumber(L, i);
        else
            E_type_error("number", L, i);
    }
    else E_arity_error(L, i);

    return 0.0;
}

static int L_getinteger(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isnumber(L, i))
            return (int) lua_tonumber(L, i);
        else
            E_type_error("number", L, i);
    }
    else E_arity_error(L, i);

    return 0.0;
}

/*---------------------------------------------------------------------------*/

static int E_getentity(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isnil (L, i)) return 0;
        if (E_isentity(L, i)) return E_toentity(L, i);

        E_type_error("entity", L, i);
    }
    else E_arity_error(L, i);

    return 0;
}

static int E_getcamera(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (E_iscamera(L, i))
            return entity_data(E_toentity(L, i));
        else
            E_type_error("camera", L, i);
    }
    else E_arity_error(L, i);

    return 0;
}

static int E_getgalaxy(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (E_isgalaxy(L, i))
            return entity_data(E_toentity(L, i));
        else
            E_type_error("galaxy", L, i);
    }
    else E_arity_error(L, i);

    return 0;
}

static int E_getsprite(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (E_issprite(L, i))
            return entity_data(E_toentity(L, i));
        else
            E_type_error("sprite", L, i);
    }
    else E_arity_error(L, i);

    return 0;
}

static int E_getstring(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (E_isstring(L, i))
            return entity_data(E_toentity(L, i));
        else
            E_type_error("string", L, i);
    }
    else E_arity_error(L, i);

    return 0;
}

static int E_getlight(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (E_islight(L, i))
            return entity_data(E_toentity(L, i));
        else
            E_type_error("light", L, i);
    }
    else E_arity_error(L, i);

    return 0;
}

static int E_getsound(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
#ifndef NAUDIO
        if (E_issound(L, i))
            return E_tosound(L, i);
        else
            E_type_error("sound", L, i);
#else
        lua_pushnil(L);
        return 1;
#endif
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Display configuration                                                     */

static int E_add_host(lua_State *L)
{
    int i = add_host(L_getstring (L, -5),
                     L_getinteger(L, -4),
                     L_getinteger(L, -3),
                     L_getinteger(L, -2),
                     L_getinteger(L, -1));

    lua_pushnumber(L, i);
    return 1;
}

static int E_set_host_flag(lua_State *L)
{
    send_set_host_flag(L_getinteger(L, -3),
                       L_getinteger(L, -2),
                       L_getboolean(L, -1));
    return 0;
}

static int E_add_tile(lua_State *L)
{
    int j = send_add_tile(L_getinteger(L, -5),
                          L_getinteger(L, -4),
                          L_getinteger(L, -3),
                          L_getinteger(L, -2),
                          L_getinteger(L, -1));
    lua_pushnumber(L, j);
    return 1;
}

static int E_set_tile_flag(lua_State *L)
{
    send_set_tile_flag(L_getinteger(L, -3),
                       L_getinteger(L, -2),
                       L_getboolean(L, -1));
    return 0;
}

static int E_set_tile_position(lua_State *L)
{
    int j = L_getinteger(L, -10);

    float o[3];
    float r[3];
    float u[3];

    o[0] = L_getnumber(L, -9);
    o[1] = L_getnumber(L, -8);
    o[2] = L_getnumber(L, -7);
    r[0] = L_getnumber(L, -6);
    r[1] = L_getnumber(L, -5);
    r[2] = L_getnumber(L, -4);
    u[0] = L_getnumber(L, -3);
    u[1] = L_getnumber(L, -2);
    u[2] = L_getnumber(L, -1);

    send_set_tile_position(j, o, r, u);
    return 0;
}

static int E_set_tile_viewport(lua_State *L)
{
    send_set_tile_viewport(L_getinteger(L, -5),
                           L_getinteger(L, -4),
                           L_getinteger(L, -3),
                           L_getinteger(L, -2),
                           L_getinteger(L, -1));
    return 0;
}

static int E_set_tile_line_screen(lua_State *L)
{
    send_set_tile_line_screen(L_getinteger(L, -6),
                              L_getnumber (L, -5),
                              L_getnumber (L, -4),
                              L_getnumber (L, -3),
                              L_getnumber (L, -2),
                              L_getnumber (L, -1));
    return 0;
}

static int E_set_tile_view_mirror(lua_State *L)
{
    float p[4];

    p[0] = L_getnumber(L, -4);
    p[1] = L_getnumber(L, -3);
    p[2] = L_getnumber(L, -2);
    p[3] = L_getnumber(L, -1);

    send_set_tile_view_mirror(L_getinteger(L, -5), p);

    return 0;
}

static int E_set_tile_view_offset(lua_State *L)
{
    float d[4];

    d[0] = L_getnumber(L, -3);
    d[1] = L_getnumber(L, -2);
    d[2] = L_getnumber(L, -1);

    send_set_tile_view_offset(L_getinteger(L, -4), d);

    return 0;
}

static int E_get_display_union(lua_State *L)
{
    float b[4];

    get_display_union(b);

    lua_pushnumber(L, b[0]);
    lua_pushnumber(L, b[1]);
    lua_pushnumber(L, b[2]);
    lua_pushnumber(L, b[3]);

    return 4;
}

static int E_get_display_bound(lua_State *L)
{
    float b[6];

    get_display_bound(b);

    lua_pushnumber(L, b[0]);
    lua_pushnumber(L, b[1]);
    lua_pushnumber(L, b[2]);
    lua_pushnumber(L, b[3]);
    lua_pushnumber(L, b[4]);
    lua_pushnumber(L, b[5]);

    return 6;
}

/*---------------------------------------------------------------------------*/
/* Miscellaneous functions                                                   */

static int E_enable_timer(lua_State *L)
{
    enable_timer(L_getboolean(L, -1));
    return 0;
}

static int E_get_joystick(lua_State *L)
{
    float a[2];

    get_joystick(L_getinteger(L, -1), a);

    lua_pushnumber(L, (double) a[0]);
    lua_pushnumber(L, (double) a[1]);
    return 2;
}

static int E_get_modifier(lua_State *L)
{
    int i = L_getinteger(L, -1);

    lua_pushboolean(L, (SDL_GetModState() & i) ? 1 : 0);

    return 1;
}

static int E_set_typeface(lua_State *L)
{
    float e = 0.001f;
    float o = 0.010f;

    switch (lua_gettop(L))
    {
    case 1: send_set_font(L_getstring(L, -1), e, o); break;
    case 2: send_set_font(L_getstring(L, -2),
                          L_getnumber(L, -1), o);    break;
    case 3: send_set_font(L_getstring(L, -3),
                          L_getnumber(L, -2),
                          L_getnumber(L, -1));       break;
    }

    return 0;
}

static int E_set_background(lua_State *L)
{
    int n = lua_gettop(L);
    float c0[3];
    float c1[3];

    if (n == 6)
    {
        c0[0] = L_getnumber(L, -6);
        c0[1] = L_getnumber(L, -5);
        c0[2] = L_getnumber(L, -4);
        c1[0] = L_getnumber(L, -3);
        c1[1] = L_getnumber(L, -2);
        c1[2] = L_getnumber(L, -1);

        send_set_background(c0, c1);
    }
    else
    {
        c0[0] = L_getnumber(L, -3);
        c0[1] = L_getnumber(L, -2);
        c0[2] = L_getnumber(L, -1);

        send_set_background(c0, c0);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Entity hierarchy functions                                                */

static int E_parent_entity(lua_State *L)
{
    send_parent_entity(E_getentity(L, -2),
                       E_getentity(L, -1));
    return 0;
}

static int E_delete_entity(lua_State *L)
{
    send_delete_entity(E_getentity(L, -1));
    return 0;
}

static int E_create_clone(lua_State *L)
{
    int id = send_create_clone(E_getentity(L, -1));

    E_pushentity(L, id);
    return 1;
}

static int E_get_entity_parent(lua_State *L)
{
    int id = get_entity_parent(E_getentity(L, -1));

    E_pushentity(L, id);
    return 1;
}

static int E_get_entity_child(lua_State *L)
{
    int id = get_entity_child(E_getentity (L, -2),
                            L_getinteger(L, -1));

    E_pushentity(L, id);
    return 1;
}

/*---------------------------------------------------------------------------*/
/* Entity transform functions                                                */

static int E_set_entity_position(lua_State *L)
{
    float p[3];

    p[0] = L_getnumber(L, -3);
    p[1] = L_getnumber(L, -2);
    p[2] = L_getnumber(L, -1);

    send_set_entity_position(E_getentity(L, -4), p);
    return 0;
}

static int E_set_entity_rotation(lua_State *L)
{
    float r[3];

    r[0] = L_getnumber(L, -3);
    r[1] = L_getnumber(L, -2);
    r[2] = L_getnumber(L, -1);

    send_set_entity_rotation(E_getentity(L, -4), r);
    return 0;
}

static int E_set_entity_scale(lua_State *L)
{
    float s[3];

    s[0] = L_getnumber(L, -3);
    s[1] = L_getnumber(L, -2);
    s[2] = L_getnumber(L, -1);

    send_set_entity_scale(E_getentity(L, -4), s);
    return 0;
}

static int E_set_entity_bound(lua_State *L)
{
    float b[6];

    b[0] = L_getnumber(L, -6);
    b[0] = L_getnumber(L, -5);
    b[0] = L_getnumber(L, -4);
    b[0] = L_getnumber(L, -3);
    b[1] = L_getnumber(L, -2);
    b[2] = L_getnumber(L, -1);

    send_set_entity_bound(E_getentity(L, -7), b);
    return 0;
}

static int E_set_entity_alpha(lua_State *L)
{
    send_set_entity_alpha(E_getentity(L, -2),
                          L_getnumber(L, -1));
    return 0;
}

static int E_set_entity_flag(lua_State *L)
{
    send_set_entity_flag(E_getentity (L, -3),
                         L_getinteger(L, -2),
                         L_getboolean(L, -1));
    return 0;
}

static int E_set_entity_frag_prog(lua_State *L)
{
    int id           = E_getentity(L, -2);
    const char *file = L_getstring(L, -1);
    char       *text = load_file(file, "r", NULL);

    send_set_entity_frag_prog(id, text);
    if (text) free(text);

    return 0;
}

static int E_set_entity_vert_prog(lua_State *L)
{
    int id           = E_getentity(L, -2);
    const char *file = L_getstring(L, -1);
    char       *text = load_file(file, "r", NULL);

    send_set_entity_vert_prog(id, text);
    if (text) free(text);

    return 0;
}

/*---------------------------------------------------------------------------*/

static int E_move_entity(lua_State *L)
{
    float v[3];

    v[0] = L_getnumber(L, -3);
    v[1] = L_getnumber(L, -2);
    v[2] = L_getnumber(L, -1);

    send_move_entity(E_getentity(L, -4), v);
    return 0;
}

static int E_turn_entity(lua_State *L)
{
    float r[3];

    r[0] = L_getnumber(L, -3);
    r[1] = L_getnumber(L, -2);
    r[2] = L_getnumber(L, -1);

    send_turn_entity(E_getentity(L, -4), r);
    return 0;
}

/*---------------------------------------------------------------------------*/

static int E_get_entity_position(lua_State *L)
{
    int  id = E_getentity(L, -1);
    float p[3];

    get_entity_position(id, p);

    lua_pushnumber(L, p[0]);
    lua_pushnumber(L, p[1]);
    lua_pushnumber(L, p[2]);

    return 3;
}

static int E_get_entity_x_vector(lua_State *L)
{
    int  id = E_getentity(L, -1);
    float v[3];

    get_entity_x_vector(id, v);

    lua_pushnumber(L, v[0]);
    lua_pushnumber(L, v[1]);
    lua_pushnumber(L, v[2]);

    return 3;
}

static int E_get_entity_y_vector(lua_State *L)
{
    int  id = E_getentity(L, -1);
    float v[3];

    get_entity_y_vector(id, v);

    lua_pushnumber(L, v[0]);
    lua_pushnumber(L, v[1]);
    lua_pushnumber(L, v[2]);

    return 3;
}

static int E_get_entity_z_vector(lua_State *L)
{
    int  id = E_getentity(L, -1);
    float v[3];

    get_entity_z_vector(id, v);

    lua_pushnumber(L, v[0]);
    lua_pushnumber(L, v[1]);
    lua_pushnumber(L, v[2]);

    return 3;
}

static int E_get_entity_scale(lua_State *L)
{
    int  id = E_getentity(L, -1);
    float s[3];

    get_entity_scale(id, s);

    lua_pushnumber(L, s[0]);
    lua_pushnumber(L, s[1]);
    lua_pushnumber(L, s[2]);

    return 3;
}

static int E_get_entity_alpha(lua_State *L)
{
    int  id = E_getentity(L, -1);
    float a = get_entity_alpha(id);

    lua_pushnumber(L, a);

    return 1;
}

static int E_get_entity_bound(lua_State *L)
{
    int  id = E_getentity(L, -1);
    float b[6];

    get_entity_bound(id, b);

    lua_pushnumber(L, b[0]);
    lua_pushnumber(L, b[1]);
    lua_pushnumber(L, b[2]);
    lua_pushnumber(L, b[3]);
    lua_pushnumber(L, b[4]);
    lua_pushnumber(L, b[5]);

    return 6;
}

/*---------------------------------------------------------------------------*/
/* Entity constructors.                                                      */

static int E_create_camera(lua_State *L)
{
    int fl = L_getinteger(L, -1);
    int id = send_create_camera(fl);

    E_pushentity(L, id);
    return 1;
}

static int E_create_sprite(lua_State *L)
{
    int id = send_create_sprite(L_getstring(L, -1));

    E_pushentity(L, id);
    return 1;
}

static int E_create_object(lua_State *L)
{
    int id = send_create_object(L_getstring(L, -1));

    E_pushentity(L, id);
    return 1;
}

static int E_create_string(lua_State *L)
{
    int id = send_create_string(L_getstring(L, -1));

    E_pushentity(L, id);
    return 1;
}

static int E_create_galaxy(lua_State *L)
{
    int id = send_create_galaxy(L_getstring(L, -1));

    E_pushentity(L, id);
    return 1;
}

static int E_create_light(lua_State *L)
{
    int fl = L_getinteger(L, -1);
    int id = send_create_light(fl);

    E_pushentity(L, id);
    return 1;
}

static int E_create_pivot(lua_State *L)
{
    int id = send_create_pivot();

    E_pushentity(L, id);
    return 1;
}

/*---------------------------------------------------------------------------*/
/* Galaxy controls                                                           */

static int E_set_galaxy_magnitude(lua_State *L)
{
    send_set_galaxy_magnitude(E_getgalaxy(L, -2),
                              L_getnumber(L, -1));
    return 0;
}

static int E_get_star_index(lua_State *L)
{
    int gd = E_getgalaxy(L, -2);
    int id = E_getentity(L, -1);

    float p[3];
    float v[3];

    get_entity_position(id, p);
    get_entity_z_vector(id, v);

    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];

    lua_pushnumber(L, pick_galaxy(gd, p, v));
    return 1;
}

static int E_get_star_position(lua_State *L)
{
    float p[3];

    get_star_position(E_getgalaxy (L, -2),
                      L_getinteger(L, -1), p);

    lua_pushnumber(L, p[0]);
    lua_pushnumber(L, p[1]);
    lua_pushnumber(L, p[2]);

    return 3;
}

/*---------------------------------------------------------------------------*/
/* Camera controls                                                           */

static int E_set_camera_offset(lua_State *L)
{
    float v[3], M[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };

    v[0] = L_getnumber(L, -3);
    v[1] = L_getnumber(L, -2);
    v[2] = L_getnumber(L, -1);

    send_set_camera_offset(E_getcamera(L, -4), v, M);
    return 0;
}

static int E_set_camera_stereo(lua_State *L)
{
    float l[3];
    float r[3];

    l[0] = L_getnumber(L, -6);
    l[1] = L_getnumber(L, -5);
    l[2] = L_getnumber(L, -4);
    r[0] = L_getnumber(L, -3);
    r[1] = L_getnumber(L, -2);
    r[2] = L_getnumber(L, -1);

    send_set_camera_stereo(E_getcamera(L, -8), l, r, L_getinteger(L, -7));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Light controls                                                            */

static int E_set_light_color(lua_State *L)
{
    send_set_light_color(E_getlight (L, -4),
                         L_getnumber(L, -3),
                         L_getnumber(L, -2),
                         L_getnumber(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* String controls                                                           */

static int E_set_string_fill(lua_State *L)
{
    send_set_string_fill(E_getstring(L, -5),
                         L_getnumber(L, -4),
                         L_getnumber(L, -3),
                         L_getnumber(L, -2),
                         L_getnumber(L, -1));
    return 0;
}

static int E_set_string_line(lua_State *L)
{
    send_set_string_line(E_getstring(L, -5),
                         L_getnumber(L, -4),
                         L_getnumber(L, -3),
                         L_getnumber(L, -2),
                         L_getnumber(L, -1));
    return 0;
}

static int E_set_string_text(lua_State *L)
{
    send_set_string_text(E_getstring(L, -2),
                         L_getstring(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Sprite controls                                                           */

static int E_set_sprite_range(lua_State *L)
{
    send_set_sprite_range(E_getsprite(L, -5),
                          L_getnumber(L, -4),
                          L_getnumber(L, -3),
                          L_getnumber(L, -2),
                          L_getnumber(L, -1));
    return 0;
}

static int E_get_sprite_pixel(lua_State *L)
{
    unsigned char p[4];

    get_sprite_p(E_getsprite (L, -3),
                 L_getinteger(L, -2),
                 L_getinteger(L, -1), p);

    lua_pushnumber(L, (double) p[0] / 255.0);
    lua_pushnumber(L, (double) p[1] / 255.0);
    lua_pushnumber(L, (double) p[2] / 255.0);
    lua_pushnumber(L, (double) p[3] / 255.0);

    return 4;
}

static int E_get_sprite_size(lua_State *L)
{
    int id = E_getsprite(L, -1);

    lua_pushnumber(L, (double) get_sprite_w(id));
    lua_pushnumber(L, (double) get_sprite_h(id));

    return 2;
}

/*---------------------------------------------------------------------------*/
/* Sound functions                                                           */

static int E_load_sound(lua_State *L)
{
#ifndef NAUDIO
    E_pushsound(L, load_sound(L_getstring(L, -1)));
#else
    lua_pushnil(L);
#endif
    return 1;
}

static int E_free_sound(lua_State *L)
{
    free_sound(E_getsound(L, -1));
    return 0;
}

static int E_stop_sound(lua_State *L)
{
    stop_sound(E_getsound(L, -1));
    return 0;
}

static int E_play_sound(lua_State *L)
{
    play_sound(E_getsound(L, -1));
    return 0;
}

static int E_loop_sound(lua_State *L)
{
    loop_sound(E_getsound(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Console functions                                                         */

static int E_clear_console(lua_State *L)
{
    clear_console();
    return 0;
}

static int E_color_console(lua_State *L)
{
    color_console(L_getnumber(L, -3),
                  L_getnumber(L, -2),
                  L_getnumber(L, -1));
    return 0;
}

static int E_print_console(lua_State *L)
{
    int i, n = lua_gettop(L);
    const char *str;

    for (i = n; i > 0; --i)
        if ((str = L_getstring(L, -i)))
            print_console(str);

    return 1;
}

/*---------------------------------------------------------------------------*/

static int E_exit(lua_State *L)
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

void add_argument(int i, const char *arg)
{
    lua_getglobal(L, "E");

    if (lua_istable(L, -1))
    {
        lua_pushstring(L, "argument");
        lua_gettable(L, -2);

        if (lua_istable(L, -1))
        {
            lua_pushstring(L, arg);
            lua_rawseti(L, -2, i);
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

    lua_function(L, "create_camera",        E_create_camera);
    lua_function(L, "create_sprite",        E_create_sprite);
    lua_function(L, "create_object",        E_create_object);
    lua_function(L, "create_string",        E_create_string);
    lua_function(L, "create_galaxy",        E_create_galaxy);
    lua_function(L, "create_light",         E_create_light);
    lua_function(L, "create_pivot",         E_create_pivot);
    lua_function(L, "create_clone",         E_create_clone);

    /* Entity control. */

    lua_function(L, "parent_entity",        E_parent_entity);
    lua_function(L, "delete_entity",        E_delete_entity);

    lua_function(L, "get_entity_parent",    E_get_entity_parent);
    lua_function(L, "get_entity_child",     E_get_entity_child);

    lua_function(L, "set_entity_position",  E_set_entity_position);
    lua_function(L, "set_entity_rotation",  E_set_entity_rotation);
    lua_function(L, "set_entity_scale",     E_set_entity_scale);
    lua_function(L, "set_entity_bound",     E_set_entity_bound);
    lua_function(L, "set_entity_alpha",     E_set_entity_alpha);
    lua_function(L, "set_entity_flag",      E_set_entity_flag);
    lua_function(L, "set_entity_frag_prog", E_set_entity_frag_prog);
    lua_function(L, "set_entity_vert_prog", E_set_entity_vert_prog);

    lua_function(L, "get_entity_position",  E_get_entity_position);
    lua_function(L, "get_entity_x_vector",  E_get_entity_x_vector);
    lua_function(L, "get_entity_y_vector",  E_get_entity_y_vector);
    lua_function(L, "get_entity_z_vector",  E_get_entity_z_vector);
    lua_function(L, "get_entity_scale",     E_get_entity_scale);
    lua_function(L, "get_entity_bound",     E_get_entity_bound);
    lua_function(L, "get_entity_alpha",     E_get_entity_alpha);

    lua_function(L, "move_entity",          E_move_entity);
    lua_function(L, "turn_entity",          E_turn_entity);

    /* Galaxy control. */

    lua_function(L, "set_galaxy_magnitude", E_set_galaxy_magnitude);
    lua_function(L, "get_star_index",       E_get_star_index);
    lua_function(L, "get_star_position",    E_get_star_position);

    /* Camera control. */

    lua_function(L, "set_camera_offset",    E_set_camera_offset);
    lua_function(L, "set_camera_stereo",    E_set_camera_stereo);

    /* Light control. */

    lua_function(L, "set_light_color",      E_set_light_color);

    /* Sprite control. */

    lua_function(L, "set_sprite_range",     E_set_sprite_range);
    lua_function(L, "get_sprite_pixel",     E_get_sprite_pixel);
    lua_function(L, "get_sprite_size",      E_get_sprite_size);

    /* String control. */

    lua_function(L, "set_string_fill",      E_set_string_fill);
    lua_function(L, "set_string_line",      E_set_string_line);
    lua_function(L, "set_string_text",      E_set_string_text);

    /* Sound control. */

    lua_function(L, "load_sound",           E_load_sound);
    lua_function(L, "free_sound",           E_free_sound);
    lua_function(L, "stop_sound",           E_stop_sound);
    lua_function(L, "play_sound",           E_play_sound);
    lua_function(L, "loop_sound",           E_loop_sound);

    /* Console */

    lua_function(L, "clear_console",        E_clear_console);
    lua_function(L, "color_console",        E_color_console);
    lua_function(L, "print_console",        E_print_console);

    /* Display */

    lua_function(L, "add_host",             E_add_host);
    lua_function(L, "add_tile",             E_add_tile);
    lua_function(L, "set_host_flag",        E_set_host_flag);
    lua_function(L, "set_tile_flag",        E_set_tile_flag);
    lua_function(L, "set_tile_position",    E_set_tile_position);
    lua_function(L, "set_tile_viewport",    E_set_tile_viewport);
    lua_function(L, "set_tile_line_screen", E_set_tile_line_screen);
    lua_function(L, "set_tile_view_mirror", E_set_tile_view_mirror);
    lua_function(L, "set_tile_view_offset", E_set_tile_view_offset);
    lua_function(L, "get_display_union",    E_get_display_union);
    lua_function(L, "get_display_bound",    E_get_display_bound);

    /* Misc. */

    lua_function(L, "enable_timer",         E_enable_timer);
    lua_function(L, "get_joystick",         E_get_joystick);
    lua_function(L, "get_modifier",         E_get_modifier);
    lua_function(L, "set_typeface",         E_set_typeface);
    lua_function(L, "set_background",       E_set_background);
    lua_function(L, "exit",                 E_exit);

    /* Constants. */

    lua_constant(L, "entity_flag_hidden",        FLAG_HIDDEN);
    lua_constant(L, "entity_flag_wireframe",     FLAG_WIREFRAME);
    lua_constant(L, "entity_flag_billboard",     FLAG_BILLBOARD);
    lua_constant(L, "entity_flag_unlit",         FLAG_UNLIT);
    lua_constant(L, "entity_flag_transparent",   FLAG_TRANSPARENT);
    lua_constant(L, "entity_flag_bounded",       FLAG_BOUNDED);
    lua_constant(L, "entity_flag_line_smooth",   FLAG_LINE_SMOOTH);
    lua_constant(L, "entity_flag_pos_tracked_0", FLAG_POS_TRACKED_0);
    lua_constant(L, "entity_flag_rot_tracked_0", FLAG_ROT_TRACKED_0);
    lua_constant(L, "entity_flag_pos_tracked_1", FLAG_POS_TRACKED_1);
    lua_constant(L, "entity_flag_rot_tracked_1", FLAG_ROT_TRACKED_1);

    lua_constant(L, "host_flag_full",            HOST_FULL);
    lua_constant(L, "host_flag_stereo",          HOST_STEREO);
    lua_constant(L, "host_flag_framed",          HOST_FRAMED);
    lua_constant(L, "tile_flag_flip_x",          TILE_FLIP_X);
    lua_constant(L, "tile_flag_flip_y",          TILE_FLIP_Y);
    lua_constant(L, "tile_flag_offset",          TILE_OFFSET);
    lua_constant(L, "tile_flag_mirror",          TILE_MIRROR);
    lua_constant(L, "tile_flag_test",            TILE_TEST);

    lua_constant(L, "camera_type_orthogonal",    CAMERA_ORTHO);
    lua_constant(L, "camera_type_perspective",   CAMERA_PERSP);

    lua_constant(L, "stereo_mode_none",          STEREO_NONE);
    lua_constant(L, "stereo_mode_quad",          STEREO_QUAD);
    lua_constant(L, "stereo_mode_red_blue",      STEREO_RED_BLUE);
    lua_constant(L, "stereo_mode_varrier_01",    STEREO_VARRIER_01);
    lua_constant(L, "stereo_mode_varrier_11",    STEREO_VARRIER_11);
    lua_constant(L, "stereo_mode_varrier_33",    STEREO_VARRIER_33);
    lua_constant(L, "stereo_mode_varrier_41",    STEREO_VARRIER_41);

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
        luaopen_socket(L);
        luaopen_electro(L);

        return 1;
    }
    return 0;
}

void free_script(void)
{
    lua_close(L);
}

void load_script(const char *file)
{
    int  err;
    FILE *fp;

    const char *path = get_file_path(file);
    const char *name = get_file_name(file);

    open_path(path);

    /* Load and execute the script. */

    if ((fp = open_file(name, "r")))
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
    else error("Script not found: %s", file);
}

/*---------------------------------------------------------------------------*/
