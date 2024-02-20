#pragma once

#include "model_loading/model_internal.h"

#include <vector>
#include <string>

struct gltf_node_t {
  std::vector<int> child_node_idxs;
  int gltf_mesh_handle = -1;
};

struct gltf_scene_t {
  std::vector<int> root_nodes;
};

struct gltf_attributes_t {
  int normals_accessor_idx = -1;
  int positions_accessor_idx = -1;
};

struct gltf_primitive_t {
  int indicies_accessor_idx = -1;
  int material_idx = -1;
  // gpu topology type
  int mode = -1;
  gltf_attributes_t attribs;
};

struct gltf_mesh_t {
  std::string name;
  std::vector<gltf_primitive_t> primitives; 
};

struct gltf_buffer_t {
  int byte_length = -1;
  std::string uri;
};

struct gltf_buffer_view_t {
  int gltf_buffer_index = -1;
  int byte_offset = -1;
  int byte_length = -1;
  int target = -1;
  int byte_stride = 1; 
};

struct gltf_accessor_t {
  int buffer_view_idx = -1;
  int byte_offset = -1;
  int component_type = -1;
  int count = -1;
  std::string type;
};

int gltf_parse_integer();
std::string gltf_parse_string();
std::vector<int> gltf_parse_integer_array();

gltf_primitive_t gltf_parse_primitive();
std::vector<gltf_primitive_t> gltf_parse_primitives_section();
gltf_attributes_t gltf_parse_attribs();

void gltf_skip_section();
void gltf_parse_scenes_section();
void gltf_parse_asset_section();
void gltf_parse_nodes_section();
void gltf_parse_meshes_section();
void gltf_parse_buffers_section();
void gltf_parse_buffer_views_section();
void gltf_parse_accessors_section();

void gltf_eat();
void gltf_parse_section();
char gltf_peek();
void gltf_preprocess(const char* filepath);
void gltf_load_file(const char* filepath, mesh_t& mesh);
