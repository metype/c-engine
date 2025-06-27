#include "util.h"

extern void Thread_sleep(float seconds); // NOLINT(*-redundant-declaration)
extern float Engine_ticks_to_seconds(int ticks); // NOLINT(*-redundant-declaration)
extern bool Input_GetKey(int keycode, const bool* keyArr, int numkeys); // NOLINT(*-redundant-declaration)
extern float Lerp(float a, float b, float t); // NOLINT(*-redundant-declaration)
extern int Min(int a, int b); // NOLINT(*-redundant-declaration)
extern int Max(int a, int b); // NOLINT(*-redundant-declaration)
extern float Minf(float a, float b); // NOLINT(*-redundant-declaration)
extern float Maxf(float a, float b); // NOLINT(*-redundant-declaration)