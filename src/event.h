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

#define EVENT_NULL                  0
#define EVENT_DRAW                  1
#define EVENT_EXIT                  2

#define EVENT_CREATE_CAMERA         3
#define EVENT_CREATE_SPRITE         4
#define EVENT_CREATE_OBJECT         5
#define EVENT_CREATE_STRING         6
#define EVENT_CREATE_GALAXY         7
#define EVENT_CREATE_LIGHT          8
#define EVENT_CREATE_PIVOT          9
#define EVENT_CREATE_CLONE         10
#define EVENT_CREATE_IMAGE         11
#define EVENT_CREATE_BRUSH         12

#define EVENT_PARENT_ENTITY        13
#define EVENT_DELETE_ENTITY        14

#define EVENT_SET_ENTITY_POSITION  15
#define EVENT_SET_ENTITY_BASIS     16
#define EVENT_SET_ENTITY_SCALE     17
#define EVENT_SET_ENTITY_BOUND     18
#define EVENT_SET_ENTITY_ALPHA     19
#define EVENT_SET_ENTITY_FLAGS     20

#define EVENT_CREATE_MESH          21
#define EVENT_CREATE_VERT          22
#define EVENT_CREATE_FACE          23
#define EVENT_CREATE_EDGE          24

#define EVENT_DELETE_MESH          25
#define EVENT_DELETE_VERT          26
#define EVENT_DELETE_FACE          27
#define EVENT_DELETE_EDGE          28

#define EVENT_SET_MESH             29
#define EVENT_SET_VERT             30
#define EVENT_SET_FACE             31
#define EVENT_SET_EDGE             32

#define EVENT_SET_BRUSH_IMAGE      33
#define EVENT_SET_BRUSH_COLOR      34
#define EVENT_SET_BRUSH_FLAGS      35
#define EVENT_SET_BRUSH_FRAG_PROG  36
#define EVENT_SET_BRUSH_VERT_PROG  37

#define EVENT_SET_GALAXY_MAGNITUDE 38
#define EVENT_SET_CAMERA_OFFSET    39
#define EVENT_SET_CAMERA_STEREO    40
#define EVENT_SET_SPRITE_RANGE     41
#define EVENT_SET_SPRITE_BRUSH     42
#define EVENT_SET_STRING_FILL      43
#define EVENT_SET_STRING_LINE      44
#define EVENT_SET_STRING_TEXT      45
#define EVENT_SET_LIGHT_COLOR      46
#define EVENT_SET_BACKGROUND       47
#define EVENT_SET_FONT             48

#define EVENT_ADD_TILE             49
#define EVENT_SET_TILE_FLAGS       50
#define EVENT_SET_TILE_POSITION    51
#define EVENT_SET_TILE_VIEWPORT    52
#define EVENT_SET_TILE_LINE_SCREEN 53
#define EVENT_SET_TILE_VIEW_MIRROR 54
#define EVENT_SET_TILE_VIEW_OFFSET 55

/*---------------------------------------------------------------------------*/

#endif
