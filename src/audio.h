#ifndef CPROJ_AUDIO_H
#define CPROJ_AUDIO_H

#define NUM_CHANNELS 16

struct LoadedWAV {
    Mix_Chunk* wave;
};

enum ChannelFlags : uint16_t {
    CHANNEL_FLAG_LOOP          = 0b00001,
    CHANNEL_FLAG_FADE_OUT      = 0b00010,
    CHANNEL_FLAG_FADE_IN       = 0b00100,
    CHANNEL_DO_NOT_AUTO_SELECT = 0b01000,
};

typedef struct {
    ChannelFlags flags;
    uint32_t fade_in_ms;
    uint32_t fade_out_ms;
    uint8_t volume;
} channel_data;

class audio_manager {
        public:
        static void init();

        static bool load_audio(const std::string& filePath, const std::string& key);

        static bool play_audio(int channel, const std::string& key);

        static bool play_audio(const std::string& key);

        static void stop_audio(int channel);

        static bool is_audio_playing(int channel);

        static void free();

        static void set_channel_flags(int channel, uint16_t flags);

        static void set_channel_fade_in_ms(int channel, uint32_t time);

        static void set_channel_fade_out_ms(int channel, uint32_t time);

        static void set_channel_volume(int channel, uint8_t volume);

        static void set_channel_data(int channel, channel_data data);

        static void SDLCALL channel_complete_callback (int channel);

        private:
        inline static std::unordered_map<std::string, LoadedWAV> loaded_audio = {};
        inline static std::unordered_map<int, std::string> last_played_on_each_channel = {};
        inline static std::unordered_map<int, channel_data> channel_info = {};
        inline static std::unordered_map<int, bool> channels_stopped;
        inline static std::vector<int> in_use_channels;


        inline static SDL_AudioDeviceID deviceID;
        inline static int audio_rate;
        inline static SDL_AudioFormat audio_format;
        inline static int audio_channels;
};
#endif //CPROJ_AUDIO_H
