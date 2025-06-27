#ifndef CENGINE_MATH_H
#define CENGINE_MATH_H

#include <stdbool.h>

typedef struct float2_s {
    float x;
    float y;
} float2_s;

typedef struct float3_s {
    float x;
    float y;
    float z;
} float3_s;

// Not inlining random functions cause honestly if you want speed you aren't calling random often ;3
int i_rand(int max);
int i_rand_d(int min, int max);

float f_rand(float max);
float f_rand_d(float min, float max);

float2_s float2_rand(float x_max, float y_max);
float2_s float2_rand_d(float x_min, float x_max, float y_min, float y_max);

float3_s float3_rand(float x_max, float y_max, float z_max);
float3_s float3_rand_d(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);

static inline float2_s float2_sub(float2_s a, float2_s b) { return (float2_s){a.x - b.x, a.y - b.y}; }
static inline float2_s float2_add(float2_s a, float2_s b) { return (float2_s){a.x + b.x, a.y + b.y}; }

static inline float2_s float2_mul(float2_s a, float2_s b) { return (float2_s){a.x * b.x, a.y * b.y}; }
static inline float2_s float2_div(float2_s a, float2_s b) { return (float2_s){a.x / b.x, a.y / b.y}; }
static inline float2_s float2_sub_f(float2_s a, float b) { return (float2_s){a.x - b, a.y - b}; }
static inline float2_s float2_add_f(float2_s a, float b) { return (float2_s){a.x + b, a.y + b}; }

static inline float2_s float2_mul_f(float2_s a, float b) { return (float2_s){a.x * b, a.y * b}; }

static inline float2_s float2_div_f(float2_s a, float b) { return (float2_s){a.x / b, a.y / b}; }

static inline float2_s f2(float x, float y) { return (float2_s){x, y}; }

static inline float3_s f3(float x, float y, float z) { return (float3_s){x, y, z}; }
static inline float3_s float3_sub(float3_s a, float3_s b) { return (float3_s){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline float3_s float3_add(float3_s a, float3_s b) { return (float3_s){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline float3_s float3_mul(float3_s a, float3_s b) { return (float3_s){a.x * b.x, a.y * b.y, a.z * b.z}; }

static inline float3_s float3_div(float3_s a, float3_s b) { return (float3_s){a.x / b.x, a.y / b.y, a.z / b.z}; }
static inline float3_s float3_sub_f(float3_s a, float b) { return (float3_s){a.x - b, a.y - b, a.z - b}; }
static inline float3_s float3_add_f(float3_s a, float b) { return (float3_s){a.x + b, a.y + b, a.z + b}; }
static inline float3_s float3_mul_f(float3_s a, float b) { return (float3_s){a.x * b, a.y * b, a.z * b}; }

static inline float3_s float3_div_f(float3_s a, float b) { return (float3_s){a.x / b, a.y / b, a.z / b}; }

static inline float float2_dot(float2_s one, float2_s two) { return (one.x * two.x) + (two.y * one.y); }

static inline float float3_dot(float3_s one, float3_s two) { return (one.x * two.x) + (two.y * one.y) + (two.y * one.y); }

static inline float2_s float2_perpendicular(float2_s one) { return (float2_s){one.y, -one.x}; }

static inline float signed_triangle_area(float2_s a, float2_s b, float2_s c) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "ArgumentSelectionDefects"
    return float2_dot(float2_sub(c, a), float2_perpendicular(float2_sub(b, a))) / 2;
#pragma clang diagnostic pop
}

static inline bool point_in_triangle(float2_s a, float2_s b, float2_s c, float2_s p, float3_s* weights) {
//    float area_ABP = signed_triangle_area(a, b, p);
//    float area_BCP = signed_triangle_area(b, c, p);
//    float area_CAP = signed_triangle_area(c, a, p);
//    bool inTri = (area_ABP <= 0 && area_BCP <= 0 && area_CAP <= 0) || (area_ABP >= 0 && area_BCP >= 0 && area_CAP >= 0);
//
////    float invAreaSum = 1 / (area_ABP + area_BCP + area_CAP);
////    float weightA = area_BCP * invAreaSum;
////    float weightB = area_CAP * invAreaSum;
////    float weightC = area_ABP * invAreaSum;
////    if(weights != nullptr) *weights = (float3_s){weightA, weightB, weightC};
//
//    return inTri;
    float A = .5f * (-b.y * c.x + a.y * (-b.x + c.x) + a.x * (b.y - c.y) + b.x * c.y);
    float sign = A < 0.f ? -1.f : 1.f;
    float s = (a.y * c.x - a.x * c.y + (c.y - a.y) * p.x + (a.x - c.x) * p.y) * sign;
    float t = (a.x * b.y - a.y * b.x + (a.y - b.y) * p.x + (b.x - a.x) * p.y) * sign;

    return s > 0 && t > 0 && (s + t) < 2 * A * sign;
}

static inline bool point_in_rect(float2_s tl, float2_s br, float2_s p) { return p.x > tl.x && p.x < br.x && p.y > tl.y && p.y < br.y; }
#endif //CENGINE_MATH_H
