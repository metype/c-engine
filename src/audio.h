#ifndef CPROJ_AUDIO_H
#define CPROJ_AUDIO_H

#include "SDL3_mixer/SDL_mixer.h"
#include "hashmap.h"

#define NUM_CHANNELS 16

typedef struct LoadedWAV {
    Mix_Chunk* wave;
} LoadedWAV;

typedef enum : uint16_t {
    CHANNEL_FLAG_LOOP          = 0b00001,
    CHANNEL_FLAG_FADE_OUT      = 0b00010,
    CHANNEL_FLAG_FADE_IN       = 0b00100,
    CHANNEL_DO_NOT_AUTO_SELECT = 0b01000,
} ChannelFlags;

typedef struct {
    ChannelFlags flags;
    uint32_t fade_in_ms;
    uint32_t fade_out_ms;
    uint8_t volume;
} channel_data;

void M_init();

bool M_load_audio(char* filePath, char* key);

bool M_play_audio(char* key, int channel);

void M_stop_audio(int channel);

bool M_is_audio_playing(int channel);

void M_free();

void M_set_channel_flags(int channel, uint16_t flags);

void M_set_channel_fade_in_ms(int channel, uint32_t time);

void M_set_channel_fade_out_ms(int channel, uint32_t time);

void M_set_channel_volume(int channel, uint8_t volume);

void M_set_channel_data(int channel, channel_data data);

void SDLCALL M_channel_complete_callback(int channel);

#endif //CPROJ_AUDIO_H
