#pragma once

#include "utils/vectors.h"
#include "utils/quaternion.h"

struct transform_t {
  vec3 pos;
  vec3 scale;
  // vec3 rot;
  quaternion_t rot;
};

mat4 get_model_matrix(transform_t& t);
