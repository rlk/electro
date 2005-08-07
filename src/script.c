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
    if (id <= 0)
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

    return 0.0;
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
    set_entity_body_attr_f(E_getentity (L, -3),
                           L_getinteger(L, -2),
                           L_getnumber (L, -1));
    return 0;
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
    lua_pushnumber(L, get_entity_join_attr_f(E_getentity (L, -3),
                                             E_getentity (L, -2),
                                             L_getinteger(L, -1)));
    return 1;
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
    lua_pushnumber(L, get_mesh(E_getobject(L, -2), L_getinteger(L, -1)));
    return 1;
}

static int E_get_vert(lua_State *L)
{
    float v[3];
    float n[3];
    float t[2];

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
    int vi[3];

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
    int vi[2];

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
    E_pushimage(L, send_create_image(L_getstring(L, -1)));
    return 1;
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

static int E_delete_image(lua_State *L)
{
    return 0;
}

/*===========================================================================*/
/* Brush functions                                                           */

static int E_create_brush(lua_State *L)
{
    E_pushbrush(L, send_create_brush(NULL, NULL));
    return 1;
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
    send_set_brush_image(E_getbrush(L, -2),
                         E_getimage(L, -1));
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

static int E_delete_brush(lua_State *L)
{
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

static int E_exit(lua_State *L)
{
    SDL_Event e;

    e.type = SDL_QUIT;
    SDL_PushEvent(&e);

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

static void lua_function(lua_State *L, const char *n, lua_CFunction f)
{
    lua_pushstring(L, n);
    lua_pushstring(L, n);
    lua_pushcclosure(L, f, 1);
    lua_settable(L, -3);
}

static void lua_constant(lua_State *L, const char *n, int v)
{
    lua_pushstring(L, n);
    lua_pushnumber(L, v);
    lua_settable(L, -3);
}

void luaopen_electro(lua_State *L)
{
    /* Create the Electro environment table. */

    lua_pushstring(L, "E");
    lua_newtable(L);

    /* Entity contructors and destructors */

    lua_function(L, "create_camera",         E_create_camera);
    lua_function(L, "create_sprite",         E_create_sprite);
    lua_function(L, "create_object",         E_create_object);
    lua_function(L, "create_string",         E_create_string);
    lua_function(L, "create_galaxy",         E_create_galaxy);
    lua_function(L, "create_light",          E_create_light);
    lua_function(L, "create_pivot",          E_create_pivot);
    lua_function(L, "create_clone",          E_create_clone);

    /* Entity functions */

    lua_function(L, "parent_entity",         E_parent_entity);
    lua_function(L, "delete_entity",         E_delete_entity);

    lua_function(L, "get_entity_parent",     E_get_entity_parent);
    lua_function(L, "get_entity_child",      E_get_entity_child);

    lua_function(L, "set_entity_position",   E_set_entity_position);
    lua_function(L, "set_entity_rotation",   E_set_entity_rotation);
    lua_function(L, "set_entity_scale",      E_set_entity_scale);
    lua_function(L, "set_entity_alpha",      E_set_entity_alpha);
    lua_function(L, "set_entity_flags",      E_set_entity_flags);

    lua_function(L, "add_entity_force",      E_add_entity_force);
    lua_function(L, "add_entity_torque",     E_add_entity_torque);

    lua_function(L, "get_entity_position",   E_get_entity_position);
    lua_function(L, "get_entity_x_vector",   E_get_entity_x_vector);
    lua_function(L, "get_entity_y_vector",   E_get_entity_y_vector);
    lua_function(L, "get_entity_z_vector",   E_get_entity_z_vector);
    lua_function(L, "get_entity_scale",      E_get_entity_scale);
    lua_function(L, "get_entity_bound",      E_get_entity_bound);
    lua_function(L, "get_entity_alpha",      E_get_entity_alpha);

    lua_function(L, "move_entity",           E_move_entity);
    lua_function(L, "turn_entity",           E_turn_entity);

    lua_function(L, "set_entity_body_type",  E_set_entity_body_type);
    lua_function(L, "set_entity_geom_type",  E_set_entity_geom_type);
    lua_function(L, "set_entity_joint_type", E_set_entity_joint_type);

    lua_function(L, "set_entity_body_attr",  E_set_entity_body_attr);
    lua_function(L, "set_entity_geom_attr",  E_set_entity_geom_attr);
    lua_function(L, "set_entity_joint_attr", E_set_entity_joint_attr);
    lua_function(L, "get_entity_geom_attr",  E_get_entity_geom_attr);
    lua_function(L, "get_entity_joint_attr", E_get_entity_joint_attr);

    /* Object functions */

    lua_function(L, "create_mesh",           E_create_mesh);
    lua_function(L, "create_vert",           E_create_vert);
    lua_function(L, "create_face",           E_create_face);
    lua_function(L, "create_edge",           E_create_edge);

    lua_function(L, "set_mesh",              E_set_mesh);
    lua_function(L, "set_vert",              E_set_vert);
    lua_function(L, "set_face",              E_set_face);
    lua_function(L, "set_edge",              E_set_edge);

    lua_function(L, "get_mesh",              E_get_mesh);
    lua_function(L, "get_vert",              E_get_vert);
    lua_function(L, "get_face",              E_get_face);
    lua_function(L, "get_edge",              E_get_edge);

    lua_function(L, "get_mesh_count",        E_get_mesh_count);
    lua_function(L, "get_vert_count",        E_get_vert_count);
    lua_function(L, "get_face_count",        E_get_face_count);
    lua_function(L, "get_edge_count",        E_get_edge_count);

    lua_function(L, "delete_mesh",           E_delete_mesh);
    lua_function(L, "delete_vert",           E_delete_vert);
    lua_function(L, "delete_face",           E_delete_face);
    lua_function(L, "delete_edge",           E_delete_edge);

    /* Sprite functions */

    lua_function(L, "set_sprite_brush",      E_set_sprite_brush);
    lua_function(L, "set_sprite_range",      E_set_sprite_range);

    /* String functions */

    lua_function(L, "set_string_fill",       E_set_string_fill);
    lua_function(L, "set_string_line",       E_set_string_line);
    lua_function(L, "set_string_text",       E_set_string_text);

    /* Camera functions */

    lua_function(L, "set_camera_offset",     E_set_camera_offset);
    lua_function(L, "set_camera_stereo",     E_set_camera_stereo);

    /* Light functions */

    lua_function(L, "set_light_color",       E_set_light_color);

    /* Galaxy functions */

    lua_function(L, "set_galaxy_magnitude",  E_set_galaxy_magnitude);
    lua_function(L, "get_star_index",        E_get_star_index);
    lua_function(L, "get_star_position",     E_get_star_position);

    /* Image functions */

    lua_function(L, "create_image",          E_create_image);
    lua_function(L, "delete_image",          E_delete_image);
    lua_function(L, "get_image_pixel",       E_get_image_pixel);
    lua_function(L, "get_image_size",        E_get_image_size);

    /* Brush functions */

    lua_function(L, "create_brush",          E_create_brush);
    lua_function(L, "delete_brush",          E_delete_brush);
    lua_function(L, "set_brush_flags",       E_set_brush_flags);
    lua_function(L, "set_brush_image",       E_set_brush_image);
    lua_function(L, "set_brush_color",       E_set_brush_color);
    lua_function(L, "set_brush_frag_prog",   E_set_brush_frag_prog);
    lua_function(L, "set_brush_vert_prog",   E_set_brush_vert_prog);

    /* Sound functions */

    lua_function(L, "load_sound",            E_load_sound);
    lua_function(L, "free_sound",            E_free_sound);
    lua_function(L, "stop_sound",            E_stop_sound);
    lua_function(L, "play_sound",            E_play_sound);
    lua_function(L, "loop_sound",            E_loop_sound);

    /* Console functions */

    lua_function(L, "clear_console",         E_clear_console);
    lua_function(L, "color_console",         E_color_console);
    lua_function(L, "print_console",         E_print_console);

    /* Configuration */

    lua_function(L, "add_host",              E_add_host);
    lua_function(L, "add_tile",              E_add_tile);
    lua_function(L, "set_host_flags",        E_set_host_flags);
    lua_function(L, "set_tile_flags",        E_set_tile_flags);
    lua_function(L, "set_tile_position",     E_set_tile_position);
    lua_function(L, "set_tile_viewport",     E_set_tile_viewport);
    lua_function(L, "set_tile_line_screen",  E_set_tile_line_screen);
    lua_function(L, "set_tile_view_mirror",  E_set_tile_view_mirror);
    lua_function(L, "set_tile_view_offset",  E_set_tile_view_offset);
    lua_function(L, "get_display_union",     E_get_display_union);
    lua_function(L, "get_display_bound",     E_get_display_bound);

    /* Miscellaneous */

    lua_function(L, "enable_timer",          E_enable_timer);
    lua_function(L, "get_joystick",          E_get_joystick);
    lua_function(L, "get_modifier",          E_get_modifier);
    lua_function(L, "set_typeface",          E_set_typeface);
    lua_function(L, "set_background",        E_set_background);
    lua_function(L, "exit",                  E_exit);

    /* Entity constants */

    lua_constant(L, "entity_flag_hidden",        FLAG_HIDDEN);
    lua_constant(L, "entity_flag_wireframe",     FLAG_WIREFRAME);
    lua_constant(L, "entity_flag_billboard",     FLAG_BILLBOARD);
    lua_constant(L, "entity_flag_line_smooth",   FLAG_LINE_SMOOTH);
    lua_constant(L, "entity_flag_pos_tracked_0", FLAG_POS_TRACKED_0);
    lua_constant(L, "entity_flag_rot_tracked_0", FLAG_ROT_TRACKED_0);
    lua_constant(L, "entity_flag_pos_tracked_1", FLAG_POS_TRACKED_1);
    lua_constant(L, "entity_flag_rot_tracked_1", FLAG_ROT_TRACKED_1);

    /* Body constants */

    lua_constant(L, "body_attr_gravity",         BODY_ATTR_GRAVITY);

    /* Geom constants */

    lua_constant(L, "geom_type_none",            -1);
    lua_constant(L, "geom_type_box",             dBoxClass);
    lua_constant(L, "geom_type_plane",           dPlaneClass);
    lua_constant(L, "geom_type_sphere",          dSphereClass);
    lua_constant(L, "geom_type_capsule",         dCCylinderClass);

    lua_constant(L, "geom_attr_category",        GEOM_ATTR_CATEGORY);
    lua_constant(L, "geom_attr_collider",        GEOM_ATTR_COLLIDER);
    lua_constant(L, "geom_attr_response",        GEOM_ATTR_RESPONSE);
    lua_constant(L, "geom_attr_callback",        GEOM_ATTR_CALLBACK);
    lua_constant(L, "geom_attr_mass",            GEOM_ATTR_MASS);
    lua_constant(L, "geom_attr_bounce",          GEOM_ATTR_BOUNCE);
    lua_constant(L, "geom_attr_friction",        GEOM_ATTR_FRICTION);
    lua_constant(L, "geom_attr_soft_erp",        GEOM_ATTR_SOFT_ERP);
    lua_constant(L, "geom_attr_soft_cfm",        GEOM_ATTR_SOFT_CFM);

    /* Joint constants */

    lua_constant(L, "joint_type_ball",           dJointTypeBall);
    lua_constant(L, "joint_type_hinge",          dJointTypeHinge);
    lua_constant(L, "joint_type_slider",         dJointTypeSlider);
    lua_constant(L, "joint_type_universal",      dJointTypeUniversal);
    lua_constant(L, "joint_type_hinge_2",        dJointTypeHinge2);

    lua_constant(L, "joint_attr_anchor",         JOINT_ATTR_ANCHOR);
    lua_constant(L, "joint_attr_axis_1",         JOINT_ATTR_AXIS_1);
    lua_constant(L, "joint_attr_axis_2",         JOINT_ATTR_AXIS_2);
    lua_constant(L, "joint_attr_value",          JOINT_ATTR_VALUE);
    lua_constant(L, "joint_attr_rate_1",         JOINT_ATTR_RATE_1);
    lua_constant(L, "joint_attr_rate_2",         JOINT_ATTR_RATE_2);
    lua_constant(L, "joint_attr_lo_stop",        dParamLoStop);
    lua_constant(L, "joint_attr_lo_stop_2",      dParamLoStop2);
    lua_constant(L, "joint_attr_hi_stop",        dParamHiStop);
    lua_constant(L, "joint_attr_hi_stop_2",      dParamHiStop2);
    lua_constant(L, "joint_attr_velocity",       dParamVel);
    lua_constant(L, "joint_attr_velocity_2",     dParamVel2);
    lua_constant(L, "joint_attr_force_max",      dParamFMax);
    lua_constant(L, "joint_attr_force_max_2",    dParamFMax2);
    lua_constant(L, "joint_attr_bounce",         dParamBounce);
    lua_constant(L, "joint_attr_bounce_2",       dParamBounce2);
    lua_constant(L, "joint_attr_cfm",            dParamCFM);
    lua_constant(L, "joint_attr_cfm_2",          dParamCFM2);
    lua_constant(L, "joint_attr_stop_erp",       dParamStopERP);
    lua_constant(L, "joint_attr_stop_erp_2",     dParamStopERP2);
    lua_constant(L, "joint_attr_stop_cfm",       dParamStopCFM);
    lua_constant(L, "joint_attr_stop_cfm_2",     dParamStopCFM2);
    lua_constant(L, "joint_attr_susp_erp",       dParamSuspensionERP);
    lua_constant(L, "joint_attr_susp_cfm",       dParamSuspensionCFM);

    /* Brush constants */

    lua_constant(L, "brush_flag_diffuse",        BRUSH_DIFFUSE);
    lua_constant(L, "brush_flag_specular",       BRUSH_SPECULAR);
    lua_constant(L, "brush_flag_ambient",        BRUSH_AMBIENT);
    lua_constant(L, "brush_flag_shiny",          BRUSH_SHINY);
    lua_constant(L, "brush_flag_transparent",    BRUSH_TRANSPARENT);
    lua_constant(L, "brush_flag_unlit",          BRUSH_UNLIT);

    /* Configuration constants */

    lua_constant(L, "host_flag_full",            HOST_FULL);
    lua_constant(L, "host_flag_stereo",          HOST_STEREO);
    lua_constant(L, "host_flag_framed",          HOST_FRAMED);

    lua_constant(L, "tile_flag_flip_x",          TILE_FLIP_X);
    lua_constant(L, "tile_flag_flip_y",          TILE_FLIP_Y);
    lua_constant(L, "tile_flag_offset",          TILE_OFFSET);
    lua_constant(L, "tile_flag_mirror",          TILE_MIRROR);
    lua_constant(L, "tile_flag_test",            TILE_TEST);

    /* Camera constants */

    lua_constant(L, "camera_type_orthogonal",    CAMERA_ORTHO);
    lua_constant(L, "camera_type_perspective",   CAMERA_PERSP);

    lua_constant(L, "stereo_mode_none",          STEREO_NONE);
    lua_constant(L, "stereo_mode_quad",          STEREO_QUAD);
    lua_constant(L, "stereo_mode_red_blue",      STEREO_RED_BLUE);
    lua_constant(L, "stereo_mode_varrier_01",    STEREO_VARRIER_01);
    lua_constant(L, "stereo_mode_varrier_11",    STEREO_VARRIER_11);
    lua_constant(L, "stereo_mode_varrier_33",    STEREO_VARRIER_33);
    lua_constant(L, "stereo_mode_varrier_41",    STEREO_VARRIER_41);

    /* Light constants */

    lua_constant(L, "light_type_positional",     LIGHT_POSITIONAL);
    lua_constant(L, "light_type_directional",    LIGHT_DIRECTIONAL);

    /* Miscellaneous constants */

    lua_constant(L, "key_modifier_shift",        KMOD_SHIFT);
    lua_constant(L, "key_modifier_control",      KMOD_CTRL);
    lua_constant(L, "key_modifier_alt",          KMOD_ALT);

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
