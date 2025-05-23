#ifndef CPROJ_ERRORS_H
#define CPROJ_ERRORS_H
#include <stdio.h>

#define assert(cond, msg) do{\
    if(!DEBUG) {break;}\
    if( !(cond) ) {\
        printf("Assert %s failed! (%s) @ %s:%d in %s()", #msg, #cond, __FILE__, __LINE__, __FUNCTION__);\
        fflush(stdout);\
        __builtin_trap();\
}} while(0)

#endif //CPROJ_ERRORS_H
