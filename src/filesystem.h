#ifndef CENGINE_FILESYSTEM_H
#define CENGINE_FILESYSTEM_H

#include "definitions.h"

#if CENGINE_LINUX
#include <bits/types/FILE.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#define FS_PATH_MAX PATH_MAX
#elif CENGINE_WIN32
#define FS_PATH_MAX MAX_PATH
#include "./win32_file_wrapper.h"
#include "win32_stdlib.h"
#endif

typedef enum file_access_flags {
    FA_FILE_READ = 0x1,
    FA_FILE_WRITE = 0x2,
    FA_FILE_TRUNCATE = 0x4,
    FA_FILE_APPEND = 0x8,
    FA_FILE_CREATE = 0x10,
    FA_FILE_ENSURE_CREATE = 0x20,
    FA_FILE_NO_FOLLOW_SYMLINK = 0x40,
} file_access_flags_e;

typedef struct file {
    FILE* f_ptr;
    char* parent_dir;
    char* file_name;
} file_s;

/* Opens a file with the specified name, placing a date string inside at %s
    Usage: file log_file = FS_open_dated("logs/%s.log", nullptr, FILE_WRITE);
    Arguments:
        const char* name -> The name of the file to be created.
        const char* date_fmt_str -> The date format string to use, null results in the function using "%Y-%j-%H-%M-%S" (YYYY-DDD-HH-MM-SS)
        file_access_flags_e access_flags -> The access flags to use for this file open operation.
*/
file_s FS_open_dated(const char* name, const char* date_fmt_str, file_access_flags_e access_flags);


file_s FS_open(const char* path, file_access_flags_e access_flags);
void FS_init();

char* FS_get_working_dir();

void FS_create_dir_if_not_exist(const char* name);

void FS_close(file_s file_to_close);

#endif //CENGINE_FILESYSTEM_H
