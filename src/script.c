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
#include "physics.h"
#include "utility.h"
#include "entity.h"
#include "sound.h"
#include "image.h"
#include "brush.h"
#include "font.h"
#include "script.h"

static lua_State *L;

#define METATABLE_KEY "__Edatamt"

/*===========================================================================*/
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

/*===========================================================================*/
/* Generic userdata handlers                                                 */

#define USERDATA_ENTITY 0
#define USERDATA_SOUND  1
#define USERDATA_IMAGE  2
#define USERDATA_BRUSH  3

static int E_tousertype(lua_State *L, int i)
{
    return ((int *) lua_touserdata(L, i))[0];
}

static int E_touserdata(lua_State *L, int i)
{
    return ((int *) lua_touserdata(L, i))[1];
}

static int E_equserdata(lua_State *L)
{
    lua_pushboolean(L, ((E_tousertype(L, -2) == E_tousertype(L, -1)) &&
                        (E_touserdata(L, -2) == E_touserdata(L, -1))));
    return 1;
}

static void E_pushuserdata(lua_State *L, int type, int data)
{
    /* Create and initialize a new userdata object. */

    int *p = (int *) lua_newuserdata(L, 2 * sizeof (int));

    p[0] = type;
    p[1] = data;

    /* Attach the userdata metatable. */

    lua_getglobal(L, METATABLE_KEY);
    lua_setmetatable(L, -2);
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
    if (id <= 0 || entity_type(id) == 0)
        lua_pushnil(L);
    else
        E_pushuserdata(L, USERDATA_ENTITY, id);
}

/*---------------------------------------------------------------------------*/
/* Image userdata handlers                                                   */

static int E_toimage(lua_State *L, int i)
{
    return E_touserdata(L, i);
}

static int E_isimage(lua_State *L, int i)
{
    return ((lua_isuserdata(L, i)) &&
              (E_tousertype(L, i) == USERDATA_IMAGE));
}

static void E_pushimage(lua_State *L, int id)
{
    if (id < 0)
        lua_pushnil(L);
    else
        E_pushuserdata(L, USERDATA_IMAGE, id);
}

/*---------------------------------------------------------------------------*/
/* Brush userdata handlers                                                   */

static int E_tobrush(lua_State *L, int i)
{
    return E_touserdata(L, i);
}

static int E_isbrush(lua_State *L, int i)
{
    return ((lua_isuserdata(L, i)) &&
              (E_tousertype(L, i) == USERDATA_BRUSH));
}

static void E_pushbrush(lua_State *L, int id)
{
    if (id < 0)
        lua_pushnil(L);
    else
        E_pushuserdata(L, USERDATA_BRUSH, id);
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

/*===========================================================================*/
/* Function argument error reporters                                         */

static void E_dump_stack(lua_State *L)
{
    struct lua_Debug ar;
    int i;

    for (i = 1; lua_getstack(L, i, &ar); ++i)
        if (lua_getinfo(L, "Snl", &ar))
        {
            int line = ar.currentline;

            if (ar.name)
                debug("%5d: %s() %s:%d", i, ar.name, ar.source, line);
            else
                debug("%5d: toplevel %s:%d", i, ar.source, line);
        }
}

static void E_type_error(const char *type, lua_State *L, int i)
{
    const char *name = lua_tostring(L, lua_upvalueindex(1));
    const char *got;

    if (lua_isuserdata(L, i))
        switch (E_tousertype(L, i))
        {
        case USERDATA_ENTITY: got = entity_name(E_toentity(L, i)); break;
        case USERDATA_SOUND:  got = "sound";   break;
        case USERDATA_IMAGE:  got = "image";   break;
        case USERDATA_BRUSH:  got = "brush";   break;
        default:              got = "unknown"; break;
        }
    else
        got = lua_typename(L, lua_type(L, i));

    error("'%s' expected %s, got %s", name, type, got);
    E_dump_stack(L);
}

static void E_arity_error(lua_State *L, int i)
{
    const char *name = lua_tostring(L, lua_upvalueindex(1));

    error("'%s' expected %d parameters, got %d", name, -i, lua_gettop(L));
    E_dump_stack(L);
}

/*---------------------------------------------------------------------------*/
/* Type checking functions                                                   */

static int E_isobject(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(E_toentity(L, i)) == TYPE_OBJECT);
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

static int E_iscamera(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(E_toentity(L, i)) == TYPE_CAMERA);
}

static int E_islight(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(E_toentity(L, i)) == TYPE_LIGHT);
}

static int E_isgalaxy(lua_State *L, int i)
{
    return lua_isuserdata(L, i)
        && (entity_type(E_toentity(L, i)) == TYPE_GALAXY);
}

/*---------------------------------------------------------------------------*/
/* Lua function argument type and arity checkers                             */

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

    return 0;
}

/*---------------------------------------------------------------------------*/
/* Electro function argument type and arity checkers                         */

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

static int E_getobject(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (E_isobject(L, i))
            return entity_data(E_toentity(L, i));
        else
            E_type_error("object", L, i);
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

static int E_getimage(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isnil(L, i))
            return 0;

        if (E_isimage(L, i))
            return E_toimage(L, i);
        else
            E_type_error("image", L, i);
    }
    return 0;
}

