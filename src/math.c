#include "math.h"

float float2_dot(float2_s one, float2_s two) {
    return (one.x * two.x) + (two.y * one.y);
}

float float3_dot(float3_s one, float3_s two) {
    return (one.x * two.x) + (two.y * one.y) + (two.y * one.y);
}

float2_s float2_perpendicular(float2_s one) {
    return (float2_s){one.y, -one.x};
}

float signed_triangle_area(float2_s a, float2_s b, float2_s c) {
    float2_s ac = {c.x - a.x, c.y - a.y};
    float2_s ab_perp = float2_perpendicular((float2_s){b.x - a.x, b.y - a.y});
    return float2_dot(ac, ab_perp) / 2;
}

bool point_in_triangle(float2_s a, float2_s b, float2_s c, float2_s p, float3_s* weights) {
    float area_ABP = signed_triangle_area(a, b, p);
    float area_BCP = signed_triangle_area(b, c, p);
    float area_CAP = signed_triangle_area(c, a, p);
    bool inTri = area_ABP >= 0 && area_BCP >= 0 && area_CAP >= 0;

    float invAreaSum = 1 / (area_ABP + area_BCP + area_CAP);
    float weightA = area_BCP * invAreaSum;
    float weightB = area_CAP * invAreaSum;
    float weightC = area_ABP * invAreaSum;
    if(weights != nullptr) *weights = (float3_s){weightA, weightB, weightC};

    return inTri;
}