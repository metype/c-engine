#ifndef CENGINE_FILESYSTEM_H
#define CENGINE_FILESYSTEM_H

#include <bits/types/FILE.h>

typedef enum file_access_flags {
    FILE_READ = 0x1,
    FILE_WRITE = 0x2,
    FILE_TRUNCATE = 0x4,
    FILE_APPEND = 0x8,
    FILE_CREATE = 0x10,
    FILE_ENSURE_CREATE = 0x20,
    FILE_NO_FOLLOW_SYMLINK = 0x40,
} file_access_flags;

typedef struct file {
    FILE* f_ptr;
    const char* parent_dir;
    const char* file_name;
} file;

file F_open(const char* path, file_access_flags access_flags);
void F_init();
char* F_get_working_dir();

#endif //CENGINE_FILESYSTEM_H
