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

#ifndef SOUND_H
#define SOUND_H

/*---------------------------------------------------------------------------*/

#define SOUND_NULL 0
#define SOUND_STOP 1
#define SOUND_PLAY 2
#define SOUND_LOOP 3

/*---------------------------------------------------------------------------*/

int startup_sound(void);

int  load_sound(const char *);
void free_sound(int);
void stop_sound(int);
void play_sound(int);
void loop_sound(int);

void set_sound_entity(int, int);
void set_sound_camera(int);

void set_sound_amplitude(int, float);
void set_sound_frequency(int, float);

void nuke_sounds(void);

/*---------------------------------------------------------------------------*/

#endif
