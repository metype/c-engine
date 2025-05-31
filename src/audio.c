#include <malloc.h>
#include <pthread.h>
#include "errors.h"
#include "audio.h"
#include "log.h"
#include "definitions.h"

hash_map_s* loaded_audio = {};
char** last_played_on_each_channel = {};
channel_data_s* channel_info = {};
bool* channels_stopped;

bool has_audio_not_initialized = true;

pthread_mutex_t audio_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t audio_mutex_attr;

void Audio_init() {
    pthread_mutex_init(&audio_mutex, &audio_mutex_attr);
    mutex_locked_code(&audio_mutex, {
        bool success = SDL_Init(SDL_INIT_AUDIO);
        if (!success) {
            Log_printf(LOG_LEVEL_ERROR, "%s\n", SDL_GetError());
        }
        assert_err(SDL_Init(SDL_INIT_AUDIO), "SDL Failed to init!", SDL_GetError());

        int audio_rate = MIX_DEFAULT_FREQUENCY;
        SDL_AudioFormat audio_format = MIX_DEFAULT_FORMAT;
        int audio_channels = MIX_DEFAULT_CHANNELS;

        SDL_AudioSpec spec;
        spec.format = audio_format;
        spec.channels = audio_channels;
        spec.freq = audio_rate;

        int device_count;
        SDL_AudioDeviceID *devices = SDL_GetAudioPlaybackDevices(&device_count);

        if (devices == nullptr || device_count == 0) {
            Log_print(LOG_LEVEL_ERROR, "No audio devices found, aborting audio manager.");
            if(err == 0) pthread_mutex_unlock(&audio_mutex);
            return;
        }

        if (!Mix_OpenAudio(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec)) {
            Log_printf(LOG_LEVEL_ERROR, "Couldn't open audio: %s", SDL_GetError());
            if(err == 0) pthread_mutex_unlock(&audio_mutex);
            return;
        } else {
            Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
            Log_printf(LOG_LEVEL_INFO, "Audio initialized! %d Hz %d bit%s %s\n", audio_rate,
                       (audio_format & 0xFF),
                       (SDL_AUDIO_ISFLOAT(audio_format) ? " (float)" : ""),
                       (audio_channels > 2) ? "surround" :
                       (audio_channels > 1) ? "stereo" : "mono");

        }

        fflush(stdout);

        Mix_ChannelFinished(&Audio_channel_complete_callback);
        Mix_AllocateChannels(NUM_CHANNELS);

        loaded_audio = malloc(sizeof(hash_map_s));
        Map_init(loaded_audio);

        channels_stopped = malloc(sizeof(bool) * NUM_CHANNELS);
        channel_info = malloc(sizeof(channel_data_s) * NUM_CHANNELS);
        last_played_on_each_channel = malloc(sizeof(char *) * NUM_CHANNELS);

        for (int i = 0; i < NUM_CHANNELS; i++) {
            channel_info[i].flags = 0;
            last_played_on_each_channel[i] = malloc(sizeof(char) * 64);
            Audio_set_channel_volume(i, 100);
        }

        has_audio_not_initialized = false;

    });
}

bool Audio_load(char* filePath, char* key) {
    mutex_locked_code(&audio_mutex, {
        if (has_audio_not_initialized) {
            Log_printf(LOG_LEVEL_ERROR,
                       "Cannot load audio file \"%s\" into key \"%s\", audio system failed to initialize!",
                       filePath, key);
            pthread_mutex_unlock(&audio_mutex);
            return false;
        }
        loaded_wav_s *wavData = malloc(sizeof(loaded_wav_s));
        wavData->wave = Mix_LoadWAV(filePath);
        if (wavData->wave == nullptr) {
            if(err == 0) pthread_mutex_unlock(&audio_mutex);
            return false;
        }
        map_set(loaded_audio, key, wavData);
    });
    return true;
}

