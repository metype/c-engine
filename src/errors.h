#ifndef CPROJ_ERRORS_H
#define CPROJ_ERRORS_H
#include <stdio.h>
#include "definitions.h"

#define assert(cond, msg, handle) assert_err(cond, msg, "", handle)

#if CENGINE_DEBUG == 1
#if CENGINE_LINUX
    #define assert_err(cond, msg, err_str, handle) do{                                                                                                              \
        if( !(cond) ) {                                                                                                                                     \
            Log_printf(LOG_LEVEL_ERROR, "Assert %s failed! (%s) @ %s:%d in %s()!\nErrStr: \"%s\"", #msg, #cond, __FILE__, __LINE__, __FUNCTION__, err_str); \
            fflush(stdout);                                                                                                                                 \
            __builtin_trap();                                                                                                                                       \
            handle;                                                                                                                                                        \
    }} while(0)
#elif CENGINE_WIN32
#define assert_err(cond, msg, err_str, handle) do{                                                                                                                  \
        if( !(cond) ) {                                                                                                                                     \
            Log_printf(LOG_LEVEL_ERROR, "Assert %s failed! (%s) @ %s:%d in %s()!\nErrStr: \"%s\"", #msg, #cond, __FILE__, __LINE__, __FUNCTION__, err_str); \
            fflush(stdout);                                                                                                                                 \
            __debugbreak();                                                                                                                                         \
            handle;                                                                                                                                                        \
    }} while(0)
#endif
#else
#define assert_err(cond, msg, err_str, handle) do{                                                                                                                  \
        if( !(cond) ) {                                                                                                                                     \
            Log_printf(LOG_LEVEL_ERROR, "Assert %s failed! (%s) @ %s:%d in %s()!\nErrStr: \"%s\"", #msg, #cond, __FILE__, __LINE__, __FUNCTION__, err_str); \
            handle;                                                                                                                                         \
    }} while(0)
#endif

#endif //CPROJ_ERRORS_H
