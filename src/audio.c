#include <malloc.h>
#include <pthread.h>
#include "errors.h"
#include "audio.h"
#include "log.h"

hash_map* loaded_audio = {}; //LoadedWAV
char** last_played_on_each_channel = {}; //std::string
channel_data * channel_info = {}; //channel_data
bool* channels_stopped;
bool* in_use_channels;

int audio_rate;
int audio_channels;
SDL_AudioFormat audio_format;
SDL_AudioDeviceID deviceID;

pthread_mutex_t audio_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t audio_mutex_attr;

void M_init() {
    pthread_mutex_init(&audio_mutex, &audio_mutex_attr);
    pthread_mutex_lock(&audio_mutex);
    bool success = SDL_Init(SDL_INIT_AUDIO);
    if(!success) {
        printf("%s\n", SDL_GetError());
    }
    assert_err(SDL_Init(SDL_INIT_AUDIO), "SDL Failed to init!", SDL_GetError());

    audio_rate = MIX_DEFAULT_FREQUENCY;
    audio_format = MIX_DEFAULT_FORMAT;
    audio_channels = MIX_DEFAULT_CHANNELS;

    SDL_AudioSpec spec = {audio_format, audio_channels, audio_rate};

    int device_count;
    SDL_AudioDeviceID* devices = SDL_GetAudioPlaybackDevices(&device_count);

    if(devices == nullptr || device_count == 0) {
        printf("No audio devices found, aborting audio manager.");
        fflush(stdout);
        return;
    }

    if (!Mix_OpenAudio(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK , &spec)) {
        printf("Couldn't open audio: %s", SDL_GetError());
        fflush(stdout);
        return;
    } else {
        Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
        L_printf(LOG_LEVEL_INFO, "Opened audio at %d Hz %d bit%s %s\n", audio_rate,
                    (audio_format&0xFF),
                    (SDL_AUDIO_ISFLOAT(audio_format) ? " (float)" : ""),
                    (audio_channels > 2) ? "surround" :
                    (audio_channels > 1) ? "stereo" : "mono");
    }

    fflush(stdout);

    Mix_ChannelFinished(&M_channel_complete_callback);
    Mix_AllocateChannels(NUM_CHANNELS);

    loaded_audio = malloc(sizeof(hash_map));
    map_init(loaded_audio);

    in_use_channels = malloc(sizeof(bool) * NUM_CHANNELS);
    channels_stopped = malloc(sizeof(bool) * NUM_CHANNELS);
    channel_info = malloc(sizeof(channel_data) * NUM_CHANNELS);
    last_played_on_each_channel = malloc(sizeof(char*) * NUM_CHANNELS);

    for(int i = 0; i < NUM_CHANNELS; i++) {
        channel_info[i].flags = 0;
        last_played_on_each_channel[i] = malloc(sizeof(char) * 64);
    }

    pthread_mutex_unlock(&audio_mutex);

    for(int i = 0; i < NUM_CHANNELS; i++) {
        M_set_channel_volume(i, 100);
    }
}

bool M_load_audio(char* filePath, char* key) {
    pthread_mutex_lock(&audio_mutex);
    LoadedWAV* wavData = malloc(sizeof(LoadedWAV));
    wavData->wave = Mix_LoadWAV(filePath);
    if(wavData->wave == nullptr) {
        return false;
    }
    map_set(loaded_audio, key, wavData);
    pthread_mutex_unlock(&audio_mutex);
    return true;
}

bool M_play_audio(char* key, int channel) {
    pthread_mutex_lock(&audio_mutex);
    if (channel > NUM_CHANNELS) {
        printf("%s(%s, %i) called but there's only %i channels!!!", __func__, key, channel, NUM_CHANNELS);
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
            return false;
        }
    }
    LoadedWAV* wavData = map_get(loaded_audio, key, LoadedWAV*);
    if(!wavData) return false;
    if(!wavData->wave) return false;
    last_played_on_each_channel[channel] = key;
    in_use_channels[channel] = true;
    channels_stopped[channel] = false;
    if(channel_info[channel].flags & CHANNEL_FLAG_FADE_IN) {
        Mix_FadeInChannel(channel, wavData->wave, 0, (int)channel_info[channel].fade_in_ms);
    } else {
        if(Mix_PlayChannel(channel, wavData->wave, 0) == -1) {
            printf("gay sex");
        }
    }
    pthread_mutex_unlock(&audio_mutex);
    return true;
}

