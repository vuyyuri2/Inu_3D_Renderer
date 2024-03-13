#pragma once

#include <vector>
#include <unordered_set>

#include "utils/vectors.h"
#include "utils/mats.h"
#include "transform.h"
#include "animation/animation_internal.h"

struct object_t {
  int id = -1;
  transform_t transform;
  int model_id = -1;
  std::vector<int> child_objects; 
  mat4 model_mat;
  std::vector<animation_chunk_data_ref_t> anim_chunk_refs;
};

struct scene_t {
  std::unordered_set<int> parent_objs;
};

int create_object(transform_t& transform);
void attach_model_to_obj(int obj_id, int model_id);
void attach_child_obj_to_obj(int obj_id, int child_obj_id);

void set_obj_as_parent(int obj_id);
void update_obj_model_mats();
void attach_anim_chunk_ref_to_obj(int obj_id, animation_chunk_data_ref_t& ref);

void render_scene();
