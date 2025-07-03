#ifndef CENGINE_SCENE_H
#define CENGINE_SCENE_H

#include "string.h"
#include "math.h"



typedef struct rect {
    float2_s tl;
    float2_s br;
} rect;

typedef struct tri {
    float2_s a;
    float2_s b;
    float2_s c;
    float2_s velocity;
    rect bb;
} tri;

typedef struct actor_s actor_s;
typedef struct viewport_s viewport;

typedef struct scene {
//    int triangle_count;
//    tri* triangles;
//    uint32_t* triangle_colors;
//    uint32_t* pixels;
    volatile actor_s* actor_tree;
    viewport* base_vp;
} scene;

string* Scene_serialize(void* serialized_obj);

void* Scene_deserialize(string* str);

void Scene_save(scene* scene_ptr, const char* path);
#endif //CENGINE_SCENE_H
