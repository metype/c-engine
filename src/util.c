#include <unistd.h>
#include "util.h"

void E_sleep(float seconds) {
    usleep((unsigned) (seconds * 1000000));
}