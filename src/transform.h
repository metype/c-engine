#ifndef CENGINE_TRANSFORM_H
#define CENGINE_TRANSFORM_H
#include "math.h"

typedef struct transform {
    float rotation;
    float2_s position;
    float2_s scale;
} transform_s;

#define TRANSFORM_DEFAULT TRANSFORM_q(0.f, 0.f)
#define TRANSFORM(px, py, rot, sx, sy) (transform_s){.rotation = rot, .position = (float2_s){.x = px, .y = py}, .scale = (float2_s){.x = sx, .y = sy}}
#define TRANSFORM_q(px, py) TRANSFORM(px, py, 0.f, 1.f, 1.f)
#endif //CENGINE_TRANSFORM_H
