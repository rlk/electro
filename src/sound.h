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

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

/*---------------------------------------------------------------------------*/

#define SOUND_NULL 0
#define SOUND_STOP 1
#define SOUND_PLAY 2
#define SOUND_LOOP 3

struct sound
{
    int   mode;
    int   chan;

    OggVorbis_File file;
};

/*---------------------------------------------------------------------------*/

int  sound_init(void);

int  sound_create(const char *);
void sound_delete(int);

void sound_stop(int);
void sound_play(int);
void sound_loop(int);

/*---------------------------------------------------------------------------*/

#endif
