#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef CPROJ_AUDIO_H
#define CPROJ_AUDIO_H

#include "SDL3_mixer/SDL_mixer.h"
#include "hashmap.h"

#define NUM_CHANNELS 16

typedef enum : uint16_t {
    CHANNEL_FLAG_LOOP          = 0b00001,
    CHANNEL_FLAG_FADE_OUT      = 0b00010,
    CHANNEL_FLAG_FADE_IN       = 0b00100,
    CHANNEL_DO_NOT_AUTO_SELECT = 0b01000,
} channel_flags_e;


typedef struct LoadedWAV {
    Mix_Chunk* wave;
} loaded_wav_s;

typedef struct {
    channel_flags_e flags;
    uint32_t fade_in_ms;
    uint32_t fade_out_ms;
    uint8_t volume;
} channel_data_s;

void Audio_init();

bool Audio_load(char* filePath, char* key);

bool Audio_play(char* key, int channel);

void Audio_stop(int channel);

bool Audio_is_playing(int channel);

void Audio_free();

void Audio_set_channel_flags(int channel, uint16_t flags);

void Audio_set_channel_fade_in_ms(int channel, uint32_t time);

void Audio_set_channel_fade_out_ms(int channel, uint32_t time);

void Audio_set_channel_volume(int channel, uint8_t volume);

void Audio_set_channel_data(int channel, channel_data_s data);

void SDLCALL Audio_channel_complete_callback(int channel);

#endif //CPROJ_AUDIO_H

#pragma clang diagnostic pop