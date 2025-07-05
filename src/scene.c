#include "scene.h"
#include "actors/actor.h"
#include "log.h"
#include "viewport.h"
#include "filesystem.h"

string* Scene_serialize(void* serialized_obj) { // NOLINT(*-no-recursion)
    if(!serialized_obj) return s("nil");
    scene* scene_ptr = serialized_obj;
    string* ret_str = s("");

    s_cat(ret_str, so("base_vp {\n\t"));
    s_cat(ret_str, S_replace_n(Viewport_serialize(scene_ptr->base_vp), so("\n"), so("\n\t"), -1));
    s_cat(ret_str, so("} viewport\n"));

    s_cat(ret_str, so("actor_tree {\n\t"));
    s_cat(ret_str, S_replace_n(Actor_serialize(scene_ptr->actor_tree), so("\n"), so("\n\t"), -1));
    s_cat(ret_str, so("} actor\n"));

    return ret_str;
}

void* Scene_deserialize(string* str) {
    if(!str) return nullptr;
    if(!str->c_str) return nullptr;

    scene* scene_ptr = malloc(sizeof(scene));

    scene_ptr->highlights = nullptr;

    generic_deserialize_begin("scene")
        deserialize_stage_0()

        if (stage == 1) {
            snprintf(value_buf, 128, "%s", buffer);
            {
                if (strcmp(name_buf, "actor_tree") == 0) {
                    if (strcmp(value_buf, "{") == 0) {
                        scene_ptr->actor_tree = (actor_s *) Deserialize_block_to_obj(
                                S_convert((str->c_str + marcher - 1)));
                        unsigned block_count = 1;
                        while (block_count) {
                            marcher++;
                            if (str->c_str[marcher] == '{')block_count++;
                            if (str->c_str[marcher] == '}')block_count--;
                            used = true;
                        }
                        while (str->c_str[marcher] != '\n')marcher++;
                        stage = -1;
                    }
                }
                if (strcmp(name_buf, "base_vp") == 0) {
                    if (strcmp(value_buf, "{") == 0) {
                        scene_ptr->base_vp = (viewport *) Deserialize_block_to_obj(
                                S_convert((str->c_str + marcher - 1)));
                        unsigned block_count = 1;
                        while (block_count) {
                            marcher++;
                            if (str->c_str[marcher] == '{')block_count++;
                            if (str->c_str[marcher] == '}')block_count--;
                            used = true;
                        }
                        while (str->c_str[marcher] != '\n')marcher++;
                        stage = -1;
                    }
                }
            }
        }

        deserialize_stage_2()
    generic_deserialize_end()

    return scene_ptr;
}

void Scene_save(scene* scene_ptr, const char* path) {
    string* cwd = sc(FS_get_working_dir());

    FS_create_dir_if_not_exist(cwd->c_str);

    char* final_path = malloc(sizeof(char) * FS_PATH_MAX);
    snprintf(final_path, FS_PATH_MAX, "/saves/%s.scene", path);

    s_cat(cwd, sco(final_path));

    file_s scene_file = FS_open(cwd->c_str, FA_FILE_WRITE);

    ref_dec(&cwd->refcount);

    file_ready_serialize(scene, scene_ptr, serialized_data);
    fprintf(scene_file.f_ptr, "%s", serialized_data->c_str);
    FS_close(scene_file);
    ref_dec(&serialized_data->refcount);
}