#include <unistd.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include "filesystem.h"
#include "log.h"

char working_dir[PATH_MAX];

mode_t convert_fa_flags_to_open(file_access_flags access_flags) {
    mode_t open_flags = 0;
    bool has_set_read = false;
    for(int i = 0; i < 32; i++) {
        if(!(access_flags & 1 << i)) continue;
        switch (1 << i) {
            case FILE_READ:
                if (open_flags & O_WRONLY) {
                    open_flags ^= O_RDONLY;
                    open_flags |= O_RDWR;
                }
                has_set_read = true;
                break;
            case FILE_WRITE:
                open_flags |= O_WRONLY;
                if (open_flags & O_RDWR) open_flags ^= O_WRONLY;
                if (has_set_read) {
                    open_flags ^= O_WRONLY;
                    open_flags |= O_RDWR;
                }
                break;
            case FILE_CREATE:
                open_flags |= O_CREAT;
                break;
            case FILE_ENSURE_CREATE:
                open_flags |= O_EXCL;
                break;
            case FILE_APPEND:
                open_flags |= O_APPEND;
                break;
            case FILE_TRUNCATE:
                open_flags |= O_TRUNC;
                break;
            case FILE_NO_FOLLOW_SYMLINK:
                open_flags |= O_NOFOLLOW;
                break;
        }
    }
    return open_flags;
}

char* convert_fa_flags_to_fopen(file_access_flags access_flags) {
    char* flags;
    if(access_flags & FILE_WRITE) {
        flags = "w";
        if(access_flags & FILE_APPEND) {
            flags = "a";
        }
    }
    if(access_flags & FILE_READ) {
        flags = "r";
        if(access_flags & FILE_WRITE) {
            flags = "r+";
            if(access_flags & FILE_APPEND) {
                flags = "a+";
            }
        }
    }
    return flags;
}

file F_open(const char* path, file_access_flags access_flags) {
    file f;
    FILE* file = fopen(path, convert_fa_flags_to_fopen(access_flags));
    f.f_ptr = file;
    f.file_name = path;
    char* parent_dir = malloc(sizeof(path));
    strcpy(parent_dir, path);
    for(int i = (int)strlen(path) - 1; i > 0; i--) {
#ifdef __WIN32__
        if(path[i] == '/' || path[i] == '\\') {
#else
        if(path[i] == '/') {
#endif
            parent_dir[i] = 0;
            break;
        }
    }
    f.parent_dir = parent_dir;
    return f;
}

void F_init() {
    if(getcwd(working_dir, sizeof(working_dir)) == nullptr) {
        L_print(LOG_LEVEL_FATAL, "Cannot get current working directory!");
        exit(-1);
    }
}

char* F_get_working_dir() {
    char* buf = malloc(sizeof(char) * PATH_MAX);
    snprintf(buf, PATH_MAX, "%s", working_dir);
    return buf;
}