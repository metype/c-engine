#ifndef CPROJ_ERRORS_H
#define CPROJ_ERRORS_H
#include <stdio.h>

#define assert(cond, msg) assert_err(cond, msg, "")

#if DEBUG == 1
    #define assert_err(cond, msg, err_str) do{                                                                                                              \
        if( !(cond) ) {                                                                                                                                     \
            Log_printf(LOG_LEVEL_ERROR, "Assert %s failed! (%s) @ %s:%d in %s()!\nErrStr: \"%s\"", #msg, #cond, __FILE__, __LINE__, __FUNCTION__, err_str); \
            fflush(stdout);                                                                                                                                 \
            __builtin_trap();                                                                                                                               \
    }} while(0)
#else
    #define assert_err(cond, msg, err_str) do {cond;} while(0)
#endif

#endif //CPROJ_ERRORS_H
