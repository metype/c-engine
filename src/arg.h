#ifndef ARG_H
#define ARG_H
typedef struct argument {
    char short_opt;
    const char* long_opt;
    bool required;
    int* val;
} argument;

#define ARG_DEF(arg_list, arg_list_idx, arg_list_len, long_arg, short_arg, req, value_ptr) do { \
    if((arg_list_idx) >= (arg_list_len)) {                                                      \
        arg_list_len *= 2;                                                                      \
        void* new_arg_list = realloc(arg_list, sizeof(struct argument) * (arg_list_len));         \
        assert(new_arg_list != nullptr, "Enough memory to allocate argument list.");            \
        arg_list = new_arg_list;                                                                \
    }                                                                                           \
    arg_list[arg_list_idx++] = (argument){short_arg, long_arg, req, value_ptr};                 \
} while(0)

#endif //ARG_H