static int E_getbrush(lua_State *L, int i)
{
    if (1 <= -i && -i <= lua_gettop(L))
    {
        if (lua_isnil(L, i))
            return 0;

        if (E_isbrush(L, i))
            return E_tobrush(L, i);
        else
            E_type_error("brush", L, i);
    }
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

/*===========================================================================*/
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

static int E_set_entity_tracking(lua_State *L)
{
    send_set_entity_tracking(E_getentity (L, -3),
                             L_getinteger(L, -2),
                             L_getinteger(L, -1));
    return 0;
}

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

static int E_set_entity_alpha(lua_State *L)
{
    send_set_entity_alpha(E_getentity(L, -2),
                          L_getnumber(L, -1));
    return 0;
}

static int E_set_entity_flags(lua_State *L)
{
    send_set_entity_flags(E_getentity (L, -3),
                          L_getinteger(L, -2),
                          L_getboolean(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/

static int E_add_entity_force(lua_State *L)
{
    add_entity_force(E_getentity(L, -4),
                     L_getnumber(L, -3),
                     L_getnumber(L, -2),
                     L_getnumber(L, -1));
    return 0;
}

static int E_add_entity_torque(lua_State *L)
{
    add_entity_torque(E_getentity(L, -4),
                      L_getnumber(L, -3),
                      L_getnumber(L, -2),
                      L_getnumber(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Entity relative transform                                                 */

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
/* Entity query                                                              */

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

static int E_get_entity_flags(lua_State *L)
{
    int id    = E_getentity(L, -2);
    int flags = E_getentity(L, -1);
    int value = get_entity_flags(id);

    lua_pushboolean(L, (flags & value));

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

static int E_set_entity_body_type(lua_State *L)
{
    set_entity_body_type(E_getentity (L, -2),
                         L_getboolean(L, -1));
    return 0;
}

static int E_set_entity_body_attr(lua_State *L)
{
    set_entity_body_attr_i(E_getentity (L, -3),
                           L_getinteger(L, -2),
                           L_getinteger(L, -1));
    return 0;
}

static int E_get_entity_body_attr(lua_State *L)
{
    float v[3];

    switch (L_getinteger(L, -1))
    {
    case BODY_ATTR_GRAVITY:
        lua_pushnumber(L, get_entity_body_attr_i(E_getentity (L, -2),
                                                 L_getinteger(L, -1)));
        return 1;

    case BODY_ATTR_CENTER:
        get_entity_body_attr_v(E_getentity (L, -2),
                               L_getinteger(L, -1), v);
        lua_pushnumber(L, v[0]);
        lua_pushnumber(L, v[1]);
        lua_pushnumber(L, v[2]);
        return 3;

    default:
        lua_pushnumber(L, 0);
        return 1;
    }
}

/*---------------------------------------------------------------------------*/

static int E_set_entity_geom_type(lua_State *L)
{
    int n = lua_gettop  (L);
    int i = E_getentity (L, -n + 0);
    int t = L_getinteger(L, -n + 1);

    float v[4] = { 0, 0, 0, 0 };

    switch (t)
    {
    case dSphereClass:
        v[0] = L_getnumber(L, -n + 2);
        break;
    case dCCylinderClass:
        v[0] = L_getnumber(L, -n + 2);
        v[1] = L_getnumber(L, -n + 3);
        break;
    case dBoxClass:
        v[0] = L_getnumber(L, -n + 2);
        v[1] = L_getnumber(L, -n + 3);
        v[2] = L_getnumber(L, -n + 4);
        break;
    case dPlaneClass:
        v[0] = L_getnumber(L, -n + 2);
        v[1] = L_getnumber(L, -n + 3);
        v[2] = L_getnumber(L, -n + 4);
        v[3] = L_getnumber(L, -n + 5);
        break;
    }

    set_entity_geom_type(i, t, v);
    return 0;
}

static int E_set_entity_geom_attr(lua_State *L)
{
    int n = lua_gettop  (L);
    int i = E_getentity (L, -n + 0);
    int p = L_getinteger(L, -n + 1);

    switch (p)
    {
    case GEOM_ATTR_CATEGORY:
    case GEOM_ATTR_COLLIDER:
    case GEOM_ATTR_RESPONSE:
    case GEOM_ATTR_CALLBACK:
        set_entity_geom_attr_i(i, p, L_getinteger(L, -n + 2));
        break;

    case GEOM_ATTR_MASS:
    case GEOM_ATTR_BOUNCE:
    case GEOM_ATTR_FRICTION:
    case GEOM_ATTR_SOFT_ERP:
    case GEOM_ATTR_SOFT_CFM:
        set_entity_geom_attr_f(i, p, L_getnumber(L, -n + 2));
        break;
    }

    return 0;
}

static int E_get_entity_geom_attr(lua_State *L)
{
    switch (L_getinteger(L, -1))
    {
    case GEOM_ATTR_CATEGORY:
    case GEOM_ATTR_COLLIDER:
    case GEOM_ATTR_RESPONSE:
    case GEOM_ATTR_CALLBACK:
        lua_pushnumber(L, get_entity_geom_attr_i(E_getentity (L, -2),
                                                 L_getinteger(L, -1)));
        break;

    case GEOM_ATTR_MASS:
    case GEOM_ATTR_BOUNCE:
    case GEOM_ATTR_FRICTION:
    case GEOM_ATTR_SOFT_ERP:
    case GEOM_ATTR_SOFT_CFM:
        lua_pushnumber(L, get_entity_geom_attr_f(E_getentity (L, -2),
                                                 L_getinteger(L, -1)));
        break;

    default:
        lua_pushnumber(L, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int E_set_entity_joint_type(lua_State *L)
{
    set_entity_join_type(E_getentity (L, -3),
                         E_getentity (L, -2),
                         L_getinteger(L, -1));
    return 0;
}

static int E_set_entity_joint_attr(lua_State *L)
{
    int n = lua_gettop  (L);
    int i = E_getentity (L, -n + 0);
    int j = E_getentity (L, -n + 1);
    int p = L_getinteger(L, -n + 2);

    float v[3] = { 0, 0, 0 };

    switch (p)
    {
    case JOINT_ATTR_ANCHOR:
    case JOINT_ATTR_AXIS_1:
    case JOINT_ATTR_AXIS_2:
        v[0] = L_getnumber(L, -n + 3);
        v[1] = L_getnumber(L, -n + 4);
        v[2] = L_getnumber(L, -n + 5);
        set_entity_join_attr_v(i, j, p, v);
        break;

    default:
        set_entity_join_attr_f(i, j, p, L_getnumber(L, -n + 3));
        break;
    }

    return 0;
}

static int E_get_entity_joint_attr(lua_State *L)
{
    int n = lua_gettop  (L);
    int i = E_getentity (L, -n + 0);
    int j = E_getentity (L, -n + 1);
    int p = L_getinteger(L, -n + 2);

    float v[3] = { 0, 0, 0 };

    switch (p)
    {
    case JOINT_ATTR_ANCHOR:
    case JOINT_ATTR_AXIS_1:
    case JOINT_ATTR_AXIS_2:
        get_entity_join_attr_v(i, j, p, v);
        lua_pushnumber(L, v[0]);
        lua_pushnumber(L, v[1]);
        lua_pushnumber(L, v[2]);
        return 3;

    default:
        lua_pushnumber(L, get_entity_join_attr_f(i, j, p));
        return 1;
    }
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
    int id = send_create_sprite(E_getbrush(L, -1));

    E_pushentity(L, id);
    return 1;
}

static int E_create_object(lua_State *L)
{
    int id;

    if (lua_gettop(L) == 1)
        id = send_create_object(L_getstring(L, -1));
    else
        id = send_create_object(NULL);

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
    int id = send_create_galaxy(L_getstring(L, -2),
                                E_getbrush (L, -1));

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
/* Object functions                                                          */

static int E_create_mesh(lua_State *L)
{
    int N = lua_gettop(L);
    int i = E_getobject(L, -N);
    int j = send_create_mesh(i);

    if (N > 1)
        send_set_mesh(i, j, E_getbrush(L, -N + 1));

    lua_pushnumber(L, j);
    return 1;
}

static int E_create_vert(lua_State *L)
{
    int N = lua_gettop (L);
    int i = E_getobject(L, -N);
    int j = send_create_vert(i);

    float v[3];
    float n[3];
    float t[2];

    get_vert(i, j, v, n, t);

    v[0] = L_getnumber(L, -N + 1);
    v[1] = L_getnumber(L, -N + 2);
    v[2] = L_getnumber(L, -N + 3);

    if (N >= 7)
    {
        n[0] = L_getnumber(L, -N + 4);
        n[1] = L_getnumber(L, -N + 5);
        n[2] = L_getnumber(L, -N + 6);
    }
    if (N >= 9)
    {
        t[0] = L_getnumber(L, -N + 7);
        t[1] = L_getnumber(L, -N + 8);
    }
    if (N >= 2)
        send_set_vert(i, j, v, n, t);

    lua_pushnumber(L, j);
    return 1;
}

static int E_create_face(lua_State *L)
{
    int i = E_getobject (L, -5);
    int j = L_getinteger(L, -4);
    int k = send_create_face(i, j);

    int vi[3];

    vi[0] = L_getinteger(L, -3);
    vi[1] = L_getinteger(L, -2);
    vi[2] = L_getinteger(L, -1);

    send_set_face(i, j, k, vi);

    lua_pushnumber(L, k);
    return 1;
}

static int E_create_edge(lua_State *L)
{
    int i = E_getobject (L, -4);
    int j = L_getinteger(L, -3);
    int k = send_create_edge(i, j);

    int vi[2];

    vi[0] = L_getinteger(L, -2);
    vi[1] = L_getinteger(L, -1);

    send_set_edge(i, j, k, vi);

    lua_pushnumber(L, k);
    return 1;
}

/* Object modifiers                                                          */

static int E_set_mesh(lua_State *L)
{
    send_set_mesh(E_getobject (L, -3),
                  L_getinteger(L, -2),
                  E_getbrush  (L, -1));
    return 0;
}

static int E_set_vert(lua_State *L)
{
    int N = lua_gettop  (L);
    int i = E_getobject (L, -N);
    int j = L_getinteger(L, -N + 1);

    float v[3];
    float n[3];
    float t[2];

    get_vert(i, j, v, n, t);

    if (N >= 5)
    {
        v[0] = L_getnumber(L, -N + 2);
        v[1] = L_getnumber(L, -N + 3);
        v[2] = L_getnumber(L, -N + 4);
    }
    if (N >= 8)
    {
        n[0] = L_getnumber(L, -N + 5);
        n[1] = L_getnumber(L, -N + 6);
        n[2] = L_getnumber(L, -N + 7);
    }
    if (N >= 10)
    {
        t[0] = L_getnumber(L, -N + 8);
        t[1] = L_getnumber(L, -N + 9);
    }

    send_set_vert(i, j, v, n, t);
    return 0;
}

static int E_set_face(lua_State *L)
{
    int vi[3];

    vi[0] = L_getinteger(L, -3);
    vi[1] = L_getinteger(L, -2);
    vi[2] = L_getinteger(L, -1);

    send_set_face(E_getobject (L, -6),
                  L_getinteger(L, -5),
                  L_getinteger(L, -4), vi);
    return 0;
}

static int E_set_edge(lua_State *L)
{
    int vi[2];

    vi[0] = L_getinteger(L, -2);
    vi[1] = L_getinteger(L, -1);

    send_set_face(E_getobject (L, -5),
                  L_getinteger(L, -4),
                  L_getinteger(L, -3), vi);
    return 0;
}

/* Object query                                                              */

static int E_get_mesh(lua_State *L)
{
    E_pushbrush(L, get_mesh(E_getobject(L, -2), L_getinteger(L, -1)));
    return 1;
}

static int E_get_vert(lua_State *L)
{
    float v[3] = { 0, 0, 0 };
    float n[3] = { 0, 0, 1 };
    float t[2] = { 0, 0    };

    get_vert(E_getobject(L, -2), L_getinteger(L, -1), v, n, t);

    lua_pushnumber(L, v[0]);
    lua_pushnumber(L, v[1]);
    lua_pushnumber(L, v[2]);
    lua_pushnumber(L, n[0]);
    lua_pushnumber(L, n[1]);
    lua_pushnumber(L, n[2]);
    lua_pushnumber(L, t[0]);
    lua_pushnumber(L, t[1]);

    return 8;
}

static int E_get_face(lua_State *L)
{
    int vi[3] = { -1, -1, -1 };

    get_face(E_getobject (L, -3),
             L_getinteger(L, -2),
             L_getinteger(L, -1), vi);

    lua_pushnumber(L, vi[0]);
    lua_pushnumber(L, vi[1]);
    lua_pushnumber(L, vi[2]);

    return 3;
}

static int E_get_edge(lua_State *L)
{
    int vi[2] = { -1, -1 };

    get_edge(E_getobject (L, -3),
             L_getinteger(L, -2),
             L_getinteger(L, -1), vi);

    lua_pushnumber(L, vi[0]);
    lua_pushnumber(L, vi[1]);

    return 2;
}

/* Object counters                                                           */

static int E_get_mesh_count(lua_State *L)
{
    lua_pushnumber(L, get_mesh_count(E_getobject(L, -1)));
    return 1;
}

static int E_get_vert_count(lua_State *L)
{
    lua_pushnumber(L, get_vert_count(E_getobject(L, -1)));
    return 1;
}

static int E_get_face_count(lua_State *L)
{
    lua_pushnumber(L, get_face_count(E_getobject (L, -2),
                                     L_getinteger(L, -1)));
    return 1;
}

static int E_get_edge_count(lua_State *L)
{
    lua_pushnumber(L, get_edge_count(E_getobject (L, -2),
                                     L_getinteger(L, -1)));
    return 0;
}

/* Object destructors                                                        */

static int E_delete_mesh(lua_State *L)
{
    send_delete_mesh(E_getobject (L, -2),
                     L_getinteger(L, -1));
    return 0;
}

static int E_delete_vert(lua_State *L)
{
    send_delete_vert(E_getobject (L, -2),
                     L_getinteger(L, -1));
    return 0;
}

static int E_delete_face(lua_State *L)
{
    send_delete_face(E_getobject (L, -3),
                     L_getinteger(L, -2),
                     L_getinteger(L, -1));
    return 0;
}

static int E_delete_edge(lua_State *L)
{
    send_delete_edge(E_getobject (L, -3),
                     L_getinteger(L, -2),
                     L_getinteger(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Sprite functions                                                          */

static int E_set_sprite_brush(lua_State *L)
{
    send_set_sprite_brush(E_getsprite(L, -2),
                          E_getbrush (L, -1));
    return 0;
}

static int E_set_sprite_range(lua_State *L)
{
    send_set_sprite_range(E_getsprite(L, -5),
                          L_getnumber(L, -4),
                          L_getnumber(L, -3),
                          L_getnumber(L, -2),
                          L_getnumber(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* String functions                                                          */

static int E_set_string_fill(lua_State *L)
{
    send_set_string_fill(E_getstring(L, -2),
                         E_getbrush (L, -1));
    return 0;
}

static int E_set_string_line(lua_State *L)
{
    send_set_string_line(E_getstring(L, -2),
                         E_getbrush (L, -1));
    return 0;
}

static int E_set_string_text(lua_State *L)
{
    send_set_string_text(E_getstring(L, -2),
                         L_getstring(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Camera functions                                                          */

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

static int E_set_camera_range(lua_State *L)
{
    send_set_camera_range(E_getcamera(L, -3),
                          L_getnumber(L, -2),
                          L_getnumber(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Light functions                                                           */

static int E_set_light_color(lua_State *L)
{
    send_set_light_color(E_getlight (L, -4),
                         L_getnumber(L, -3),
                         L_getnumber(L, -2),
                         L_getnumber(L, -1));
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Galaxy functions                                                          */

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

/*===========================================================================*/
/* Image functions                                                           */

static int E_create_image(lua_State *L)
{
    int N = lua_gettop(L);

    if (N == 6)
        E_pushimage(L, send_create_image(L_getstring(L, -6),
                                         L_getstring(L, -5),
                                         L_getstring(L, -4),
                                         L_getstring(L, -3),
                                         L_getstring(L, -2),
                                         L_getstring(L, -1)));
    else
    {
#ifdef VIDEOTEX
        if (lua_isnumber(L, -1))
            E_pushimage(L, send_create_video(L_getinteger(L, -1)));
        else
#endif
            E_pushimage(L, send_create_image(L_getstring(L, -1),
                                             NULL, NULL, NULL, NULL, NULL));
    }
 
    return 1;
}

static int E_delete_image(lua_State *L)
{
    send_delete_image(E_getimage(L, -1));
    return 0;
}

static int E_get_image_pixel(lua_State *L)
{
    unsigned char c[4];

    get_image_c(E_getimage  (L, -3),
                L_getinteger(L, -2),
                L_getinteger(L, -1), c);

    lua_pushnumber(L, (double) c[0] / 255.0);
    lua_pushnumber(L, (double) c[1] / 255.0);
    lua_pushnumber(L, (double) c[2] / 255.0);
    lua_pushnumber(L, (double) c[3] / 255.0);

    return 4;
}

static int E_get_image_size(lua_State *L)
{
    int id = E_getimage(L, -1);

    lua_pushnumber(L, (double) get_image_w(id));
    lua_pushnumber(L, (double) get_image_h(id));

    return 2;
}

/*===========================================================================*/
/* Brush functions                                                           */

static int E_create_brush(lua_State *L)
{
    E_pushbrush(L, send_create_brush(NULL, NULL));
    return 1;
}

static int E_delete_brush(lua_State *L)
{
    send_delete_brush(E_getbrush(L, -1));
    return 0;
}

static int E_set_brush_flags(lua_State *L)
{
    send_set_brush_flags(E_getbrush  (L, -3),
                         L_getinteger(L, -2),
                         L_getboolean(L, -1));
    return 0;
}

static int E_set_brush_image(lua_State *L)
{
    if (lua_gettop(L) >= 3)
        send_set_brush_image(E_getbrush  (L, -3),
                             E_getimage  (L, -2),
                             L_getinteger(L, -1));
    else
        send_set_brush_image(E_getbrush  (L, -2),
                             E_getimage  (L, -1), 0);
    return 0;
}

static int E_set_brush_color(lua_State *L)
{
    float d[4], s[4], a[4], x[1];

    int N = lua_gettop(L);
    int f = 0;

    d[0] = L_getnumber(L, -N + 1);
    d[1] = L_getnumber(L, -N + 2);
    d[2] = L_getnumber(L, -N + 3);
    d[3] = L_getnumber(L, -N + 4);
    f   |= BRUSH_DIFFUSE;

    if (N >= 9)
    {
        s[0] = L_getnumber(L, -N + 5);
        s[1] = L_getnumber(L, -N + 6);
        s[2] = L_getnumber(L, -N + 7);
        s[3] = L_getnumber(L, -N + 8);
        f   |= BRUSH_SPECULAR;
    }
    if (N >= 13)
    {
        a[0] = L_getnumber(L, -N +  9);
        a[1] = L_getnumber(L, -N + 10);
        a[2] = L_getnumber(L, -N + 11);
        a[3] = L_getnumber(L, -N + 12);
        f   |= BRUSH_AMBIENT;
    }

    if (N >= 14)
    {
        x[0] = L_getnumber(L, -N + 13);
        f   |= BRUSH_SHINY;
    }

    send_set_brush_color(E_getbrush(L, -N), d, s, a, x, f);
    return 0;
}

static int E_set_brush_frag_prog(lua_State *L)
{
    int id           = E_getbrush (L, -2);
    const char *file = L_getstring(L, -1);

    char *text = load_file(file, "r", NULL);

    send_set_brush_frag_prog(id, text);
    if (text) free(text);

    return 0;
}

static int E_set_brush_vert_prog(lua_State *L)
{
    int id           = E_getbrush (L, -2);
    const char *file = L_getstring(L, -1);

    char *text = load_file(file, "r", NULL);

    send_set_brush_vert_prog(id, text);
    if (text) free(text);

    return 0;
}

static int E_set_brush_frag_param(lua_State *L)
{
    int   N = lua_gettop(L);
    float v[4];

    v[0] = (N >= 3) ? L_getnumber(L, -N + 2) : 0;
    v[1] = (N >= 4) ? L_getnumber(L, -N + 3) : 0;
    v[2] = (N >= 5) ? L_getnumber(L, -N + 4) : 0;
    v[3] = (N >= 6) ? L_getnumber(L, -N + 5) : 0;

    send_set_brush_frag_param(E_getbrush  (L, -N + 0),
                              L_getinteger(L, -N + 1), v);
    return 0;
}

static int E_set_brush_vert_param(lua_State *L)
{
    int   N = lua_gettop(L);
    float v[4];

    v[0] = (N >= 3) ? L_getnumber(L, -N + 2) : 0;
    v[1] = (N >= 4) ? L_getnumber(L, -N + 3) : 0;
    v[2] = (N >= 5) ? L_getnumber(L, -N + 4) : 0;
    v[3] = (N >= 6) ? L_getnumber(L, -N + 5) : 0;

    send_set_brush_vert_param(E_getbrush  (L, -N + 0),
                              L_getinteger(L, -N + 1), v);
    return 0;
}

static int E_set_brush_line_width(lua_State *L)
{
    send_set_brush_line_width(E_getbrush (L, -2),
                              L_getnumber(L, -1));
    return 0;
}

/*===========================================================================*/
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

/*===========================================================================*/
/* Console functions                                                         */

static int E_clear_console(lua_State *L)
{
    clear_console();
    return 0;
}

static int E_close_console(lua_State *L)
{
    set_console_enable(0);
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
    int i, N = lua_gettop(L);
    const char *str;

    for (i = N; i > 0; --i)
        if ((str = L_getstring(L, -i)))
            print_console(str);

    return 1;
}

/*===========================================================================*/
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

static int E_set_host_flags(lua_State *L)
{
    send_set_host_flags(L_getinteger(L, -3),
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

static int E_set_tile_flags(lua_State *L)
{
    send_set_tile_flags(L_getinteger(L, -3),
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

/*===========================================================================*/
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
    float o = 0.000f;

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
    int N = lua_gettop(L);
    float c0[3];
    float c1[3];

    if (N == 6)
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

/*===========================================================================*/
/* System functions                                                          */

static int E_exit(lua_State *L)
{
    SDL_Event e;

    e.type = SDL_QUIT;
    SDL_PushEvent(&e);

    return 0;
}

static int E_nuke(lua_State *L)
{
    enable_timer(0);

    nuke_entities();
    nuke_brushes();
    nuke_images();
    nuke_sounds();

    return 0;
}

static int E_pushdir(lua_State *L)
{
    path_push(L_getstring(L, -1));
    return 0;
}

static int E_popdir(lua_State *L)
{
    path_pop();
    return 0;
}

static int E_chdir(lua_State *L)
{
    chdir(L_getstring(L, -1));
    return 0;
}

/*===========================================================================*/
/* Script callback backcallers                                               */

static int lua_callassert(lua_State *L, int nin, int nout, const char *name)
{
    int r = 0;

    if (lua_pcall(L, nin, nout, 0) == LUA_ERRRUN)
        error("%s: %s", name, lua_tostring(L, -1));
    else
        r = lua_toboolean(L, -1);

    lua_pop(L, 1);

    opengl_check(name);

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

int do_timer_script(float dt)
{
    const char *name = "do_timer";

    lua_getglobal(L, name);

    if (lua_isfunction(L, -1))
    {
        lua_pushnumber(L, dt);

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

int do_contact_script(int i, int j, const float p[3],
                                    const float n[3], float d)
{
    const char *name = "do_contact";

    lua_getglobal(L, name);

    if (lua_isfunction(L, -1))
    {
        E_pushentity(L, i);
        E_pushentity(L, j);

        lua_pushnumber (L, (lua_Number) p[0]);
        lua_pushnumber (L, (lua_Number) p[1]);
        lua_pushnumber (L, (lua_Number) p[2]);
        lua_pushnumber (L, (lua_Number) n[0]);
        lua_pushnumber (L, (lua_Number) n[1]);
        lua_pushnumber (L, (lua_Number) n[2]);
        lua_pushnumber (L, (lua_Number) d);

        return lua_callassert(L, 9, 1, name);
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

/*===========================================================================*/
/* Script setup/shutdown                                                     */

static void lua_function(lua_State *L, const char *name, lua_CFunction func)
{
    lua_pushstring(L, name);
    lua_pushstring(L, name);
    lua_pushcclosure(L, func, 1);
    lua_settable(L, -3);
}

static void lua_constant(lua_State *L, const char *name, int value)
{
    lua_pushstring(L, name);
    lua_pushnumber(L, value);
    lua_settable(L, -3);
}

/*---------------------------------------------------------------------------*/

struct function_def
{
    const char   *name;
    lua_CFunction func;
};

struct constant_def
{
    const char *name;
    int        value;
};

static struct function_def functions[] = {

    /* Entity contructors and destructors */

    { "create_camera",         E_create_camera         },
    { "create_sprite",         E_create_sprite         },
    { "create_object",         E_create_object         },
    { "create_string",         E_create_string         },
    { "create_galaxy",         E_create_galaxy         },
    { "create_light",          E_create_light          },
    { "create_pivot",          E_create_pivot          },
    { "create_clone",          E_create_clone          },

    /* Entity functions */

    { "parent_entity",         E_parent_entity         },
    { "delete_entity",         E_delete_entity         },

    { "get_entity_parent",     E_get_entity_parent     },
    { "get_entity_child",      E_get_entity_child      },

    { "set_entity_tracking",   E_set_entity_tracking   },
    { "set_entity_position",   E_set_entity_position   },
    { "set_entity_rotation",   E_set_entity_rotation   },
    { "set_entity_scale",      E_set_entity_scale      },
    { "set_entity_alpha",      E_set_entity_alpha      },
    { "set_entity_flags",      E_set_entity_flags      },

    { "add_entity_force",      E_add_entity_force      },
    { "add_entity_torque",     E_add_entity_torque     },

    { "get_entity_position",   E_get_entity_position   },
    { "get_entity_x_vector",   E_get_entity_x_vector   },
    { "get_entity_y_vector",   E_get_entity_y_vector   },
    { "get_entity_z_vector",   E_get_entity_z_vector   },
    { "get_entity_scale",      E_get_entity_scale      },
    { "get_entity_bound",      E_get_entity_bound      },
    { "get_entity_alpha",      E_get_entity_alpha      },
    { "get_entity_flags",      E_get_entity_flags      },

    { "move_entity",           E_move_entity           },
    { "turn_entity",           E_turn_entity           },

    { "set_entity_body_type",  E_set_entity_body_type  },
    { "set_entity_geom_type",  E_set_entity_geom_type  },
    { "set_entity_joint_type", E_set_entity_joint_type },

    { "set_entity_body_attr",  E_set_entity_body_attr  },
    { "set_entity_geom_attr",  E_set_entity_geom_attr  },
    { "set_entity_joint_attr", E_set_entity_joint_attr },
    { "get_entity_body_attr",  E_get_entity_body_attr  },
    { "get_entity_geom_attr",  E_get_entity_geom_attr  },
    { "get_entity_joint_attr", E_get_entity_joint_attr },

    /* Object functions */

    { "create_mesh",           E_create_mesh           },
    { "create_vert",           E_create_vert           },
    { "create_face",           E_create_face           },
    { "create_edge",           E_create_edge           },

    { "set_mesh",              E_set_mesh              },
    { "set_vert",              E_set_vert              },
    { "set_face",              E_set_face              },
    { "set_edge",              E_set_edge              },

    { "get_mesh",              E_get_mesh              },
    { "get_vert",              E_get_vert              },
    { "get_face",              E_get_face              },
    { "get_edge",              E_get_edge              },

    { "get_mesh_count",        E_get_mesh_count        },
    { "get_vert_count",        E_get_vert_count        },
    { "get_face_count",        E_get_face_count        },
    { "get_edge_count",        E_get_edge_count        },

    { "delete_mesh",           E_delete_mesh           },
    { "delete_vert",           E_delete_vert           },
    { "delete_face",           E_delete_face           },
    { "delete_edge",           E_delete_edge           },

    /* Sprite functions */

    { "set_sprite_brush",      E_set_sprite_brush      },
    { "set_sprite_range",      E_set_sprite_range      },

    /* String functions */

    { "set_string_fill",       E_set_string_fill       },
    { "set_string_line",       E_set_string_line       },
    { "set_string_text",       E_set_string_text       },

    /* Camera functions */

    { "set_camera_offset",     E_set_camera_offset     },
    { "set_camera_stereo",     E_set_camera_stereo     },
    { "set_camera_range",      E_set_camera_range      },

    /* Light functions */

    { "set_light_color",       E_set_light_color       },

    /* Galaxy functions */

    { "set_galaxy_magnitude",  E_set_galaxy_magnitude  },
    { "get_star_index",        E_get_star_index        },
    { "get_star_position",     E_get_star_position     },

    /* Image functions */

    { "create_image",          E_create_image          },
    { "delete_image",          E_delete_image          },
    { "get_image_pixel",       E_get_image_pixel       },
    { "get_image_size",        E_get_image_size        },

    /* Brush functions */

    { "create_brush",          E_create_brush          },
    { "delete_brush",          E_delete_brush          },
    { "set_brush_flags",       E_set_brush_flags       },
    { "set_brush_image",       E_set_brush_image       },
    { "set_brush_color",       E_set_brush_color       },
    { "set_brush_frag_prog",   E_set_brush_frag_prog   },
    { "set_brush_vert_prog",   E_set_brush_vert_prog   },
    { "set_brush_frag_param",  E_set_brush_frag_param  },
    { "set_brush_vert_param",  E_set_brush_vert_param  },
    { "set_brush_line_width",  E_set_brush_line_width  },

    /* Sound functions */

    { "load_sound",            E_load_sound            },
    { "free_sound",            E_free_sound            },
    { "stop_sound",            E_stop_sound            },
    { "play_sound",            E_play_sound            },
    { "loop_sound",            E_loop_sound            },

    /* Console functions */

    { "clear_console",         E_clear_console         },
    { "close_console",         E_close_console         },
    { "color_console",         E_color_console         },
    { "print_console",         E_print_console         },

    /* Configuration */

    { "add_host",              E_add_host              },
    { "add_tile",              E_add_tile              },
    { "set_host_flags",        E_set_host_flags        },
    { "set_tile_flags",        E_set_tile_flags        },
    { "set_tile_position",     E_set_tile_position     },
    { "set_tile_viewport",     E_set_tile_viewport     },
    { "set_tile_line_screen",  E_set_tile_line_screen  },
    { "set_tile_view_mirror",  E_set_tile_view_mirror  },
    { "set_tile_view_offset",  E_set_tile_view_offset  },
    { "get_display_union",     E_get_display_union     },
    { "get_display_bound",     E_get_display_bound     },

    /* Miscellaneous */

    { "enable_timer",          E_enable_timer          },
    { "get_joystick",          E_get_joystick          },
    { "get_modifier",          E_get_modifier          },
    { "set_typeface",          E_set_typeface          },
    { "set_background",        E_set_background        },
    { "exit",                  E_exit                  },
    { "nuke",                  E_nuke                  },
    { "chdir",                 E_chdir                 },
    { "popdir",                E_popdir                 },
    { "pushdir",               E_pushdir                 },
};

static struct constant_def constants[] = {

    /* Entity constants */

    { "entity_flag_hidden",        FLAG_HIDDEN         },
    { "entity_flag_wireframe",     FLAG_WIREFRAME      },
    { "entity_flag_billboard",     FLAG_BILLBOARD      },
    { "entity_flag_line_smooth",   FLAG_LINE_SMOOTH    },
    { "entity_flag_left_eye",      FLAG_LEFT_EYE       },
    { "entity_flag_right_eye",     FLAG_RIGHT_EYE      },
    { "entity_flag_track_pos",     FLAG_TRACK_POS      },
    { "entity_flag_track_rot",     FLAG_TRACK_ROT      },

    { "tracking_mode_local",       TRACK_LOCAL         },
    { "tracking_mode_world",       TRACK_WORLD         },

    /* Body constants */

    { "body_attr_gravity",         BODY_ATTR_GRAVITY   },

    /* Geom constants */

    { "geom_type_none",            -1                  },
    { "geom_type_box",             dBoxClass           },
    { "geom_type_plane",           dPlaneClass         },
    { "geom_type_sphere",          dSphereClass        },
    { "geom_type_capsule",         dCCylinderClass     },

    { "geom_attr_category",        GEOM_ATTR_CATEGORY  },
    { "geom_attr_collider",        GEOM_ATTR_COLLIDER  },
    { "geom_attr_response",        GEOM_ATTR_RESPONSE  },
    { "geom_attr_callback",        GEOM_ATTR_CALLBACK  },
    { "geom_attr_mass",            GEOM_ATTR_MASS      },
    { "geom_attr_bounce",          GEOM_ATTR_BOUNCE    },
    { "geom_attr_friction",        GEOM_ATTR_FRICTION  },
    { "geom_attr_soft_erp",        GEOM_ATTR_SOFT_ERP  },
    { "geom_attr_soft_cfm",        GEOM_ATTR_SOFT_CFM  },

    /* Joint constants */

    { "joint_type_ball",           dJointTypeBall      },
    { "joint_type_hinge",          dJointTypeHinge     },
    { "joint_type_slider",         dJointTypeSlider    },
    { "joint_type_universal",      dJointTypeUniversal },
    { "joint_type_hinge_2",        dJointTypeHinge2    },

    { "joint_attr_anchor",         JOINT_ATTR_ANCHOR   },
    { "joint_attr_axis_1",         JOINT_ATTR_AXIS_1   },
    { "joint_attr_axis_2",         JOINT_ATTR_AXIS_2   },
    { "joint_attr_value",          JOINT_ATTR_VALUE    },
    { "joint_attr_rate_1",         JOINT_ATTR_RATE_1   },
    { "joint_attr_rate_2",         JOINT_ATTR_RATE_2   },
    { "joint_attr_lo_stop",        dParamLoStop        },
    { "joint_attr_lo_stop_2",      dParamLoStop2       },
    { "joint_attr_hi_stop",        dParamHiStop        },
    { "joint_attr_hi_stop_2",      dParamHiStop2       },
    { "joint_attr_velocity",       dParamVel           },
    { "joint_attr_velocity_2",     dParamVel2          },
    { "joint_attr_force_max",      dParamFMax          },
    { "joint_attr_force_max_2",    dParamFMax2         },
    { "joint_attr_bounce",         dParamBounce        },
    { "joint_attr_bounce_2",       dParamBounce2       },
    { "joint_attr_cfm",            dParamCFM           },
    { "joint_attr_cfm_2",          dParamCFM2          },
    { "joint_attr_stop_erp",       dParamStopERP       },
    { "joint_attr_stop_erp_2",     dParamStopERP2      },
    { "joint_attr_stop_cfm",       dParamStopCFM       },
    { "joint_attr_stop_cfm_2",     dParamStopCFM2      },
    { "joint_attr_susp_erp",       dParamSuspensionERP },
    { "joint_attr_susp_cfm",       dParamSuspensionCFM },

    /* Brush constants */

    { "brush_flag_diffuse",        BRUSH_DIFFUSE       },
    { "brush_flag_specular",       BRUSH_SPECULAR      },
    { "brush_flag_ambient",        BRUSH_AMBIENT       },
    { "brush_flag_shiny",          BRUSH_SHINY         },
    { "brush_flag_transparent",    BRUSH_TRANSPARENT   },
    { "brush_flag_env_map_0",      BRUSH_ENV_MAP_0     },
    { "brush_flag_env_map_1",      BRUSH_ENV_MAP_1     },
    { "brush_flag_env_map_2",      BRUSH_ENV_MAP_2     },
    { "brush_flag_env_map_3",      BRUSH_ENV_MAP_3     },
    { "brush_flag_sky_map_0",      BRUSH_SKY_MAP_0     },
    { "brush_flag_sky_map_1",      BRUSH_SKY_MAP_1     },
    { "brush_flag_sky_map_2",      BRUSH_SKY_MAP_2     },
    { "brush_flag_sky_map_3",      BRUSH_SKY_MAP_3     },
    { "brush_flag_unlit",          BRUSH_UNLIT         },

    /* Configuration constants */

    { "host_flag_full",            HOST_FULL           },
    { "host_flag_stereo",          HOST_STEREO         },
    { "host_flag_framed",          HOST_FRAMED         },

    { "tile_flag_flip_x",          TILE_FLIP_X         },
    { "tile_flag_flip_y",          TILE_FLIP_Y         },
    { "tile_flag_offset",          TILE_OFFSET         },
    { "tile_flag_mirror",          TILE_MIRROR         },
    { "tile_flag_test",            TILE_TEST           },

    /* Camera constants */

    { "camera_type_orthogonal",    CAMERA_ORTHO        },
    { "camera_type_perspective",   CAMERA_PERSP        },

    { "stereo_mode_none",          STEREO_NONE         },
    { "stereo_mode_quad",          STEREO_QUAD         },
    { "stereo_mode_red_blue",      STEREO_RED_BLUE     },
    { "stereo_mode_varrier_01",    STEREO_VARRIER_01   },
    { "stereo_mode_varrier_11",    STEREO_VARRIER_11   },
    { "stereo_mode_varrier_33",    STEREO_VARRIER_33   },
    { "stereo_mode_varrier_41",    STEREO_VARRIER_41   },

    /* Light constants */

    { "light_type_positional",     LIGHT_POSITIONAL    },
    { "light_type_directional",    LIGHT_DIRECTIONAL   },

    /* Miscellaneous constants */

    { "key_modifier_shift",        KMOD_SHIFT          },
    { "key_modifier_control",      KMOD_CTRL           },
    { "key_modifier_alt",          KMOD_ALT            },
};

struct constant_def keys[] = {

    { "key_unknown",      SDLK_UNKNOWN      },
    { "key_backspace",    SDLK_BACKSPACE    },
    { "key_tab",          SDLK_TAB          },
    { "key_clear",        SDLK_CLEAR        },
    { "key_return",       SDLK_RETURN       },
    { "key_pause",        SDLK_PAUSE        },
    { "key_escape",       SDLK_ESCAPE       },
    { "key_space",        SDLK_SPACE        },
    { "key_exclaim",      SDLK_EXCLAIM      },
    { "key_quotedbl",     SDLK_QUOTEDBL     },
    { "key_hash",         SDLK_HASH         },
    { "key_dollar",       SDLK_DOLLAR       },
    { "key_ampersand",    SDLK_AMPERSAND    },
    { "key_quote",        SDLK_QUOTE        },
    { "key_leftparen",    SDLK_LEFTPAREN    },
    { "key_rightparen",   SDLK_RIGHTPAREN   },
    { "key_asterisk",     SDLK_ASTERISK     },
    { "key_plus",         SDLK_PLUS         },
    { "key_comma",        SDLK_COMMA        },
    { "key_minus",        SDLK_MINUS        },
    { "key_period",       SDLK_PERIOD       },
    { "key_slash",        SDLK_SLASH        },
    { "key_0",            SDLK_0            },
    { "key_1",            SDLK_1            },
    { "key_2",            SDLK_2            },
    { "key_3",            SDLK_3            },
    { "key_4",            SDLK_4            },
    { "key_5",            SDLK_5            },
    { "key_6",            SDLK_6            },
    { "key_7",            SDLK_7            },
    { "key_8",            SDLK_8            },
    { "key_9",            SDLK_9            },
    { "key_colon",        SDLK_COLON        },
    { "key_semicolon",    SDLK_SEMICOLON    },
    { "key_less",         SDLK_LESS         },
    { "key_equals",       SDLK_EQUALS       },
    { "key_greater",      SDLK_GREATER      },
    { "key_question",     SDLK_QUESTION     },
    { "key_at",           SDLK_AT           },
    { "key_leftbracket",  SDLK_LEFTBRACKET  },
    { "key_backslash",    SDLK_BACKSLASH    },
    { "key_rightbracket", SDLK_RIGHTBRACKET },
    { "key_caret",        SDLK_CARET        },
    { "key_underscore",   SDLK_UNDERSCORE   },
    { "key_backquote",    SDLK_BACKQUOTE    },
    { "key_a",            SDLK_a            },
    { "key_b",            SDLK_b            },
    { "key_c",            SDLK_c            },
    { "key_d",            SDLK_d            },
    { "key_e",            SDLK_e            },
    { "key_f",            SDLK_f            },
    { "key_g",            SDLK_g            },
    { "key_h",            SDLK_h            },
    { "key_i",            SDLK_i            },
    { "key_j",            SDLK_j            },
    { "key_k",            SDLK_k            },
    { "key_l",            SDLK_l            },
    { "key_m",            SDLK_m            },
    { "key_n",            SDLK_n            },
    { "key_o",            SDLK_o            },
    { "key_p",            SDLK_p            },
    { "key_q",            SDLK_q            },
    { "key_r",            SDLK_r            },
    { "key_s",            SDLK_s            },
    { "key_t",            SDLK_t            },
    { "key_u",            SDLK_u            },
    { "key_v",            SDLK_v            },
    { "key_w",            SDLK_w            },
    { "key_x",            SDLK_x            },
    { "key_y",            SDLK_y            },
    { "key_z",            SDLK_z            },
    { "key_delete",       SDLK_DELETE       },
    { "key_kp0",          SDLK_KP0          },
    { "key_kp1",          SDLK_KP1          },
    { "key_kp2",          SDLK_KP2          },
    { "key_kp3",          SDLK_KP3          },
    { "key_kp4",          SDLK_KP4          },
    { "key_kp5",          SDLK_KP5          },
    { "key_kp6",          SDLK_KP6          },
    { "key_kp7",          SDLK_KP7          },
    { "key_kp8",          SDLK_KP8          },
    { "key_kp9",          SDLK_KP9          },
    { "key_kp_period",    SDLK_KP_PERIOD    },
    { "key_kp_divide",    SDLK_KP_DIVIDE    },
    { "key_kp_multiply",  SDLK_KP_MULTIPLY  },
    { "key_kp_minus",     SDLK_KP_MINUS     },
    { "key_kp_plus",      SDLK_KP_PLUS      },
    { "key_kp_enter",     SDLK_KP_ENTER     },
    { "key_kp_equals",    SDLK_KP_EQUALS    },
    { "key_up",           SDLK_UP           },
    { "key_down",         SDLK_DOWN         },
    { "key_right",        SDLK_RIGHT        },
    { "key_left",         SDLK_LEFT         },
    { "key_insert",       SDLK_INSERT       },
    { "key_home",         SDLK_HOME         },
    { "key_end",          SDLK_END          },
    { "key_pageup",       SDLK_PAGEUP       },
    { "key_pagedown",     SDLK_PAGEDOWN     },
    { "key_F1",           SDLK_F1           },
    { "key_F2",           SDLK_F2           },
    { "key_F3",           SDLK_F3           },
    { "key_F4",           SDLK_F4           },
    { "key_F5",           SDLK_F5           },
    { "key_F6",           SDLK_F6           },
    { "key_F7",           SDLK_F7           },
    { "key_F8",           SDLK_F8           },
    { "key_F9",           SDLK_F9           },
    { "key_F10",          SDLK_F10          },
    { "key_F11",          SDLK_F11          },
    { "key_F12",          SDLK_F12          },
    { "key_F13",          SDLK_F13          },
    { "key_F14",          SDLK_F14          },
    { "key_F15",          SDLK_F15          },
    { "key_numlock",      SDLK_NUMLOCK      },
    { "key_capslock",     SDLK_CAPSLOCK     },
    { "key_scrollock",    SDLK_SCROLLOCK    },
    { "key_rshift",       SDLK_RSHIFT       },
    { "key_lshift",       SDLK_LSHIFT       },
    { "key_rctrl",        SDLK_RCTRL        },
    { "key_lctrl",        SDLK_LCTRL        },
    { "key_ralt",         SDLK_RALT         },
    { "key_lalt",         SDLK_LALT         },
    { "key_rmeta",        SDLK_RMETA        },
    { "key_lmeta",        SDLK_LMETA        },
    { "key_lsuper",       SDLK_LSUPER       },
    { "key_rsuper",       SDLK_RSUPER       },
    { "key_mode",         SDLK_MODE         },
    { "key_compose",      SDLK_COMPOSE      },
    { "key_help",         SDLK_HELP         },
    { "key_print",        SDLK_PRINT        },
    { "key_sysreq",       SDLK_SYSREQ       },
    { "key_break",        SDLK_BREAK        },
    { "key_menu",         SDLK_MENU         },
    { "key_power",        SDLK_POWER        },
    { "key_euro",         SDLK_EURO         },
    { "key_undo",         SDLK_UNDO         },
};

void luaopen_electro(lua_State *L)
{
    int nf = sizeof (functions) / sizeof (struct function_def);
    int nc = sizeof (constants) / sizeof (struct constant_def);
    int nk = sizeof (keys)      / sizeof (struct constant_def);
    int i;

    /* Create the Electro environment table. */

    lua_pushstring(L, "E");
    lua_newtable(L);

    /* Add all functions and constants to the table. */

    for (i = 0; i < nf; ++i)
        lua_function(L, functions[i].name, functions[i].func);
    for (i = 0; i < nc; ++i)
        lua_constant(L, constants[i].name, constants[i].value);
    for (i = 0; i < nk; ++i)
        lua_constant(L, keys[i].name, keys[i].value);

    /* Add an empty table to hold command line arguments. */

    lua_pushstring(L, "argument");
    lua_newtable(L);
    lua_settable(L, -3);

    /* Register the "E" environment table globally. */

    lua_settable(L, LUA_GLOBALSINDEX);

    /* Create a global metatable for all userdata objects to reference. */

    lua_pushstring(L, METATABLE_KEY);
    lua_newtable(L);
    lua_function(L, "__eq", E_equserdata);
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

    /* Load and execute the script. */

    if ((fp = open_file(file, "r")))
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
