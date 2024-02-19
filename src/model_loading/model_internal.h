#pragma once

#include <vector>

#include "utils/vectors.h"

struct vertex_t {
  vec3 positon; 
};

struct mesh_t {
  std::vector<vertex_t> vertices; 
  std::vector<unsigned int> indicies;
};

