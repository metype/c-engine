#include "test_actor.h"
#include "../audio.h"
#include <stdio.h>

void test_actor_think(actor* self, app_state* state_ptr) {
    if(self->ticks_since_spawn == 0) {
        audio_load_audio("/home/emily/CLionProjects/jump_sdl3/assets/audio/testwav.wav", "song1");
        audio_play_audio("song1", -1);
    }
    printf("Gay.\n");
}