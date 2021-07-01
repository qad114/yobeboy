#ifndef AUDIO_H
#define AUDIO_H

typedef struct Audio Audio;
typedef struct AudioChannel AudioChannel;

#include "memory.h"

struct AudioChannel {
    double frequency;
    double amplitude;
    double time;
    int waveform;
};

struct Audio {
    int deviceId;
    int sampleRate;
    AudioChannel channels[4];
};

void AUD_init(Audio* audio, int sampleRate);
void AUD_destroy(Audio* audio);
void AUD_update(Audio* audio, Memory* mem);

#endif