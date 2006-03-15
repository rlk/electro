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
#include <SDL_endian.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "utility.h"
#include "matrix.h"
#include "vector.h"
#include "entity.h"
#include "sound.h"

/*---------------------------------------------------------------------------*/

static int   enabled     = 0;
static int   receiver    = 0;
static float attenuation = 10.0f;

#define BUFSIZE 2048
#define BUFFREQ 44100
#define BUFCHAN 2
#define BUFFORM AUDIO_S16

static SDL_AudioSpec spec;

/*---------------------------------------------------------------------------*/

struct frame
{
    float L;
    float R;
};

struct sound
{
    int   mode;
    float point;
    int   length;
    int   emitter;

    float amplitude;
    float frequency;

    float L_mix;
    float R_mix;

    struct frame *data;
};

static struct frame *mix;
static vector_t      sound;

/*---------------------------------------------------------------------------*/

static struct sound *get_sound(int i)
{
    return (struct sound *) vecget(sound, i);
}

static int new_sound(void)
{
    if (enabled)
    {
        int i, n = vecnum(sound);

        for (i = 0; i < n; ++i)
            if (get_sound(i)->data == 0)
                return i;

        return vecadd(sound);
    }
    return -1;
}

/*---------------------------------------------------------------------------*/
/* Convert between little-endian signed 16-bit short and native float.       */

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
static void swap16(void *p)
{
    unsigned char *c = (unsigned char *) p;
    unsigned char  t;

    t    = c[0];
    c[0] = c[1];
    c[1] =    t;
}
#endif

static short float_to_short(float f)
{
    short s;

    if      (f >= 1.0f) s =  32767;
    else if (f < -1.0f) s = -32768;
    else                s = (short) (f * 32768);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    swap16(&s);
#endif

    return s;
}

static float short_to_float(short s)
{
    short t = s;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    swap16(&t);
#endif

    return (float) t / 32768.0f;
}

/*---------------------------------------------------------------------------*/

static void mix_sound(struct sound *s, int n, float kl, float kr)
{
    float dl = (kl - s->L_mix) / n;
    float dr = (kr - s->R_mix) / n;

    int i;

    for (i = 0; i < n; )
    {
        /* Mix as much PCM as possible.  Fade toward the goal panning. */

        for (; i < n && s->point < s->length; ++i)
        {
            int j = (int) floor(s->point);

            mix[i].L += s->data[j].L * (s->L_mix + dl * i);
            mix[i].R += s->data[j].R * (s->R_mix + dr * i);
            s->point += s->frequency;
        }

        /* If the sound has played out, loop or stop. */

        if (i < n)
        {
            if (s->mode == SOUND_LOOP)
                s->point = 0.0f;
            else
            {
                s->mode  = SOUND_STOP;
                break;
            }
        }
    }

    s->L_mix = kl;
    s->R_mix = kr;
}

static void step_sound(void *data, Uint8 *stream, int length)
{
    short *output = (short *) stream;
    int i, j, n = vecnum(sound);

    int frames = length / (2 * spec.channels);

    /* Zero the mix buffer. */

    memset(mix, 0, spec.samples * sizeof (struct frame));

    /* Sum the playing sounds. */

    for (i = 0; i < n; ++i)
    {
        struct sound *s = get_sound(i);

        if (s->mode == SOUND_PLAY || s->mode == SOUND_LOOP)
        {
            float kl = 1.0f;
            float kr = 1.0f;

            /* If there exist source and listener for this sound... */

            if (receiver && s->emitter)
            {
                float x[3];
                float p[3];
                float q[3];
                float d[3];
                float dd, dx = 0;

                /* Find the direction and distance from source to listener. */

                get_entity_x_vector(receiver,   x);
                get_entity_position(receiver,   p);
                get_entity_position(s->emitter, q);

                d[0] = q[0] - p[0];
                d[1] = q[1] - p[1];
                d[2] = q[2] - p[2];

                dd = (float) sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);

                if (dd > 0.01)
                {
                    d[0] /= dd;
                    d[1] /= dd;
                    d[2] /= dd;
                
                    /* Compute the panning scalars. */

                    dx = d[0] * x[0] + d[2] * x[2];

                    if (dx < 0) kr = 1.0f + dx;
                    if (dx > 0) kl = 1.0f - dx;

                    kr *= (attenuation - dd) / attenuation;
                    kl *= (attenuation - dd) / attenuation;
                }

                if (kl < 0) kl = 0;
                if (kr < 0) kr = 0;
            }

            /* Mix this sound. */
            
            mix_sound(s, frames, kl * s->amplitude,
                                 kr * s->amplitude);
        }
    }

    /* Copy the mix buffer to the output buffer. */

    for (i = 0, j = 0; i < frames; ++i)
    {
        output[j++] = float_to_short(mix[i].L);
        output[j++] = float_to_short(mix[i].R);
    }
}

