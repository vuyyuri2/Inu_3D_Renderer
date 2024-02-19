#pragma once

#include <vector>
#include <stdint.h>

#include "utils/vectors.h"
#include "utils/log.h"

struct vertex_t {
  vec3 positon; 
};

struct mesh_t {
  std::vector<vertex_t> vertices; 
  std::vector<unsigned int> indicies;
};

