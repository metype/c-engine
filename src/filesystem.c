#include "filesystem.h"
#include "log.h"
#include "string.h"
#include "definitions.h"

#if CENGINE_LINUX
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <linux/limits.h>
#include <errno.h>
#elif CENGINE_WIN32
#include <fileapi.h>
#include <errhandlingapi.h>
#include <winerror.h>
#include <processenv.h>
#include "win32_stdlib.h"
#endif

static char working_dir[FS_PATH_MAX];

#ifdef __UNIX
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
    return 0;
}
#endif

char* convert_fa_flags_to_fopen(file_access_flags_e access_flags) {
    char* flags = malloc(sizeof(char) * 3);
    if(access_flags & FA_FILE_WRITE) {
        sprintf(flags, "w");
        if(access_flags & FA_FILE_APPEND) {
            sprintf(flags, "a");
        }
    }
    if(access_flags & FA_FILE_READ) {
        sprintf(flags, "r");
        if(access_flags & FA_FILE_WRITE) {
            sprintf(flags, "r+");
            if(access_flags & FA_FILE_APPEND) {
                sprintf(flags, "a+");
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
#if CENGINE_LINUX
    if(mkdir(path, 0777) < 0 && errno != EEXIST) {
#elif CENGINE_WIN32
        if(!CreateDirectoryA(path, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
#endif
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
#if CENGINE_LINUX
    return fopen(path,mode);
#elif CENGINE_WIN32
    FILE* f = malloc(sizeof(FILE));
    fopen_s(&f, path, mode);
    return f;
#endif
}

void FS_create_dir_if_not_exist(const char* name) {
#if CENGINE_LINUX
    struct stat st = {0};

    if (stat(name, &st) == -1) {
        mkdir(name, 0700);
    }
#elif CENGINE_WIN32
    DWORD attribs = GetFileAttributesA(name);
    if(attribs == INVALID_FILE_ATTRIBUTES || (attribs & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        CreateDirectoryA(name, nullptr);
    }
#endif
}


file_s FS_open(const char* path, file_access_flags_e access_flags) {
    file_s f;
    char* mode = convert_fa_flags_to_fopen(access_flags);
    FILE* file = fopen_mkdir(path, mode);
    free(mode);
    f.f_ptr = file;
    f.file_name = strdup(path);
    char* parent_dir = strdup(path);

    for(int i = (int)strlen(path) - 1; i > 0; i--) {
#if CENGINE_WIN32
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

    FS_create_dir_if_not_exist(cwd->c_str);

    const int date_str_len = 20;

    time_t current_time = time(nullptr);
    char* date_string = malloc(sizeof(char) * date_str_len);
#if CENGINE_LINUX
    strftime(date_string, date_str_len, (date_fmt_str) ? date_fmt_str : "%Y-%j-%H-%M-%S", localtime(&current_time));
#elif CENGINE_WIN32
    struct tm time_struct;
    localtime_s(&time_struct, &current_time);
    strftime(date_string, date_str_len, (date_fmt_str) ? date_fmt_str : "%Y-%j-%H-%M-%S", &time_struct);
#endif

    char* file_name = malloc(sizeof(char) * 1024);

    snprintf(file_name, 1024, name, date_string);

    s_cat(cwd, sco(file_name));

    file_s opened_file = FS_open(cwd->c_str, access_flags);

    ref_dec(&cwd->refcount);

    free(date_string);
    return opened_file;
}

void FS_init() {
#if CENGINE_LINUX
    if(getcwd(working_dir, sizeof(working_dir)) == nullptr) {
        Log_print(LOG_LEVEL_FATAL, "Cannot get current working directory!");
        exit(-1);
    }
#elif CENGINE_WIN32
    if(GetCurrentDirectory(sizeof(working_dir), working_dir)) {
        Log_print(LOG_LEVEL_FATAL, "Cannot get current working directory!");
        W32_Exit(-1);
    }
#endif
}

char* FS_get_working_dir() {
    char* buf = malloc(sizeof(char) * FS_PATH_MAX);
    snprintf(buf, FS_PATH_MAX, "%s", working_dir);
    return buf;
}

void FS_close(file_s file_to_close) {
    fflush(file_to_close.f_ptr);
    fclose(file_to_close.f_ptr);

    free(file_to_close.file_name);
    free(file_to_close.parent_dir);
}