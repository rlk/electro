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

#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "utility.h"
#include "vector.h"
#include "entity.h"
#include "sound.h"

/*---------------------------------------------------------------------------*/

#define BUFSIZE 2048
#define BUFFREQ 44100
#define BUFCHAN 2
#define BUFFORM AUDIO_S16

static SDL_AudioSpec spec;
static float        *buff;

/*---------------------------------------------------------------------------*/

struct sound
{
    int mode;
    int chan;

    OggVorbis_File file;
};

static vector_t sound;

/*---------------------------------------------------------------------------*/

#define S(i) ((struct sound *) vecget(sound, i))

static int new_sound(void)
{
    int i, n = vecnum(sound);

    for (i = 0; i < n; ++i)
        if (S(i)->chan == 0)
            return i;

    return vecadd(sound);
}

/*---------------------------------------------------------------------------*/

static int mix_sound(int i, float *fbuf, short *sbuf, int max)
{
    char *buf = (char *) sbuf;

    int j, k, d = (S(i)->chan == 2) ? 1 : 2;

    int siz = max / d;
    int tot = 0;
    int len = 0;
    int bit = 0;

    /* Try to fill the short buffer with Ogg.  Rewind loops as necessary. */

    while (siz > 0)
        if ((len = ov_read(&S(i)->file, buf, siz, 0, 2, 1, &bit)) > 0)
        {
            tot += len;
            buf += len;
            siz -= len;
        }
        else
        {
            if (S(i)->mode == SOUND_LOOP)
                ov_pcm_seek(&S(i)->file, 0);
            else
                break;
        }
        
    /* Mix read data into the float buffer, duplicating a mono channel. */

    tot = tot / sizeof (short);

    for (j = 0; j < tot; ++j)
        for (k = 0; k < d; ++k)
            fbuf[j * d + k] += (float) sbuf[j];

    /* Signal EOF. */

    return (tot == 0);
}

static void step_sound(void *data, Uint8 *stream, int length)
{
    short *output = (short *) stream;
    int i, n = vecnum(sound);

    /* Zero the mix buffer. */

    memset(buff, 0, spec.samples * spec.channels * sizeof (float));

    /* Sum the playing sounds (using the output buffer as temp space). */

    for (i = 0; i < n; ++i)
        if (S(i)->mode == SOUND_PLAY ||
            S(i)->mode == SOUND_LOOP)
        {
            if (mix_sound(i, buff, output, length))
                S(i)->mode = SOUND_STOP;
        }

    /* Copy the mix buffer to the output buffer, clamping as necessary. */

    for (i = 0; i < length / 2; ++i)
        if      (buff[i] >  32767) output[i] =  32767;
        else if (buff[i] < -32768) output[i] = -32768;
        else                       output[i] = (short) (buff[i]);
}

/*---------------------------------------------------------------------------*/

int init_sound(void)
{
    spec.callback = step_sound;
    spec.channels = BUFCHAN;
    spec.samples  = BUFSIZE;
    spec.format   = BUFFORM;
    spec.freq     = BUFFREQ;

    if (SDL_OpenAudio(&spec, NULL) == 0)
    {
        buff = (float *) malloc(spec.samples * spec.channels * sizeof (float));

        if ((sound = vecnew(32, sizeof (struct sound))))
            return 1;
    }
    else fprintf(stderr, "%s\n", SDL_GetError());

    return 0;
}

/*---------------------------------------------------------------------------*/

int create_sound(const char *filename)
{
    int   i;
    FILE *fp;

    if ((i = new_sound()) >= 0)
    {
        if ((fp = open_file(filename, FMODE_RB)))
        {
            if (ov_open(fp, &S(i)->file, NULL, 0) == 0)
            {
                struct vorbis_info *I = ov_info(&S(i)->file, -1);

                S(i)->mode = SOUND_STOP;
                S(i)->chan = I->channels;

                return i;
            }
            fclose(fp);
        }
        else error("OGG file '%s': %s", filename, system_error());
    }
    return -1;
}

void delete_sound(int i)
{
    ov_clear(&S(i)->file);
    memset(S(i), 0, sizeof (struct sound));
}

/*---------------------------------------------------------------------------*/

static void set_sound_mode(int i, int mode)
{
    SDL_LockAudio();
    {
        ov_pcm_seek(&S(i)->file, 0);
        S(i)->mode = mode;
    }
    SDL_UnlockAudio();
}

void stop_sound(int i)
{
    set_sound_mode(i, SOUND_STOP);
}

void play_sound(int i)
{
    set_sound_mode(i, SOUND_PLAY);
}

void loop_sound(int i)
{
    set_sound_mode(i, SOUND_LOOP);
}

/*---------------------------------------------------------------------------*/
