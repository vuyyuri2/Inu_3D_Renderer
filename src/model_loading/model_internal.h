#pragma once

#include <vector>
#include <stdint.h>

#include "utils/vectors.h"
#include "utils/log.h"
#include "gfx/gfx.h"

struct vertex_t {
  vec3 position; 
};

struct mesh_t {
  vao_t vao;

  vbo_t vbo;
  std::vector<vertex_t> vertices; 

  ebo_t ebo;
  std::vector<unsigned int> indicies;
};

struct model_t {
  std::vector<mesh_t> meshes;
};
