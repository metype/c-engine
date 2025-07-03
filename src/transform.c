#include "transform.h"
#include "serialization.h"
#include "string.h"
#include <string.h>
#include "log.h"

string* Transform_serialize(void* serialized_obj) {
    if(!serialized_obj) return s("nil");
    transform_s* transform = serialized_obj;
    string* ret_str = s("");

    generic_serialize_value(transform, position.x, float, "x");
    generic_serialize_value(transform, position.y, float, "y");
    generic_serialize_value(transform, scale.x, float, "scale_x");
    generic_serialize_value(transform, scale.y, float, "scale_y");
    generic_serialize_value(transform, rotation, float, "rot");
    generic_serialize_value(transform, top_level, bool, "top_level");

    return ret_str;
}

void* Transform_deserialize(string* str) {
    if(!str) return nullptr;
    if(!str->c_str) return nullptr;

    transform_s* transform = malloc(sizeof(transform_s));
    transform->position.x = 0;
    transform->position.y = 0;
    transform->scale.x = 0;
    transform->scale.y = 0;
    transform->rotation = 0;
    transform->top_level = false;

    generic_deserialize_begin("transform")
        deserialize_stage_0()

        deserialize_stage_1()

        deserialize_stage_2(
                generic_deserialize_value(transform, position.x, float, x);
                generic_deserialize_value(transform, position.y, float, y);
                generic_deserialize_value(transform, scale.x, float, scale_x);
                generic_deserialize_value(transform, scale.y, float, scale_y);
                generic_deserialize_value(transform, rotation, float, rot);
                generic_deserialize_value(transform, top_level, bool, top_level);
        )
    generic_deserialize_end()

    return transform;
}