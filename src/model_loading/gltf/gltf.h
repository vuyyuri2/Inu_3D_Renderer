#pragma once

#include "model_loading/model_internal.h"
#include "utils/vectors.h"

#include <vector>
#include <string>

#define MAX_SUPPORTED_TEX_COORDS 4

// TODO: need to parse transform
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
  int color_0_accessor_idx = -1;
  int tex_coord_accessor_indicies[MAX_SUPPORTED_TEX_COORDS];
};

enum class GLTF_PRIMITIVE_MODE {
  POINTS = 0,
  LINES,
  LINE_LOOP,
  LINE_STRIP,
  TRIANGLES,
  TRIANGLE_STRIP,
  TRIANGLE_FAN
};

struct gltf_primitive_t {
  int indicies_accessor_idx = -1;
  int material_idx = -1;
  // gpu topology type
  GLTF_PRIMITIVE_MODE mode = GLTF_PRIMITIVE_MODE::TRIANGLES;
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

enum class BUFFER_VIEW_TARGET {
  NONE = 0,
  ARRAY_BUFFER = 34962,
  ELEMENT_ARRAY_BUFFER = 34963
};

struct gltf_buffer_view_t {
  int gltf_buffer_index = -1;
  int byte_offset = 0;
  int byte_length = -1;
  BUFFER_VIEW_TARGET target = BUFFER_VIEW_TARGET::NONE;
  // will need to look into later on when this value becomes more important
  int byte_stride = -1; 
};

struct gltf_mat_image_info_t {
  int gltf_texture_idx = -1;
  int tex_coord_idx = 0;
};

struct gltf_pbr_metallic_roughness_t {
  vec4 base_color_factor;
  float metallic_factor = 0;
  gltf_mat_image_info_t base_color_tex_info;
};

struct gltf_material_t {
  std::string name;
  gltf_pbr_metallic_roughness_t pbr;
};

enum class ACC_COMPONENT_TYPE {
  BYTE = 5120,
  UNSIGNED_BYTE = 5121,
  SHORT = 5122,
  UNSIGNED_SHORT = 5123,
  UNSIGNED_INT = 5125,
  FLOAT = 5126
};

struct gltf_accessor_t {
  int buffer_view_idx = -1;
  int byte_offset = 0;
  ACC_COMPONENT_TYPE component_type = ACC_COMPONENT_TYPE::BYTE;
  int count = -1;
  std::string type;
};

struct gltf_image_t {
  std::string uri;
  // need to add support for image in buffer view
};

struct gltf_texture_t {
  int sampler_idx = -1;
  int image_source_idx = -1;
};

enum class MAG_FILTER {
  NONE = 0,
  NEAREST = 9728,
  LINEAR = 9729
};

enum class MIN_FILTER {
  NONE = 0,
  NEAREST = 9728,
  LINEAR = 9729,
  NEAREST_MIPMAP_NEAREST = 9984,
  LINEAR_MIPMAP_NEAREST = 9985,
  NEAREST_MIPMAP_LINEAR = 9986,
  LINEAR_MIPMAP_LINEAR = 9987
};

enum class SAMPLER_WRAP {
  NONE = 0,
  CLAMP_TO_EDGE = 33071,
  MIRRORED_REPEAT = 33648,
  REPEAT = 10497
};

struct gltf_sampler_t {
  MAG_FILTER mag_filter = MAG_FILTER::LINEAR;
  MIN_FILTER min_filter = MIN_FILTER::LINEAR;
  SAMPLER_WRAP wrap_s = SAMPLER_WRAP::REPEAT;
  SAMPLER_WRAP wrap_t = SAMPLER_WRAP::REPEAT;
};

int gltf_parse_integer();
std::string gltf_parse_string();
std::vector<int> gltf_parse_integer_array();
vec4 gltf_parse_vec4();
float gltf_parse_float();

gltf_primitive_t gltf_parse_primitive();
std::vector<gltf_primitive_t> gltf_parse_primitives_section();
gltf_attributes_t gltf_parse_attribs();

/*
 gltf sections

  asset
  scene
  scenes --
    nodes --
  nodes
    children --
    matrix
    mesh --
  meshes --
    primitives --
      attributes --
        normal --
        position --
      indices --
      mode --
      material --
    name --
  accessors
    bufferView --
    byteOffset --
    componentType --
    count --
    max
    min
    type --
  materials
    pbrMetallicRoughness
      baseColorFactor
      metallicFactor
    name
  bufferViews --
    buffer --
    byteOffset --
    byteLength --
    target --
    byteStride --
  buffers --
    byteLength --
    uri --
  textures
    sampler
    source
  images
    magFilter
    minFilter
    wrapS
    wrapT
 */

void gltf_skip_section();
void gltf_parse_scenes_section();
void gltf_parse_asset_section();
void gltf_parse_nodes_section();
void gltf_parse_meshes_section();
void gltf_parse_buffers_section();
void gltf_parse_buffer_views_section();
void gltf_parse_accessors_section();
void gltf_parse_images_section();
void gltf_parse_textures_section();
void gltf_parse_samplers_section();
void gltf_parse_materials_section();

void gltf_eat();
void gltf_parse_section();
char gltf_peek();
void gltf_preprocess(const char* filepath);
void gltf_load_file(const char* filepath, std::vector<model_t>& models);