bool Audio_play(char* key, int channel) {
    mutex_locked_code(&audio_mutex, {
        if (has_audio_not_initialized) {
            Log_printf(LOG_LEVEL_ERROR, "Cannot play audio key \"%s\", audio system failed to initialize!", key);
            if(err == 0) pthread_mutex_unlock(&audio_mutex);
            return false;
        }
        if (channel > NUM_CHANNELS) {
            Log_printf(LOG_LEVEL_ERROR, "%s(%s, %i) called but there's only %i channels!!!", __func__, key, channel,
                       NUM_CHANNELS);
            if(err == 0) pthread_mutex_unlock(&audio_mutex);
            return false;
        }
        if (channel == -1) {
            for (int i = 0; i < NUM_CHANNELS; i++) {
                if (!Mix_Playing(i) && !(channel_info[i].flags & CHANNEL_DO_NOT_AUTO_SELECT)) {
                    channel = i;
                    break;
                }
            }
            if (channel == -1) {
                if(err == 0) pthread_mutex_unlock(&audio_mutex);
                return false;
            }
        }
        loaded_wav_s *wavData = map_get(loaded_audio, key, loaded_wav_s *);
        if (!wavData) {
            if(err == 0) pthread_mutex_unlock(&audio_mutex);
            return false;
        }
        if (!wavData->wave) {
            if(err == 0) pthread_mutex_unlock(&audio_mutex);
            return false;
        }
        last_played_on_each_channel[channel] = key;
        channels_stopped[channel] = false;
        if (channel_info[channel].flags & CHANNEL_FLAG_FADE_IN) {
            Mix_FadeInChannel(channel, wavData->wave, 0, (int) channel_info[channel].fade_in_ms);
        } else {
            if (Mix_PlayChannel(channel, wavData->wave, 0) == -1) {
                Log_printf(LOG_LEVEL_INFO, "Failed to play key \"%s\" on channel %i! %s", key, channel, SDL_GetError());
            }
        }
    });
    return true;
}

void Audio_stop(int channel) {
    mutex_locked_code(&audio_mutex, {
        if (has_audio_not_initialized) {
            Log_printf(LOG_LEVEL_ERROR, "Cannot stop audio on channel %i, audio system failed to initialize!", channel);
            if (err == 0) pthread_mutex_unlock(&audio_mutex);
            return;
        }
        if (channel > NUM_CHANNELS) {
            Log_printf(LOG_LEVEL_ERROR, "%s(%i) called but there's only %i channels!!!", __func__, channel,
                       NUM_CHANNELS);
            if(err == 0) pthread_mutex_unlock(&audio_mutex);
            return;
        }
        if (channel_info[channel].flags & CHANNEL_FLAG_FADE_OUT) {
            Mix_FadeOutChannel(channel, (int) channel_info[channel].fade_out_ms);
        } else {
            Mix_ExpireChannel(channel, 1);
        }
        channels_stopped[channel] = true;
    });
}

bool Audio_is_playing(int channel) {
    if(channel > NUM_CHANNELS) {
        Log_printf(LOG_LEVEL_ERROR, "%s(%i) called but there's only %i channels!!!", __func__, channel, NUM_CHANNELS);
        return false;
    }
    bool value = false;
    mutex_locked_code(&audio_mutex, {
        if (!has_audio_not_initialized) {
            value = !channels_stopped[channel];
        }
    });
    return value;
}

void Audio_set_channel_flags(int channel, uint16_t flags) {
    if(channel > NUM_CHANNELS) {
        Log_printf(LOG_LEVEL_ERROR, "%s(%i, %i) called but there's only %i channels!!!", __func__, channel, flags,
                   NUM_CHANNELS);
        return;
    }
    mutex_locked_code(&audio_mutex, {
        if (has_audio_not_initialized) {
            Log_printf(LOG_LEVEL_ERROR, "Cannot set channel flags on channel %i, audio system failed to initialize!",
                       channel);
        } else {
            channel_info[channel].flags = flags;
        }
    });
}