void M_stop_audio(int channel) {
    pthread_mutex_lock(&audio_mutex);
    if(channel > NUM_CHANNELS) {
        printf("%s(%i) called but there's only %i channels!!!", __func__, channel, NUM_CHANNELS);
        return;
    }
    if(channel_info[channel].flags & CHANNEL_FLAG_FADE_OUT) {
        Mix_FadeOutChannel(channel, (int)channel_info[channel].fade_out_ms);
    } else {
        Mix_ExpireChannel(channel, 1);
    }
    channels_stopped[channel] = true;
    pthread_mutex_unlock(&audio_mutex);
}

bool M_is_audio_playing(int channel) {
    if(channel > NUM_CHANNELS) {
        printf("%s(%i) called but there's only %i channels!!!", __func__, channel, NUM_CHANNELS);
        return false;
    }
    pthread_mutex_lock(&audio_mutex);
    bool value = !channels_stopped[channel];
    pthread_mutex_unlock(&audio_mutex);
    return value;
}

void M_set_channel_flags(int channel, uint16_t flags) {
    if(channel > NUM_CHANNELS) {
        printf("%s(%i, %i) called but there's only %i channels!!!", __func__, channel, flags, NUM_CHANNELS);
        return;
    }
    pthread_mutex_lock(&audio_mutex);
    channel_info[channel].flags = flags;
    pthread_mutex_unlock(&audio_mutex);
}

void M_set_channel_fade_in_ms(int channel, uint32_t time) {
    if(channel > NUM_CHANNELS) {
        printf("%s(%i, %i) called but there's only %i channels!!!", __func__, channel, time, NUM_CHANNELS);
        return;
    }
    pthread_mutex_lock(&audio_mutex);
    channel_info[channel].fade_in_ms = time;
    pthread_mutex_unlock(&audio_mutex);
}

void M_set_channel_fade_out_ms(int channel, uint32_t time) {
    if(channel > NUM_CHANNELS) {
        printf("%s(%i, %i) called but there's only %i channels!!!", __func__, channel, time, NUM_CHANNELS);
        return;
    }
    pthread_mutex_lock(&audio_mutex);
    channel_info[channel].fade_out_ms = time;
    pthread_mutex_unlock(&audio_mutex);
}

void M_set_channel_volume(int channel, uint8_t volume) {
    if(channel > NUM_CHANNELS) {
        printf("%s(%i, %i) called but there's only %i channels!!!", __func__, channel, volume, NUM_CHANNELS);
        return;
    }
    pthread_mutex_lock(&audio_mutex);
    channel_info[channel].volume = volume; //(int)(((float)volume / 100.0f) * MIX_MAX_VOLUME);
    Mix_Volume(channel, channel_info[channel].volume);
    pthread_mutex_unlock(&audio_mutex);
}

void M_set_channel_data(int channel, channel_data data) {
    if(channel > NUM_CHANNELS) {
        printf("%s(%i, channel_data) called but there's only %i channels!!!", __func__, channel, NUM_CHANNELS);
        return;
    }
    pthread_mutex_lock(&audio_mutex);
    channel_info[channel] = data;
    pthread_mutex_unlock(&audio_mutex);
    M_set_channel_volume(channel, data.volume);
}

void M_free() {
    Mix_CloseAudio();
    pthread_mutex_lock(&audio_mutex);
    for(int i = 0; i < loaded_audio->capacity; i++){
        if(!loaded_audio->arr[i]) continue;
        if(!loaded_audio->arr[i]->value) continue;
        if(!((LoadedWAV *)loaded_audio->arr[i]->value)->wave) continue;

        Mix_FreeChunk(((LoadedWAV *)loaded_audio->arr[i]->value)->wave);

        free(loaded_audio->arr[i]->value);
        free(loaded_audio->arr[i]);
    }
    free(loaded_audio->arr);
    free(loaded_audio);
    pthread_mutex_unlock(&audio_mutex);
    Mix_Quit();
}

void SDLCALL M_channel_complete_callback(int channel) {
    if(channel > NUM_CHANNELS) {
        printf("%s(%i) called but there's only %i channels!!!", __func__, channel, NUM_CHANNELS);
        return;
    }
    pthread_mutex_lock(&audio_mutex);
    in_use_channels[channel] = false;
    if((channel_info[channel].flags & CHANNEL_FLAG_LOOP) && !channels_stopped[channel]) {
        pthread_mutex_unlock(&audio_mutex);

        M_play_audio(last_played_on_each_channel[channel], channel);

        pthread_mutex_lock(&audio_mutex);
    }
    pthread_mutex_unlock(&audio_mutex);
}
