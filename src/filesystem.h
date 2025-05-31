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

void FS_close(file_s file_to_close);

#endif //CENGINE_FILESYSTEM_H
