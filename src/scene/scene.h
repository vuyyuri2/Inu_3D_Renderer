#pragma once

#include <vector>

#include "utils/vectors.h"

struct transform_t {
  vec3 pos;
  vec3 scale;
  vec3 rot;
};

struct object_t {
  int id = -1;
  transform_t transform;
  int model_id = -1;
  std::vector<int> child_objects; 
};

int create_object(transform_t& transform);
void attach_model_to_obj(int obj_id, int model_id);
void attach_child_obj_to_obj(int obj_id, int child_obj_id);