/*---------------------------------------------------------------------------*/

/* ov_pcm_total seems to under-report the length of some OGGs, so this hack  */
/* makes an initial pass over the file and computes the length by hand.      */

static int size_sound(const char *filename)
{
    FILE          *fp;
    OggVorbis_File vf;
    char buf[BUFSIZE];

    int b, n, s = 0;

    if ((fp = open_file(filename, FMODE_RB)))
    {
        if (ov_open(fp, &vf, NULL, 0) == 0)
        {
            struct vorbis_info *I = ov_info(&vf, -1);

            while ((n = ov_read(&vf, buf, BUFSIZE, 0, 2, 1, &b)) > 0)
                s += n / (I->channels * 2);

            ov_clear(&vf);
        }
        else fclose(fp);
    }
    return s;
}

/*---------------------------------------------------------------------------*/

int load_sound(const char *filename)
{
    int   i = -1;
    FILE *fp;

    OggVorbis_File vf;

    if (enabled && (i = new_sound()) >= 0)
    {
        struct sound *s = get_sound(i);

        int length = size_sound(filename);

        /* Open the file and initialize Ogg Vorbis decoding. */

        if ((fp = open_file(filename, FMODE_RB)))
        {
            if (ov_open(fp, &vf, NULL, 0) == 0)
            {
                struct vorbis_info *I = ov_info(&vf, -1);

                short buf[BUFSIZE];
                int   b, j, k = 0, n;

                char *p = (char *) buf;

                /* Initialize the sound header. */

                s->mode      = SOUND_STOP;
                s->length    = length;
                s->L_mix     = 0.0f;
                s->R_mix     = 0.0f;
                s->amplitude = 1.0f;
                s->frequency = 1.0f;

                /* Allocate PCM storage. */

                s->data = (struct frame *) malloc(s->length *
                                                  sizeof (struct frame));

                /* Decode all PCM.  Convert it to 2-channel float. */

                if (I->channels == 2)
                {
                    while ((n = ov_read(&vf, p, BUFSIZE * 2, 0, 2, 1, &b)) > 0)
                        for (j = 0; j < n / 4; ++j, ++k)
                        {
                            s->data[k].L = short_to_float(buf[j * 2 + 0]);
                            s->data[k].R = short_to_float(buf[j * 2 + 1]);
                        }
                }
                else
                {
                    while ((n = ov_read(&vf, p, BUFSIZE * 2, 0, 2, 1, &b)) > 0)
                        for (j = 0; j < n / 2; ++j, ++k)
                            s->data[k].L =
                            s->data[k].R = short_to_float(buf[j]);
                }
                ov_clear(&vf);
            }
            else fclose(fp);
        }
        else error("OGG file '%s': %s", filename, system_error());
    }
    return i;
}

void free_sound(int i)
{
    if (enabled)
    {
        struct sound *s = get_sound(i);

        if (s->data) free(s->data);

        memset(s, 0, sizeof (struct sound));
    }
}

/*---------------------------------------------------------------------------*/

static void set_sound_mode(int i, int mode)
{
    if (enabled)
    {
        struct sound *s = get_sound(i);

        SDL_LockAudio();
        {
            s->point = 0.0f;
            s->L_mix = 0.0f;
            s->R_mix = 0.0f;
            s->mode  = mode;
        }
        SDL_UnlockAudio();
    }
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

void set_sound_emitter(int i, int j)
{
    if (enabled)
        get_sound(i)->emitter = j;
}

void set_sound_receiver(int j, float a)
{
    if (enabled)
    {
        receiver    = j;
        attenuation = a;
    }
}

void set_sound_amplitude(int i, float a)
{
    if (enabled)
        get_sound(i)->amplitude = MAX(a, 0.0f);
}

void set_sound_frequency(int i, float f)
{
    if (enabled && 0.0f <= f)
        get_sound(i)->frequency = MAX(f, 0.0f);
}

/*---------------------------------------------------------------------------*/

void nuke_sounds(void)
{
    if (enabled)
    {
        int i, n = vecnum(sound);

        for (i = 0; i < n; ++i)
            if (get_sound(i)->data)
                free_sound(i);
    }
}

int startup_sound(void)
{
    spec.callback = step_sound;
    spec.channels = BUFCHAN;
    spec.samples  = BUFSIZE;
    spec.format   = BUFFORM;
    spec.freq     = BUFFREQ;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0)
    {
        if (SDL_OpenAudio(&spec, NULL) == 0)
        {
            mix = (struct frame *) malloc(spec.samples *
                                          sizeof (struct frame));

            if ((sound = vecnew(32, sizeof (struct sound))))
                return (enabled = 1);
        }
        else fprintf(stderr, "%s\n", SDL_GetError());
    }
    else fprintf(stderr, "%s\n", SDL_GetError());

    return 1;
}

/*---------------------------------------------------------------------------*/
