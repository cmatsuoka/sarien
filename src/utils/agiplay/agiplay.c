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

#include <sys/soundcard.h>

#define BUFFER_SIZE 410
#define WAVEFORM_SIZE 64
#define USE_ENVELOPE
#define USE_INTERPOLATION
#define ENV_DECAY 32
#define ENV_SUSTAIN 160

struct agi_note {
	unsigned char dur_lo;
	unsigned char dur_hi;
	unsigned char frq_0;
	unsigned char frq_1;
	unsigned char vol;
};

struct channel_info {
	struct agi_note *ptr;
	int end;
	int freq;
	int phase;
	int vol;
	int env;
	int timer;
};


static int audio_fd;
static short *buffer;
static struct channel_info chn[4];


static int waveform[64] = {
       0,   8,  16,  24,  32,  40,  48,  56,
      64,  72,  80,  88,  96, 104, 112, 120,
     128, 136, 144, 152, 160, 168, 176, 184,
     192, 200, 208, 216, 224, 232, 240, 255,
       0,-248,-240,-232,-224,-216,-208,-200,
    -192,-184,-176,-168,-160,-152,-144,-136,
    -128,-120,-112,-104, -96, -88, -80, -72,
    -64, -56, -48, -40,  -32, -24, -16,  -8   	/* Ramp up */
};


int init_sound ()
{
	int i;

	/* Set sound device to 16 bit, 22 kHz mono */

	if ((audio_fd = open ("/dev/audio", O_WRONLY)) == -1)
		return -1;
	
	i = AFMT_S16_LE;
	ioctl (audio_fd, SNDCTL_DSP_SETFMT, &i);
	i = 0;
	ioctl (audio_fd, SNDCTL_DSP_STEREO, &i);
	i = 22000;
	ioctl (audio_fd, SNDCTL_DSP_SPEED, &i);

	buffer = calloc (2, 2048);

	return 0;
}


void close_sound ()
{
	free (buffer);
	close (audio_fd);
}


void mix_channels (int s)
{
	register int i, p;
	int c, b;

	memset (buffer, 0, s << 1);

	for (c = 0; c < 3; c++) {
		if (!chn[c].vol)
			continue;

		p = chn[c].phase;
		for (i = 0; i < s; i++) {

			b = waveform[p >> 8];
#ifdef USE_INTERPOLATION
			b += ((waveform[((p >> 8) + 1) % WAVEFORM_SIZE] -
				waveform[p >> 8]) * (p & 0xff)) >> 8;
#endif
#ifdef USE_ENVELOPE
			if (chn[c].env >> 4 > chn[c].vol * ENV_SUSTAIN)
				chn[c].env -= ENV_DECAY;
			buffer[i] += (b * (chn[c].vol * chn[c].env >> 20)) >> 8;
#else
			buffer[i] += (b * chn[c].vol) >> 8;
#endif
			p += 11860 * 4 / chn[c].freq;
			p %= WAVEFORM_SIZE << 8;
		}
		chn[c].phase = p;
	}

	for (i = 0; i < s; i++)
		buffer[i] <<= 5;
}


void dump_buffer (int i)
{
	i *= 2;
	for (; i -= write (audio_fd, buffer, i); );
}


void stop_note (int i)
{
	chn[i].vol = 0;
}


void play_note (int i, int freq, int vol)
{
	if (vol >= 255 || freq < 16 || freq >= 960)
		vol = 0;
	chn[i].freq = freq;
	chn[i].phase = 0;
	chn[i].vol = vol;
#ifdef USE_ENVELOPE                                                             
        chn[i].env = 0x100000;                                                  
#endif                                                                          

}


void play_song (char *s)
{
	FILE *f;
	struct stat st;
	unsigned char *song;
	int i, playing;

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
	for (i = 0; i < 4; i++) {
		chn[i].ptr = (struct agi_note *)(song +
			(song[i << 1] | (song[(i << 1) + 1] << 8)));
		chn[i].timer = 0;
		chn[i].end = 0;
	}
	
	fprintf (stderr, "Playing %s... ", s);

	for (playing = 1; playing; ) {
		int freq;

		for (playing = i = 0; i < 4; i++) {
			playing |= !chn[i].end;
			
			if (chn[i].end)
				continue;

			if ((--chn[i].timer) <= 0) {
				if (chn[i].ptr >= (struct agi_note *)song +
					st.st_size)
					break;
				stop_note (i);
				freq = ((chn[i].ptr->frq_0 & 0x3f) << 4)
					| (int)(chn[i].ptr->frq_1 & 0x0f);
				if (freq)
					play_note (i, freq, chn[i].ptr->vol);
				chn[i].timer = ((int)chn[i].ptr->dur_hi << 8) |
					chn[i].ptr->dur_lo;
				if (chn[i].timer == 0xffff)
					chn[i].end = 1;
				chn[i].ptr++;
			}
		}

		mix_channels (400);
		dump_buffer (400);
	}

	fprintf (stderr, "done\n");
	free (song);
}


int main (int argc, char **argv)
{
	int i;

	if (argc < 2)
		return 0;

	if (init_sound () != 0) {
		fprintf (stderr, "Can't initialize sound\n");
		exit (-1);
	}
	
	for (i = 1; i < argc; i++) {
		play_song (argv[i]);
	}

	close_sound ();

	return 0;
}
