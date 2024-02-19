#pragma once

#include "model_loading/model_internal.h"

#include <vector>

struct gltf_mesh_t {

};

struct gltf_node_t {
  std::vector<int> child_node_handles;
  int gltf_mesh_handle = -1;
};

struct gltf_scene_t {
  std::vector<int> root_nodes;
};

std::vector<int> gltf_parse_integer_array();
void gltf_parse_scenes_section();
void gltf_parse_asset_section();
void gltf_parse_nodes_section();

void gltf_eat();
void gltf_parse_section();
char gltf_peek();
void gltf_preprocess(const char* filepath);
void gltf_load_file(const char* filepath, mesh_t& mesh);
