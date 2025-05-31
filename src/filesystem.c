#include <unistd.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "filesystem.h"
#include "log.h"
#include "string.h"
#include <errno.h>

char working_dir[PATH_MAX];

__attribute__((unused)) mode_t convert_fa_flags_to_open(file_access_flags_e access_flags) {
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

char* convert_fa_flags_to_fopen(file_access_flags_e access_flags) {
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

void make_directory(char *path) { // NOLINT(*-no-recursion)
    if(strlen(path) == 0) return;
    char *sep = strrchr(path, '/');
    if(sep != NULL) {
        *sep = 0;
        make_directory(path);
        *sep = '/';
    }
    if(mkdir(path, 0777) < 0 && errno != EEXIST) {
        Log_printf(LOG_LEVEL_ERROR, "Failed to create one or more directories for path '%s'", path);
    }
}

FILE *fopen_mkdir(const char *path, const char *mode) {
    char *sep = strrchr(path, '/');
    if(sep) {
        char *path0 = strdup(path);
        path0[ sep - path ] = 0;
        make_directory(path0);
        free(path0);
    }
    return fopen(path,mode);
}


file_s FS_open(const char* path, file_access_flags_e access_flags) {
    file_s f;
    FILE* file = fopen_mkdir(path, convert_fa_flags_to_fopen(access_flags));
    f.f_ptr = file;
    f.file_name = strdup(path);
    char* parent_dir = strdup(path);

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

file_s FS_open_dated(const char* name, const char* date_fmt_str, file_access_flags_e access_flags) {
    string* cwd = sc(FS_get_working_dir());

    struct stat st = {0};

    if (stat(cwd->c_str, &st) == -1) {
        mkdir(cwd->c_str, 0700);
    }

    const int date_str_len = 20;

    time_t current_time = time(nullptr);
    char* date_string = malloc(sizeof(char) * date_str_len);
    strftime(date_string, date_str_len, (date_fmt_str) ? date_fmt_str : "%Y-%j-%H-%M-%S", localtime(&current_time));

    char* file_name = malloc(sizeof(char) * 1024);

    snprintf(file_name, 1024, name, date_string);

    s_cat(cwd, sco(file_name));

    file_s opened_file = FS_open(cwd->c_str, access_flags);

    ref_dec(&cwd->refcount);

    free(date_string);
    return opened_file;
}

void FS_init() {
    if(getcwd(working_dir, sizeof(working_dir)) == nullptr) {
        Log_print(LOG_LEVEL_FATAL, "Cannot get current working directory!");
        exit(-1);
    }
}

char* FS_get_working_dir() {
    char* buf = malloc(sizeof(char) * PATH_MAX);
    snprintf(buf, PATH_MAX, "%s", working_dir);
    return buf;
}

void FS_close(file_s file_to_close) {
    fflush(file_to_close.f_ptr);
    fclose(file_to_close.f_ptr);

    free(file_to_close.file_name);
    free(file_to_close.parent_dir);
}