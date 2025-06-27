#ifndef CENGINE_WIN32_STDLIB_H
#define CENGINE_WIN32_STDLIB_H
#if CENGINE_WIN32

#define _CRT_RAND_S
#include <stdlib.h>

#include <heapapi.h>
#include <synchapi.h>
#include <sysinfoapi.h>
#include <processthreadsapi.h>
#include <_timeval.h>

#ifndef NULL
#define NULL const (void*)0
#endif

#ifndef nullptr
#define nullptr NULL
#endif

#ifndef __INT16_MAX__
#define __INT16_MAX__ 32767s
#endif

#define usleep(time) Sleep((DWORD)time/1000)
#define malloc W32_Malloc
#define free W32_Free
#define gettimeofday W32_GetTimeOfDay
#define exit W32_Exit
#define strdup _strdup
#define random W32_Rand
#define rand W32_Rand

#ifdef CENGINE_ACCURATE
#define srandom W32_SRandom
#else
// Don't actually do anything at all, Win32 random doesn't need a seed
// and cannot be seeded, so to save some time convert srandom calls to
// a nothing essentially, while also preventing the linter from doing
// any complaining.
#define srandom(arg) do {/* arg */} while(0)
#endif


static inline void* W32_Malloc(size_t size) {
    HANDLE heapHandle = GetProcessHeap();
    if(heapHandle == nullptr) {
        return nullptr;
    }
    return HeapAlloc(heapHandle, 0, size);
}

static inline void W32_Free(void* ptr) {
    HANDLE heapHandle = GetProcessHeap();
    if(heapHandle == nullptr) {
        return;
    }
    HeapFree(heapHandle, 0, ptr);
}

static inline void W32_GetTimeOfDay(struct timeval* tv, struct timezone* tz) {
    FILETIME fTime;
    GetSystemTimeAsFileTime(&fTime);
    long long ll_now = (LONGLONG)fTime.dwLowDateTime + ((LONGLONG)(fTime.dwHighDateTime) << 32LL);
    tv->tv_sec = (long)(ll_now / 1000000000l);
    tv->tv_usec = (long)(ll_now / 1000l);
}

static inline void W32_Exit(unsigned code) {
    ExitProcess(code);
}

static inline void W32_SRandom(time_t seed) {
    // We do nothing here lol
}

static inline int W32_Rand() {
    unsigned value;
    rand_s(&value);
    return (int)(value % RAND_MAX);
}

// Gonna just make this a macro???
//static inline void usleep(unsigned long time) {
//    Sleep((DWORD)(time / 1000));
//}

#endif
#endif //CENGINE_WIN32_STDLIB_H
