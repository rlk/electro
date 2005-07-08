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
#define EVENT_CREATE_GALAXY         6
#define EVENT_CREATE_LIGHT          7
#define EVENT_CREATE_PIVOT          8
#define EVENT_CREATE_IMAGE          9
#define EVENT_CREATE_CLONE         10

#define EVENT_PARENT_ENTITY        11
#define EVENT_DELETE_ENTITY        12

#define EVENT_SET_ENTITY_POSITION  13
#define EVENT_SET_ENTITY_BASIS     14
#define EVENT_SET_ENTITY_SCALE     15
#define EVENT_SET_ENTITY_BOUND     16
#define EVENT_SET_ENTITY_ALPHA     17
#define EVENT_SET_ENTITY_FLAG      18
#define EVENT_SET_ENTITY_FRAG_PROG 19
#define EVENT_SET_ENTITY_VERT_PROG 20

#define EVENT_SET_GALAXY_MAGNITUDE 21
#define EVENT_SET_CAMERA_OFFSET    22
#define EVENT_SET_CAMERA_STEREO    23
#define EVENT_SET_SPRITE_RANGE     24
#define EVENT_SET_LIGHT_COLOR      25
#define EVENT_SET_BACKGROUND       26

#define EVENT_ADD_TILE             27
#define EVENT_SET_TILE_FLAG        28
#define EVENT_SET_TILE_POSITION    29
#define EVENT_SET_TILE_VIEWPORT    30
#define EVENT_SET_TILE_LINE_SCREEN 31
#define EVENT_SET_TILE_VIEW_MIRROR 32
#define EVENT_SET_TILE_VIEW_OFFSET 33

/*---------------------------------------------------------------------------*/

#endif
