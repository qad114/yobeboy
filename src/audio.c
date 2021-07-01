#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include "common/bitwise.h"
#include "audio.h"
#include "constants.h"

#ifndef M_PI
    #define M_PI 3.14159265359f
#endif

void audioCallback(void* user_data, uint8_t* raw_buffer, int bytes);

void AUD_init(Audio* audio, int sampleRate) {
    SDL_Init(SDL_INIT_AUDIO);

    audio->sampleRate = sampleRate;
    for (int i = 0; i < 4; ++i) {
        AudioChannel channel;
        channel.frequency = 0.0;
        channel.amplitude = 4400.0;
        channel.time = 0.0;
        audio->channels[i] = channel;
    }

    SDL_AudioSpec want;
    want.freq = audio->sampleRate;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;
    want.callback = audioCallback;
    want.userdata = audio;

    SDL_AudioSpec have;
    //SDL_OpenAudio(&want, &have);
    audio->deviceId = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);

    SDL_PauseAudioDevice(audio->deviceId, false);
}

void AUD_destroy(Audio* audio) {
    SDL_PauseAudioDevice(audio->deviceId, true);
    SDL_AudioQuit();
    free(audio);
    audio = NULL;
}

void AUD_update(Audio* audio, Memory* mem) {
    unsigned int x;

    x = ((mem->logicalMemory[REG_NR14] & 0x7) << 8) | mem->logicalMemory[REG_NR13];
    audio->channels[0].frequency = 131072.0 / (2048.0 - x);
    if (audio->channels[0].frequency < 100.0) audio->channels[0].frequency = 0.0;

    x = ((mem->logicalMemory[REG_NR24] & 0x7) << 8) | mem->logicalMemory[REG_NR23];
    audio->channels[1].frequency = 131072.0 / (2048.0 - x);
    if (audio->channels[1].frequency < 100.0) audio->channels[1].frequency = 0.0;

    x = ((mem->logicalMemory[REG_NR34] & 0x7) << 8) | mem->logicalMemory[REG_NR33];
    audio->channels[2].frequency = getBit(mem->logicalMemory[REG_NR30], 7) ? (65536.0 / (2048.0 - x)) : 0.0;
    if (audio->channels[0].frequency < 100.0) audio->channels[0].frequency = 0.0;
}

void audioCallback(void *user_data, uint8_t *raw_buffer, int bytes) {
    Audio* audio = user_data;
    int16_t* snd = (int16_t*) raw_buffer;
	int len = bytes / sizeof(*snd);

    for (int i = 0; i < len; ++i) {
        snd[i] = 0;

        for (int j = 0; j < 3; ++j) {
            AudioChannel* channel = &(audio->channels[j]);
            double sine = sin(channel->time);
            double ampMultiplier = (sine > 0) - (sine < 0);
            snd[i] += (channel->amplitude * ampMultiplier) / 3;

            channel->time += channel->frequency * 2.0f * M_PI / audio->sampleRate;
            if (channel->time >= (2.0f * M_PI)) channel->time -= (2.0f * M_PI);
        }
    }
}