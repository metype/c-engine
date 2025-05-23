#include "engine.h"

void thread_wait_loop() {
    bool done = false;
    thread_info* threads = E_get_threads();
    while(!done) {
        thread_info* cur_info = threads;
        bool all_stopped = true;
        do {
            switch (cur_info->status) {
                case THREAD_STOPPING:
                    cur_info->status = THREAD_STOPPED;
                case THREAD_RUNNING:
                case THREAD_STARTING:
                    all_stopped = false;
                    break;
            }
            cur_info = cur_info->next;
        } while (cur_info);
        if(all_stopped) done = true;
    }
}

int main() {
    E_spawn_thread(&E_tick);

    thread_wait_loop();
    return 0;
}