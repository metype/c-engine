#ifndef CPROJ_UTIL_H
#define CPROJ_UTIL_H

#include <unistd.h>
#include "engine.h"

/**
 *  Pauses the current thread for the specified
 *  number of seconds.
 *
 * \param seconds The amount of seconds to sleep
 *
 * \returns void
 *
 */
inline void Thread_sleep(float seconds) {
    usleep((unsigned) (seconds * 1000000));
};

/**
 *  Converts a tickcount to second count.
 *
 * \param ticks The number of ticks to
 * convert to seconds
 *
 * \returns The number of seconds the tick count
 * refers to at the current tickrate
 *
 */
inline float Engine_ticks_to_seconds(int ticks) {
    return (float)ticks * (1.0f / (float)Engine_get_tick_rate());
}

/**
 *  Performs a linear interpolation between a and b, with respect to t.
 *
 *  Uses the algorithm: a + t * (b - a)
 *
 * \param a The first value
 * \param b The second value
 * \param t The interpolation value, expected to be between 0 and 1
 *
 * \returns The result of performing a linear interpolation between a and b
 * with respect to t.
 *
 */
inline float Lerp(float a, float b, float t) {
    return a + t * (b - a);
}

/**
 *  Return the smaller of two ints
 *
 * \param a The int float to compare
 * \param b The int float to compare
 *
 * \returns The smaller number between the two.
 *
 * If a==b; returns a
 */
inline int Min(int a, int b) {
    if(a > b) return b;
    return a;
}

/**
 *  Return the larger of two ints
 *
 * \param a The first int to compare
 * \param b The second int to compare
 *
 * \returns The larger number between the two.
 *
 * If a==b; returns a
 */
inline int Max(int a, int b) {
    if(a < b) return b;
    return a;
}

/**
 *  Return the smaller of two floats
 *
 * \param a The first float to compare
 * \param b The second float to compare
 *
 * \returns The smaller number between the two.
 *
 * If a==b; returns a
 */
inline float Minf(float a, float b) {
    if(a > b) return b;
    return a;
}

/**
 *  Return the larger of two floats
 *
 * \param a The first float to compare
 * \param b The second float to compare
 *
 * \returns The larger number between the two.
 *
 * If a==b; returns a
 */
inline float Maxf(float a, float b) {
    if(a < b) return b;
    return a;
}

/**
 *  Takes a bool array of keys currently pressed
 *  and checks if a certain scancode is active
 *
 *  This is a small helper function only meant to
 *  help with performing checks.
 *
 * \param scancode The scancode to check for
 * \param keyArr The key array
 * \param numkeys The length of the key array
 *
 * \returns True if the scancode is pressed.
 * False if the scancode is not true in the
 * array, if the array is NULL, or if
 * the scancode's raw value is greater than
 * the array's length.
 *
 */
inline bool Input_GetKey(int scancode, const bool* keyArr, int numkeys) {
    if(scancode > numkeys) return false;
    if(!keyArr) return false;
    return keyArr[scancode];
}

#endif //CPROJ_UTIL_H
