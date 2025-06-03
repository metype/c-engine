#include <stdlib.h>
#include <limits.h>
#include "math.h"
#include <math.h>

float2_s float2_rand(float x_max, float y_max) {
    return float2_rand_d(0, x_max, 0, y_max);
}

float2_s float2_rand_d(float x_min, float x_max, float y_min, float y_max) {
    return (float2_s){f_rand_d(x_min, x_max), f_rand_d(y_min, y_max)};
}

int i_rand(int max) {
    return i_rand_d(0, max);
}

int i_rand_d(int min, int max) {
    int gap = max - min;
    return (int)(random() % gap) + min;
}

float f_rand(float max) {
    return f_rand_d(0, max);
}

float f_rand_d(float min, float max) {
    float gap = max - min;
    int rand_i = i_rand(INT_MAX); // Get an int from 0-65535
    float rand_f = ((float)rand_i / (float)INT_MAX); // Then convert that to 0-1
    return (rand_f * gap) + min; // Then convert that to min-max
}

float3_s float3_rand(float x_max, float y_max, float z_max) {
    return float3_rand_d(0, x_max, 0, y_max, 0, z_max);
}

float3_s float3_rand_d(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max) {
    return (float3_s){f_rand_d(x_min, x_max), f_rand_d(y_min, y_max), f_rand_d(z_min, z_max)};
}

