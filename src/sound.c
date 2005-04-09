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

#include "utility.h"
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

#define SMAXINIT 128

static struct sound *S;
static int           S_max;

static int sound_exists(int sd)
{
    return (S && 0 <= sd && sd < S_max && S[sd].mode);
}

static int alloc_sound(void)
{
    int sd = -1;

    S = (struct sound *) balloc(S, &sd, &S_max,
                                sizeof (struct sound), sound_exists);
    return sd;
}

/*---------------------------------------------------------------------------*/

static int mix_sound(int sd, float *fbuf, short *sbuf, int max)
{
    char *buf = (char *) sbuf;

    int i, j, d = (S[sd].chan == 2) ? 1 : 2;

    int siz = max / d;
    int tot = 0;
    int len = 0;
    int bit = 0;

    /* Try to fill the short buffer with Ogg.  Rewind loops as necessary. */

    while (siz > 0)
        if ((len = ov_read(&S[sd].file, buf, siz, 0, 2, 1, &bit)) > 0)
        {
            tot += len;
            buf += len;
            siz -= len;
        }
        else
        {
            if (S[sd].mode == SOUND_LOOP)
                ov_pcm_seek(&S[sd].file, 0);
            else
                break;
        }
        
    /* Mix read data into the float buffer, duplicating a mono channel. */

    tot = tot / sizeof (short);

    for (i = 0; i < tot; ++i)
        for (j = 0; j < d; ++j)
            fbuf[i * d + j] += (float) sbuf[i];

    /* Signal EOF. */

    return (tot == 0);
}

static void step_sound(void *data, Uint8 *stream, int length)
{
    short *output = (short *) stream;
    int i, sd;

    /* Zero the mix buffer. */

    memset(buff, 0, spec.samples * spec.channels * sizeof (float));

    /* Sum the playing sounds (using the output buffer as temp space). */

    for (sd = 0; sd < S_max; ++sd)
        if (S[sd].mode == SOUND_PLAY ||
            S[sd].mode == SOUND_LOOP)
        {
            if (mix_sound(sd, buff, output, length))
                S[sd].mode = SOUND_STOP;
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

        if ((S = (struct sound *) calloc(SMAXINIT, sizeof (struct sound))))
        {
            S_max = SMAXINIT;
            return 1;
        }
    }
    else fprintf(stderr, "%s\n", SDL_GetError());

    return 0;
}

/*---------------------------------------------------------------------------*/

int create_sound(const char *filename)
{
    int   sd;
    FILE *fp;

    if (S && (sd = alloc_sound()) >= 0)
    {
        if ((fp = fopen(filename, FMODE_RB)))
        {
            if (ov_open(fp, &S[sd].file, NULL, 0) == 0)
            {
                struct vorbis_info *I = ov_info(&S[sd].file, -1);

                S[sd].mode = SOUND_STOP;
                S[sd].chan = I->channels;

                return sd;
            }
            fclose(fp);
        }
        else error("OGG file '%s': %s", filename, system_error());
    }
    return -1;
}

void delete_sound(int sd)
{
    if (sound_exists(sd))
    {
        ov_clear(&S[sd].file);

        memset(S + sd, 0, sizeof (struct sound));
    }
}

/*---------------------------------------------------------------------------*/

static void set_sound_mode(int sd, int mode)
{
    if (sound_exists(sd))
    {
        SDL_LockAudio();
        {
            ov_pcm_seek(&S[sd].file, 0);
            S[sd].mode = mode;
        }
        SDL_UnlockAudio();
    }
}

void stop_sound(int sd)
{
    set_sound_mode(sd, SOUND_STOP);
}

void play_sound(int sd)
{
    set_sound_mode(sd, SOUND_PLAY);
}

void loop_sound(int sd)
{
    set_sound_mode(sd, SOUND_LOOP);
}

/*---------------------------------------------------------------------------*/
