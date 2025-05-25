#include "test_actor.h"
#include "../audio.h"
#include "../log.h"
#include <stdio.h>

void test_actor_init(actor* self, app_state* state_ptr) {
    L_printf(LOG_LEVEL_DEBUG, "Test actor init.");
    M_set_channel_volume(0, 25);
    M_load_audio("/home/emily/CLionProjects/jump_sdl3/assets/audio/testwav.wav", "song1");
    M_play_audio("song1", 0);
}

void test_actor_think(actor* self, app_state* state_ptr) {
    L_printf(LOG_LEVEL_DEBUG, "Gay.\n");
}