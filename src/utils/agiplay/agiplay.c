/* AGI player for i386 Linux and OSS
 * Written by Claudio Matsuoka <claudio@helllabs.org>
 * Sun Mar 21 13:27:35 EST 1999
 *
 * Based on the format of the AGI SOUND resource as described by
 * Lance Ewing <lance.e@ihug.co.nz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "sarien.h"
#include "sound.h"
#include "rand.h"

static int playing;
static struct channel_info chn[NUM_CHANNELS];
static int endflag = -1;
static UINT8 *song;
struct sound_driver *snd;
struct gfx_driver *gfx;

static short *snd_buffer;


static short waveform[64] = {
       0,   8,  16,  24,  32,  40,  48,  56,
      64,  72,  80,  88,  96, 104, 112, 120,
     128, 136, 144, 152, 160, 168, 176, 184,
     192, 200, 208, 216, 224, 232, 240, 255,
       0,-248,-240,-232,-224,-216,-208,-200,
    -192,-184,-176,-168,-160,-152,-144,-136,
    -128,-120,-112,-104, -96, -88, -80, -72,
    -64, -56, -48, -40,  -32, -24, -16,  -8   	/* Ramp up */
};


void report (char *s, ...)
{
}

static void stop_note (int i)
{
	chn[i].adsr = AGI_SOUND_ENV_RELEASE;

	/* Stop chorus ;) */
	if (i < 3)
		stop_note (i + 4);

#ifdef __TURBOC__
	if (i == 0)
		nosound ();
#endif
}

static void play_note (int i, int freq, int vol)
{
	chn[i].phase = 0;
	chn[i].freq = freq;
	chn[i].vol = vol; 
	chn[i].env = 0x10000;
	chn[i].adsr = AGI_SOUND_ENV_ATTACK;

	/* Add chorus ;) */
	if (i < 3) {
		int newfreq = freq * 1007 / 1000;
		if (freq == newfreq)
			newfreq++;
		play_note (i + 4, newfreq, vol * 2 / 3);
	}
}

void play_agi_sound ()
{
	int i, freq;

	for (playing = i = 0; i < 4; i++) {
		playing |= !chn[i].end;

		if (chn[i].end)
			continue;

		if ((--chn[i].timer) <= 0) {
			stop_note (i);
			freq = ((chn[i].ptr->frq_0 & 0x3f) << 4)
				| (int)(chn[i].ptr->frq_1 & 0x0f);

			if (freq) {
				UINT8 v = chn[i].ptr->vol & 0x0f;
				play_note (i, freq * 10, v == 0xf ? 0 :
					0xff - (v << 1));
			}

			chn[i].timer = ((int)chn[i].ptr->dur_hi << 8) |
				chn[i].ptr->dur_lo;

			if (chn[i].timer == 0xffff) {
				chn[i].end = 1;
				chn[i].vol = 0;
				chn[i].env = 0;
				/* Chorus */
				if (i < 3) {
					chn[i + 4].vol = 0;
					chn[i + 4].env = 0;
				}
			}
			chn[i].ptr++;
		}
	}
}


void play_sound ()
{
	int i;

	if (endflag == -1)
		return;

	play_agi_sound ();

	if (!playing) {
		for (i = 0; i < NUM_CHANNELS; chn[i++].vol = 0);
		endflag = -1;
	}
}


UINT32 mix_sound (void)
{
	register int i, p;
	SINT16 *src;
	int c, b, m;

	memset (snd_buffer, 0, BUFFER_SIZE << 1);

	for (c = 0; c < NUM_CHANNELS; c++) {
		if (!chn[c].vol)
			continue;

		m = chn[c].flags & AGI_SOUND_ENVELOPE ?
			chn[c].vol * chn[c].env >> 16 :
			chn[c].vol;
	
		if (c != 3) {
			src = chn[c].ins;
	
			p = chn[c].phase;
			for (i = 0; i < BUFFER_SIZE; i++) {
				b = src[p >> 8];
				b += ((src[((p >> 8) + 1) % chn[c].size] -
					src[p >> 8]) * (p & 0xff)) >> 8;
				snd_buffer[i] += (b * m) >> 4;
	
				p += (UINT32)118600 * 4 / chn[c].freq;
	
				/* FIXME */
				if (chn[c].flags & AGI_SOUND_LOOP) {
					p %= chn[c].size << 8;
				} else {
					if (p >= chn[c].size << 8) {
						p = chn[c].vol = 0;
						chn[c].end = 1;
						break;
					}
				}
	
			}
			chn[c].phase = p;
		} else {
			/* Add white noise */
			for (i = 0; i < BUFFER_SIZE; i++) {
				b = rnd(256) - 128;
				snd_buffer[i] += (b * m) >> 4;
			}
		}

		switch (chn[c].adsr) {
		case AGI_SOUND_ENV_ATTACK:
			/* not implemented */
			chn[c].adsr = AGI_SOUND_ENV_DECAY;
			break;
		case AGI_SOUND_ENV_DECAY:
			if (chn[c].env > chn[c].vol * ENV_SUSTAIN + ENV_DECAY) {
				chn[c].env -= ENV_DECAY;
			} else {
				chn[c].env = chn[c].vol * ENV_SUSTAIN;
				chn[c].adsr = AGI_SOUND_ENV_SUSTAIN;
			}
			break;
		case AGI_SOUND_ENV_SUSTAIN:
			break;
		case AGI_SOUND_ENV_RELEASE:
			if (chn[c].env >= ENV_RELEASE) {
				chn[c].env -= ENV_RELEASE;
			} else {
				chn[c].env = 0;
			}
		}
	}

	return BUFFER_SIZE;
}


void play_song (char *s)
{
	FILE *f;
	struct stat st;
	int i;

	/* Open the AGI sound file and slurp it */
	if (!(f = fopen (s, "r"))) {
		fprintf (stderr, "Can't open %s\n", s);
		return;
	}
	fstat (fileno (f), &st);
	song = calloc (1, st.st_size);
	fread (song, 1, st.st_size, f);
	fclose (f);

	/* Initialize channel pointers */
	for (i = 0; i < NUM_CHANNELS; i++) {
		chn[i].flags = AGI_SOUND_LOOP;
		chn[i].flags |= AGI_SOUND_ENVELOPE;
		chn[i].adsr = AGI_SOUND_ENV_ATTACK;
		chn[i].ins = waveform;
		chn[i].size = WAVEFORM_SIZE;
		chn[i].ptr = (struct agi_note *)(song +
			(song[i << 1] | (song[(i << 1) + 1] << 8)));
		chn[i].timer = 0;
		chn[i].vol = 0;
		chn[i].end = 0;
	}
	
	fprintf (stderr, "Playing %s... ", s);

	for (endflag = playing = 1; playing; ) {
		sleep (1);
	}

	fprintf (stderr, "done\n");
	free (song);
}


int main (int argc, char **argv)
{
	int i;

	if (argc < 2)
		return 0;

	snd_buffer = calloc (2, BUFFER_SIZE);
	__init_sound ();

	if (snd->init (snd_buffer) != 0) {
		fprintf (stderr, "Can't initialize sound\n");
		exit (-1);
	}
	
	for (i = 1; i < argc; i++) {
		play_song (argv[i]);
	}

	snd->deinit ();

	return 0;
}

