
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <CoreAudio/CoreAudio.h>

#include "sarien.h"
#include "sound.h"
#include "console.h"

int  osx_init_sound(SINT16 *buffer);
void osx_close_sound();

static struct sound_driver sound_osx =
{
    "MacOS X Native Sound Output",
    osx_init_sound,
    osx_close_sound,
};

void __init_sound()
{
    AudioStreamBasicDescription pDescription;

    snd = &sound_osx;
    pDescription.mSampleRate       = 22050;
    pDescription.mFormatID         = kAudioFormatLinearPCM;
    pDescription.mFormatFlags      = 0;
    pDescription.mChannelsPerFrame = 1;
    pDescription.mBitsPerChannel   = 16;
    
    pDescription.mBytesPerFrame    = 0;
    pDescription.mBytesPerPacket   = 0;
    pDescription.mFramesPerPacket  = 0;
}

int osx_init_sound(SINT16 *buffer)
{
    report("sound_osx: sound output disabled\n");
    return -1;
}

void osx_close_sound()
{
}
