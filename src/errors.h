#ifndef CPROJ_ERRORS_H
#define CPROJ_ERRORS_H
#include <stdio.h>

#define assert(cond, msg) assert_err(cond, msg, "");

#define assert_err(cond, msg, err_str) do{                                                                                          \
    if( !(cond) ) {                                                                                                                 \
        if(!DEBUG) {break;}                                                                                                         \
        printf("Assert %s failed! (%s) @ %s:%d in %s()!\nErrStr: \"%s\"", #msg, #cond, __FILE__, __LINE__, __FUNCTION__, err_str);  \
        fflush(stdout);                                                                                                             \
        __builtin_trap();                                                                                                           \
}} while(0)

#endif //CPROJ_ERRORS_H
