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

#ifndef EVENT_H
#define EVENT_H

/*---------------------------------------------------------------------------*/

enum {
    EVENT_NULL,
    EVENT_DRAW,
    EVENT_EXIT,

    EVENT_CREATE_CAMERA,
    EVENT_CREATE_SPRITE,
    EVENT_CREATE_OBJECT,
    EVENT_CREATE_STRING,
    EVENT_CREATE_GALAXY,
    EVENT_CREATE_LIGHT,
    EVENT_CREATE_PIVOT,
    EVENT_CREATE_CLONE,
    EVENT_CREATE_IMAGE,
    EVENT_CREATE_BRUSH,

    EVENT_PARENT_ENTITY,
    EVENT_DELETE_ENTITY,
    EVENT_DELETE_IMAGE,
    EVENT_DELETE_BRUSH,

    EVENT_SET_ENTITY_POSITION,
    EVENT_SET_ENTITY_BASIS,
    EVENT_SET_ENTITY_SCALE,
    EVENT_SET_ENTITY_ALPHA,
    EVENT_SET_ENTITY_FLAGS,
    EVENT_SET_ENTITY_BOUND,
    
    EVENT_CREATE_MESH,
    EVENT_CREATE_VERT,
    EVENT_CREATE_FACE,
    EVENT_CREATE_EDGE,

    EVENT_DELETE_MESH,
    EVENT_DELETE_VERT,
    EVENT_DELETE_FACE,
    EVENT_DELETE_EDGE,

    EVENT_NORMAL_MESH,

    EVENT_SET_MESH,
    EVENT_SET_VERT,
    EVENT_SET_FACE,
    EVENT_SET_EDGE,

    EVENT_SET_IMAGE_PIXELS,
    EVENT_SET_BRUSH_IMAGE,
    EVENT_SET_BRUSH_COLOR,
    EVENT_SET_BRUSH_FLAGS,
    EVENT_SET_BRUSH_FRAG_SHADER,
    EVENT_SET_BRUSH_VERT_SHADER,
    EVENT_SET_BRUSH_UNIFORM,
    EVENT_SET_BRUSH_FRAG_PROG,
    EVENT_SET_BRUSH_VERT_PROG,
    EVENT_SET_BRUSH_FRAG_PARAM,
    EVENT_SET_BRUSH_VERT_PARAM,
    EVENT_SET_BRUSH_LINE_WIDTH,

    EVENT_SET_GALAXY_MAGNITUDE,
    EVENT_SET_CAMERA_OFFSET,
    EVENT_SET_CAMERA_STEREO,
    EVENT_SET_CAMERA_RANGE,
    EVENT_SET_CAMERA_IMAGE,
    EVENT_SET_SPRITE_RANGE,
    EVENT_SET_SPRITE_BRUSH,
    EVENT_SET_STRING_FILL ,
    EVENT_SET_STRING_LINE,
    EVENT_SET_STRING_TEXT,
    EVENT_SET_LIGHT_COLOR,
    EVENT_SET_BACKGROUND,
    EVENT_SET_FONT,

    EVENT_ADD_TILE,
    EVENT_SET_TILE_FLAGS,
    EVENT_SET_TILE_POSITION,
    EVENT_SET_TILE_VIEWPORT,
    EVENT_SET_TILE_LINE_SCREEN,
    EVENT_SET_TILE_VIEW_MIRROR,
    EVENT_SET_TILE_VIEW_OFFSET,
    EVENT_SET_TILE_QUALITY,
};

/*---------------------------------------------------------------------------*/

#endif
