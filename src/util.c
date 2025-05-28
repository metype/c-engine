#include <unistd.h>
#include "util.h"
#include "definitions.h"


void E_sleep(float seconds) {
    usleep((unsigned) (seconds * 1000000));
}

float E_ticks_to_seconds(int ticks) {
    return (float)ticks * (1.0f / TICK_RATE);
}