#include <unistd.h>
#include "util.h"
#include "definitions.h"
#include "engine.h"


void Thread_sleep(float seconds) {
    usleep((unsigned) (seconds * 1000000));
}

float Engine_ticks_to_seconds(int ticks) {
    return (float)ticks * (1.0f / (float)Engine_get_tick_rate());
}