void Audio_set_channel_fade_in_ms(int channel, uint32_t time) {
    if(channel > NUM_CHANNELS) {
        Log_printf(LOG_LEVEL_ERROR, "%s(%i, %i) called but there's only %i channels!!!", __func__, channel, time,
                   NUM_CHANNELS);
        return;
    }
    mutex_locked_code(&audio_mutex, {
        if (has_audio_not_initialized) {
            Log_printf(LOG_LEVEL_ERROR,
                       "Cannot set channel fade in time on channel %i, audio system failed to initialize!",
                       channel);
        } else {
            channel_info[channel].fade_in_ms = time;
        }
    });
}

void Audio_set_channel_fade_out_ms(int channel, uint32_t time) {
    if(channel > NUM_CHANNELS) {
        Log_printf(LOG_LEVEL_ERROR, "%s(%i, %i) called but there's only %i channels!!!", __func__, channel, time,
                   NUM_CHANNELS);
        return;
    }
    mutex_locked_code(&audio_mutex, {
        if (has_audio_not_initialized) {
            Log_printf(LOG_LEVEL_ERROR,
                       "Cannot set channel fade out time on channel %i, audio system failed to initialize!",
                       channel);
        } else {
            channel_info[channel].fade_out_ms = time;
        }
    });
}

void Audio_set_channel_volume(int channel, uint8_t volume) {
    if(channel > NUM_CHANNELS) {
        Log_printf(LOG_LEVEL_ERROR, "%s(%i, %i) called but there's only %i channels!!!", __func__, channel, volume,
                   NUM_CHANNELS);
        return;
    }
    mutex_locked_code(&audio_mutex, {
        if (has_audio_not_initialized) {
            Log_printf(LOG_LEVEL_ERROR, "Cannot set channel volume on channel %i, audio system failed to initialize!",
                       channel);
        } else {
            channel_info[channel].volume = volume; //(int)(((float)volume / 100.0f) * MIX_MAX_VOLUME);
            Mix_Volume(channel, channel_info[channel].volume);
        }
    });
}

void Audio_set_channel_data(int channel, channel_data_s data) {
    if (channel > NUM_CHANNELS) {
        Log_printf(LOG_LEVEL_ERROR, "%s(%i, channel_data_s) called but there's only %i channels!!!", __func__, channel,
                   NUM_CHANNELS);
        return;
    }
    mutex_locked_code(&audio_mutex, {
        if (has_audio_not_initialized) {
            Log_printf(LOG_LEVEL_ERROR, "Cannot set channel data on channel %i, audio system failed to initialize!",
                       channel);
        } else {
            channel_info[channel] = data;
            Audio_set_channel_volume(channel, data.volume);
        }
    });
}

void Audio_free() {
    Mix_CloseAudio();
    mutex_locked_code(&audio_mutex, {
        if (has_audio_not_initialized) {
            Mix_Quit();
            if(err == 0) pthread_mutex_unlock(&audio_mutex);
            return;
        }
        for (int i = 0; i < loaded_audio->capacity; i++) {
            if (!loaded_audio->arr[i]) continue;
            if (!loaded_audio->arr[i]->value) continue;
            if (!((loaded_wav_s *) loaded_audio->arr[i]->value)->wave) continue;

            Mix_FreeChunk(((loaded_wav_s *) loaded_audio->arr[i]->value)->wave);

            free(loaded_audio->arr[i]->value);
            free(loaded_audio->arr[i]);
        }
        free(loaded_audio->arr);
        free(loaded_audio);
    });
    Mix_Quit();
}

void SDLCALL Audio_channel_complete_callback(int channel) {
    if(channel > NUM_CHANNELS) {
        Log_printf(LOG_LEVEL_ERROR, "%s(%i) called but there's only %i channels!!!", __func__, channel, NUM_CHANNELS);
        return;
    }
    mutex_locked_code(&audio_mutex, {
        if ((channel_info[channel].flags & CHANNEL_FLAG_LOOP) && !channels_stopped[channel]) {
            Audio_play(last_played_on_each_channel[channel], channel);
        }
    });
}
