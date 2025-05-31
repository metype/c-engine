#ifndef CENGINE_MATH_H
#define CENGINE_MATH_H

typedef struct float2_s {
    float x;
    float y;
} float2_s;

typedef struct float3_s {
    float x;
    float y;
    float z;
} float3_s;

float float2_dot(float2_s one, float2_s two);
float float3_dot(float3_s one, float3_s two);
float2_s float2_perpendicular(float2_s one);
float signed_triangle_area(float2_s a, float2_s b, float2_s c);
bool point_in_triangle(float2_s a, float2_s b, float2_s c, float2_s p, float3_s* weights);
#endif //CENGINE_MATH_H
