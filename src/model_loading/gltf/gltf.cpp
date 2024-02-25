#include "gltf.h"

#include <ctype.h>
#include <unordered_set>

#include "utils/log.h"
#include "utils/vectors.h"
#include "model_loading/model_internal.h"
#include "gfx/gfx.h"

/*
 https://github.com/KhronosGroup/glTF-Sample-Models/blob/main/2.0/README.md#showcase
 */

char* data = NULL;
int offset = -1;
int data_len = -1;

static int active_scene = -1;
static std::vector<gltf_scene_t> gltf_scenes; 
static std::vector<gltf_node_t> gltf_nodes; 
static std::vector<gltf_mesh_t> gltf_meshes;
static std::vector<gltf_buffer_t> gltf_buffers;
static std::vector<gltf_buffer_view_t> gltf_buffer_views;
static std::vector<gltf_accessor_t> gltf_accessors; 
static std::vector<gltf_image_t> gltf_images;
static std::vector<gltf_texture_t> gltf_textures;
static std::vector<gltf_sampler_t> gltf_samplers;
static std::vector<gltf_material_t> gltf_materials;

static char folder_path[256];

void gltf_skip_section() {
  // skip logic
  int num_opening_seen = 0;  
  bool looking_for_comma = true;
  char opening = 0;
  char closing = 0;
  while (true) {
    if (looking_for_comma && gltf_peek() == ',') {
      gltf_eat();
      break;
    } else if (!looking_for_comma && num_opening_seen == 0) {
      break;
    }
    char peeked = gltf_peek();

    if (peeked == '{' && opening == 0) {
      opening = '{';
      closing = '}';
    } else if (peeked == '[' && opening == 0) {
      opening = '[';
      closing = ']';
    }

    if ((peeked == ']' || peeked == '}') && looking_for_comma) {
      break;
    }

    if (peeked == opening) {
      looking_for_comma = false;
      num_opening_seen++;
    } else if (peeked == closing) {
      num_opening_seen--;
    }
    gltf_eat();
  }

  if (!looking_for_comma && gltf_peek() == ',') {
    gltf_eat();
  }
}

void gltf_parse_asset_section() {
  inu_assert(gltf_peek() == '{');
  gltf_eat();

  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();

    inu_assert(gltf_peek() == ':');
    gltf_eat();

    if (key == "version") {
      std::string version = gltf_parse_string();
      float fversion = atof(version.c_str());
      if (fversion < 2.f || fversion >= 3.f) {
        inu_assert_msg("gltf file is not major version 2");
      }
    } else {
      gltf_skip_section();
    }

    if (gltf_peek() == ',') {
      gltf_eat();
    }
  }
  gltf_eat(); 
}

std::vector<int> gltf_parse_integer_array() {
  std::vector<int> ints;
  inu_assert(gltf_peek() == '[');
  gltf_eat();
  char* int_str_start = data + offset;
  while (gltf_peek() != ']') {
    if (gltf_peek() == ',') {
      gltf_eat();
      data[offset-1] = 0;
      int val = atoi(int_str_start);
      ints.push_back(val);
      continue;
    }
    gltf_eat();
  }
  gltf_eat();
  data[offset-1] = 0;
  int val = atoi(int_str_start);
  ints.push_back(val);
  return ints;
}

float gltf_parse_float() {
  bool looking_for_comma = true;
  char* start = data + offset;
  while (gltf_peek() != ',' && gltf_peek() != '}' && gltf_peek() != ']') {
    gltf_eat();
  }
  char c = gltf_peek();
  data[offset] = 0;
  float v = atof(start);
  data[offset] = c;
  return v;
}

vec4 gltf_parse_vec4() {
  inu_assert(gltf_peek() == '[');
  gltf_eat();

  vec4 v;

  bool looking_for_comma = true;
  v.x = gltf_parse_float();
  if (gltf_peek() == ',') gltf_eat();
  v.y = gltf_parse_float();
  if (gltf_peek() == ',') gltf_eat();
  v.z = gltf_parse_float();
  if (gltf_peek() == ',') gltf_eat();
  v.w = gltf_parse_float();

  inu_assert(gltf_peek() == ']');
  gltf_eat();

  return v;
}

void gltf_parse_scene() {

  gltf_scene_t scene;

  inu_assert(gltf_peek() == '{');
  gltf_eat();
  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();

    if (key == "nodes") {
      // get the root nodes
      inu_assert(gltf_peek() == ':');
      gltf_eat();
      scene.root_nodes = gltf_parse_integer_array();
      if (gltf_peek() == ',') gltf_eat();
    } else {
      gltf_skip_section();
    }
  }
  gltf_eat();
  gltf_scenes.push_back(scene);
}

void gltf_parse_scenes_section() {
  inu_assert(gltf_peek() == '[');
  gltf_eat();
  while (gltf_peek() != ']') {
    gltf_parse_scene();
    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
}

void gltf_parse_node() {
  gltf_node_t node;

  inu_assert(gltf_peek() == '{');
  gltf_eat();

  while (gltf_peek() != '}') {

    std::string key = gltf_parse_string();

    inu_assert(gltf_peek() == ':');
    gltf_eat();

    if (key == "children") {
      // get the root nodes
      node.child_node_idxs = gltf_parse_integer_array();
      if (gltf_peek() == ',') gltf_eat();
    } else if (key == "mesh") {
      node.gltf_mesh_handle = gltf_parse_integer();
      if (gltf_peek() == ',') gltf_eat();
    } else {
      gltf_skip_section();
    }
  }
  gltf_eat();
  gltf_nodes.push_back(node);
}

void gltf_parse_nodes_section() {
  inu_assert(gltf_peek() == '[');
  gltf_eat();
  while (gltf_peek() != ']') {
    gltf_parse_node();
    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
}

std::string gltf_parse_string() {
  inu_assert(gltf_peek() == '\"');
  gltf_eat();
  char* start = data + offset;
  while (gltf_peek() != '\"') {
    gltf_eat();
  }
  data[offset] = 0;
  std::string val = start;
  data[offset] = '\"';
  gltf_eat();

  return val;
}

gltf_attributes_t gltf_parse_attribs() {
  gltf_attributes_t attrib;

  inu_assert(gltf_peek() == '{');
  gltf_eat();

  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();

    inu_assert(gltf_peek() == ':');
    gltf_eat();

    if (key == "NORMAL") {
      attrib.normals_accessor_idx = gltf_parse_integer();
    } else if (key == "POSITION") {
      attrib.positions_accessor_idx = gltf_parse_integer();
    } else if (key == "TEXCOORD_0") {
      attrib.tex_coord_0_accessor_idx = gltf_parse_integer();
    } else {
      // not sure if this works for individual elements
      gltf_skip_section();
    }
    if (gltf_peek() == ',') gltf_eat();
  }  
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();

  return attrib;
}

gltf_primitive_t gltf_parse_primitive() {
  gltf_primitive_t prim;

  inu_assert(gltf_peek() == '{');
  gltf_eat();

  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();

    inu_assert(gltf_peek() == ':');
    gltf_eat();

    if (key == "attributes") {
      prim.attribs = gltf_parse_attribs();
    } else if (key == "indices") {
      prim.indicies_accessor_idx = gltf_parse_integer();
    } else if (key == "mode") {
      prim.mode = static_cast<GLTF_PRIMITIVE_MODE>(gltf_parse_integer());
    } else if (key == "material") {
      prim.material_idx = gltf_parse_integer();
    } else {
      gltf_skip_section();
    }
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();

  return prim;
}

std::vector<gltf_primitive_t> gltf_parse_primitives_section() {
  std::vector<gltf_primitive_t> prims;
  inu_assert(gltf_peek() == '[');
  gltf_eat();
  while (gltf_peek() != ']') {
    gltf_primitive_t prim = gltf_parse_primitive();
    prims.push_back(prim);
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();
  return prims;
}

void gltf_parse_mesh() {
  gltf_mesh_t mesh;

  inu_assert(gltf_peek() == '{');
  gltf_eat();

  while (gltf_peek() != '}') {

    std::string key = gltf_parse_string();
    inu_assert(gltf_peek() == ':');
    gltf_eat();

    if (key == "primitives") {
      mesh.primitives = gltf_parse_primitives_section();
    } else if (key == "name") {
      mesh.name = gltf_parse_string();
    } else {
      gltf_skip_section();
    }

    if (gltf_peek() == ',') gltf_eat();

  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();

  gltf_meshes.push_back(mesh);
}

void gltf_parse_meshes_section() {
  inu_assert(gltf_peek() == '[');
  gltf_eat();
  while (gltf_peek() != ']') {
    gltf_parse_mesh();
  }
  gltf_eat();
}

int gltf_parse_integer() {
  char* start = data + offset; 
  int val = -1;
  while (true) {
    if (gltf_peek() == ',') {
      gltf_eat();
      data[offset-1] = 0;
      val = atoi(start);
      break;
    } else if (gltf_peek() == '}') {
      data[offset] = 0;
      val = atoi(start);
      data[offset] = '}';
      break;
    }
    gltf_eat();
  }
  return val;
}

void gltf_parse_buffer() {
  gltf_buffer_t buffer; 
  inu_assert(gltf_peek() == '{');
  gltf_eat();
  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();
    inu_assert(gltf_peek() == ':');
    gltf_eat();
    if (key == "byteLength") {
      buffer.byte_length = gltf_parse_integer();
    } else if (key == "uri") {
      buffer.uri = gltf_parse_string();
    }
    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();
  gltf_buffers.push_back(buffer);
}

void gltf_parse_buffers_section() {
  inu_assert(gltf_peek() == '[');
  gltf_eat();
  while (gltf_peek() != ']') {
    gltf_parse_buffer();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();
}

void gltf_parse_accessor() {
  gltf_accessor_t accessor; 
  inu_assert(gltf_peek() == '{');
  gltf_eat();
  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();
    inu_assert(gltf_peek() == ':');
    gltf_eat();
    if (key == "bufferView") {
      accessor.buffer_view_idx = gltf_parse_integer();
    } else if (key == "byteOffset") {
      accessor.byte_offset = gltf_parse_integer();
    } else if (key == "componentType") {
      accessor.component_type = static_cast<ACC_COMPONENT_TYPE>(gltf_parse_integer());
    } else if (key == "count") {
      accessor.count = gltf_parse_integer();
    } else if (key == "type") {
      accessor.type = gltf_parse_string();
    } else {
      gltf_skip_section();
    }
    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();
  gltf_accessors.push_back(accessor);
}

void gltf_parse_accessors_section() {
  inu_assert(gltf_peek() == '[');
  gltf_eat();
  while (gltf_peek() != ']') {
    gltf_parse_accessor();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();
}

void gltf_parse_buffer_view() {
  gltf_buffer_view_t buffer_view;

  inu_assert(gltf_peek() == '{');
  gltf_eat();

  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();
    inu_assert(gltf_peek() == ':');
    gltf_eat();
    if (key == "buffer") {
      buffer_view.gltf_buffer_index = gltf_parse_integer();
    } else if (key == "byteOffset") {
      buffer_view.byte_offset = gltf_parse_integer();
    } else if (key == "byteLength") {
      buffer_view.byte_length = gltf_parse_integer();
    } else if (key == "byteStride") {
      buffer_view.byte_stride = gltf_parse_integer();
    } else if (key == "target") {
      buffer_view.target = static_cast<BUFFER_VIEW_TARGET>(gltf_parse_integer());
    }
    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();

  gltf_buffer_views.push_back(buffer_view);
}

void gltf_parse_buffer_views_section() {
  inu_assert(gltf_peek() == '[');
  gltf_eat();
  while (gltf_peek() != ']') {
    gltf_parse_buffer_view();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();
}

void gltf_parse_image() {
  inu_assert(gltf_peek() == '{');
  gltf_eat();

  gltf_image_t img;

  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();
    inu_assert(gltf_peek() == ':');
    gltf_eat();
    if (key == "uri") {
      img.uri = gltf_parse_string();
    } else {
      gltf_skip_section();
    }

    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();

  gltf_images.push_back(img);
}

void gltf_parse_images_section() {
  inu_assert(gltf_peek() == '[');
  gltf_eat();
  while (gltf_peek() != ']') {
    gltf_parse_image();
  }
  gltf_eat();
}

void gltf_parse_texture() {
  inu_assert(gltf_peek() == '{');
  gltf_eat();

  gltf_texture_t tex;

  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();
    inu_assert(gltf_peek() == ':');
    gltf_eat();
    if (key == "sampler") {
      tex.sampler_idx = gltf_parse_integer();
    } else if (key == "source") {
      tex.image_source_idx = gltf_parse_integer();
    } else {
      gltf_skip_section();
    }

    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();

  gltf_textures.push_back(tex);
}

void gltf_parse_textures_section() {
  inu_assert(gltf_peek() == '[');
  gltf_eat();
  while (gltf_peek() != ']') {
    gltf_parse_texture();
  }
  gltf_eat();
}

void gltf_parse_sampler() {
  inu_assert(gltf_peek() == '{');
  gltf_eat();

  gltf_sampler_t sampler;

  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();
    inu_assert(gltf_peek() == ':');
    gltf_eat();
    if (key == "magFilter") {
      sampler.mag_filter = static_cast<MAG_FILTER>(gltf_parse_integer());
    } else if (key == "minFilter") {
      sampler.min_filter = static_cast<MIN_FILTER>(gltf_parse_integer());
    } else if (key == "wrapS") {
      sampler.wrap_s = static_cast<SAMPLER_WRAP>(gltf_parse_integer());
    } else if (key == "wrapT") {
      sampler.wrap_t = static_cast<SAMPLER_WRAP>(gltf_parse_integer());
    } else {
      gltf_skip_section();
    }
    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();

  gltf_samplers.push_back(sampler);
}

void gltf_parse_samplers_section() {
  inu_assert(gltf_peek() == '[');
  gltf_eat();

  while (gltf_peek() != ']') {
    gltf_parse_sampler();
    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();
}

gltf_pbr_metallic_roughness_t gltf_parse_pbr_met_rough() {
  inu_assert(gltf_peek() == '{');
  gltf_eat();

  gltf_pbr_metallic_roughness_t pbr;
  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();
    inu_assert(gltf_peek() == ':');
    gltf_eat();
    if (key == "baseColorFactor") {
      pbr.base_color_factor = gltf_parse_vec4();
    } else if (key == "metallicFactor") {
      pbr.metallic_factor = gltf_parse_float();
    } else {
      gltf_skip_section();
    }
    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();

  return pbr;
}

void gltf_parse_material() {
  inu_assert(gltf_peek() == '{');
  gltf_eat();

  gltf_material_t mat;

  while (gltf_peek() != '}') {
    std::string key = gltf_parse_string();
    inu_assert(gltf_peek() == ':');
    gltf_eat();
    if (key == "pbrMetallicRoughness") {
      mat.pbr = gltf_parse_pbr_met_rough();
    } else if (key == "name") {
      mat.name = gltf_parse_string();
    }
    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();

  gltf_materials.push_back(mat);
}

void gltf_parse_materials_section() {
  inu_assert(gltf_peek() == '[');
  gltf_eat();

  while (gltf_peek() != ']') {
    gltf_parse_material();
    if (gltf_peek() == ',') gltf_eat();
  }
  gltf_eat();
  if (gltf_peek() == ',') gltf_eat();
}

void gltf_parse_section() {

  std::string key = gltf_parse_string();

  inu_assert(gltf_peek() == ':', "colon is not seen"); 
  gltf_eat();

  if (key == "asset") {
    gltf_parse_asset_section();
  } else if (key == "nodes") {
    gltf_parse_nodes_section();
  } else if (key == "scenes") {
    gltf_parse_scenes_section();
  } else if (key == "meshes") {
    gltf_parse_meshes_section();
  } else if (key == "scene") {
    active_scene = gltf_parse_integer();
  } else if (key == "buffers") {
    gltf_parse_buffers_section();
  } else if (key == "bufferViews") {
    gltf_parse_buffer_views_section();
  } else if (key == "accessors") {
    gltf_parse_accessors_section();
  } else if (key == "images") {
    gltf_parse_images_section();
  } else if (key == "textures") {
    gltf_parse_textures_section();
  } else if (key == "samplers") {
    gltf_parse_samplers_section();
  } else if (key == "materials") {
    gltf_parse_materials_section();
  } else {
    gltf_skip_section();
  }

  if (gltf_peek() == ',') {
    gltf_eat();
  }
}

char gltf_peek() {
  if (offset < data_len) {
    return data[offset];
  }
  return 0;
}

void gltf_eat() {
  if (offset < data_len) {
    offset++;
  }
}

void gltf_preprocess(const char* filepath) {
  int buffer_len = 1000;
  char* buffer = (char*)malloc(buffer_len * sizeof(uint8_t));
  FILE* gltf_file = fopen(filepath, "r");
  inu_assert(gltf_file, "gltf file not found");
  int count = 0;
  while (!feof(gltf_file)) {
    char c = fgetc(gltf_file);
    if (c > 0 && !isspace(c)) {
      buffer[count] = c;
      count++;
      if (count == buffer_len) {
        buffer_len *= 2;
        buffer = (char*)realloc(buffer, buffer_len);
      }
    } 
  }
  fclose(gltf_file);

  if (buffer_len > count) {
    char* end = buffer + count;
    memset(end, 0, buffer_len - count);
  }

  data = buffer;
  data_len = count; 
  offset = 0;
}

void* gltf_read_accessor_data(int accessor_idx) {
  inu_assert(accessor_idx != -1);
  gltf_accessor_t& acc = gltf_accessors[accessor_idx];
  gltf_buffer_view_t& buffer_view = gltf_buffer_views[acc.buffer_view_idx];
  
  int size_of_component = 0;
  switch (acc.component_type) {
    case ACC_COMPONENT_TYPE::BYTE: {
      size_of_component = sizeof(char);
      break;
    }
    case ACC_COMPONENT_TYPE::UNSIGNED_BYTE: {
      size_of_component = sizeof(unsigned char);
      break;
    }
  case ACC_COMPONENT_TYPE::SHORT: {
      size_of_component = sizeof(short);
      break;
    }
  case ACC_COMPONENT_TYPE::UNSIGNED_SHORT: {
      size_of_component = sizeof(unsigned short);
      break;
    }
  case ACC_COMPONENT_TYPE::UNSIGNED_INT: {
      size_of_component = sizeof(unsigned int);
      break;
    }
  case ACC_COMPONENT_TYPE::FLOAT: {
      size_of_component = sizeof(float);
      break;
    }
  }

  int size_of_element = 0;
  if (acc.type == "SCALAR") {
    size_of_element = size_of_component;
  } else if (acc.type == "VEC3") {
    size_of_element = size_of_component * 3; 
  } else if (acc.type == "VEC2") {
    size_of_element = size_of_component * 2;
  } else {
    inu_assert_msg("this accessor type is not supported");
  }

  int size_of_acc_data = size_of_element * acc.count;

  inu_assert(size_of_acc_data <= buffer_view.byte_length);
  char* data = (char*)malloc(size_of_acc_data);

  gltf_buffer_t& buffer = gltf_buffers[buffer_view.gltf_buffer_index];
  char buffer_uri_full_path[256]{};
  sprintf(buffer_uri_full_path, "%s\\%s", folder_path, buffer.uri.c_str());

  FILE* uri_file = fopen(buffer_uri_full_path, "r");
  inu_assert(uri_file, "uri file does not exist");

  int start_offset = acc.byte_offset + buffer_view.byte_offset;

  int stride = -1;
  if (buffer_view.byte_stride != -1) {
    stride = buffer_view.byte_stride;
  } else {
    stride = size_of_element;
  }

  int count = 0;
  for (int i = 0; i < acc.count; i++) {
    int offset = start_offset + (i * stride);
    fseek(uri_file, offset, SEEK_SET);
    for (int j = 0; j < size_of_element; j++) {
      data[count] = fgetc(uri_file);
      count++;
    }
  }

  inu_assert(count == size_of_acc_data, "size of actual data not equal to final count");
  
  fclose(uri_file);
  
  return (void*)data;
}

void gltf_load_file(const char* filepath, std::vector<model_t>& models) {

  printf("loading gltf file: %s\n", filepath);

  // 1. preprocess step
  gltf_preprocess(filepath);

  // 2. load meta data step
  inu_assert('{' == gltf_peek(), "initial character is not opening curly brace");
  gltf_eat();

  while (gltf_peek() != '}') {
    gltf_parse_section();
  }

  free(data);
  data = NULL;

  // 3. load into internal format/ load raw data
  for (gltf_material_t& mat : gltf_materials) {
    texture_t t;
    create_material(mat.pbr.base_color_factor, t);
  }

  const char* last_slash = strrchr(filepath, '\\');
  memset(folder_path, 0, 256);
  memcpy(folder_path, filepath, last_slash - filepath);
  for (gltf_mesh_t& gltf_mesh : gltf_meshes) {
    model_t model;
    
    // each prim could have its own vao, vbo, and ebo
    for (gltf_primitive_t& prim : gltf_mesh.primitives) {
      mesh_t mesh;

      if (prim.indicies_accessor_idx != -1) {
        void* index_data = gltf_read_accessor_data(prim.indicies_accessor_idx);
        gltf_accessor_t& acc = gltf_accessors[prim.indicies_accessor_idx];
        if (acc.component_type == ACC_COMPONENT_TYPE::UNSIGNED_INT) {
          unsigned int* uint_data = static_cast<unsigned int*>(index_data);
          for (int i = 0; i < acc.count; i++) {
            mesh.indicies.push_back(uint_data[i]);
          }
        } else if (acc.component_type == ACC_COMPONENT_TYPE::UNSIGNED_SHORT) {
          unsigned short* ushort_data = static_cast<unsigned short*>(index_data);
          for (int i = 0; i < acc.count; i++) {
            unsigned int val = ushort_data[i];
            mesh.indicies.push_back(val);
          }
        }
      }

      if (prim.attribs.normals_accessor_idx != -1) {
        // void* normals_data = gltf_read_accessor_data(prim.attribs.normals_accessor_idx);
      }

      if (prim.attribs.positions_accessor_idx != -1) {
        void* positions_data = gltf_read_accessor_data(prim.attribs.positions_accessor_idx);
        gltf_accessor_t& acc = gltf_accessors[prim.attribs.positions_accessor_idx];
        if (acc.component_type == ACC_COMPONENT_TYPE::FLOAT) {
          // can do direct static cast b/c vec3 is made up of floats
          vec3* pos_data = static_cast<vec3*>(positions_data);
          if (mesh.vertices.size() == 0) {
            mesh.vertices.resize(acc.count);
          }
          for (int i = 0; i < acc.count; i++) {
            vertex_t& vert = mesh.vertices[i];
            vert.position = pos_data[i];
          }
        } else {
          inu_assert_msg("this type for positions data is not supported yet");
        }
      }
      
      if (prim.attribs.tex_coord_0_accessor_idx != -1) {
        void* tex_0_data = gltf_read_accessor_data(prim.attribs.tex_coord_0_accessor_idx);
        gltf_accessor_t& tex_acc = gltf_accessors[prim.attribs.tex_coord_0_accessor_idx];
        if (tex_acc.component_type == ACC_COMPONENT_TYPE::FLOAT) {
          // can do direct static cast b/c vec2 is made up of floats
          vec2* tex_data = static_cast<vec2*>(tex_0_data);
          if (mesh.vertices.size() == 0) {
            mesh.vertices.resize(tex_acc.count);
          }
          for (int i = 0; i < tex_acc.count; i++) {
            vertex_t& vert = mesh.vertices[i];
            vert.tex0 = tex_data[i];
          }
        } else {
          inu_assert_msg("this type for texture data is not supported yet");
        }
      }

      mesh.mat_idx = prim.material_idx;

      mesh.vao = create_vao();
      mesh.vbo = create_vbo((float*)mesh.vertices.data(), mesh.vertices.size() * sizeof(vertex_t));
      mesh.ebo = create_ebo(mesh.indicies.data(), mesh.indicies.size() * sizeof(unsigned int));

      vao_enable_attribute(mesh.vao, mesh.vbo, 0, 3, GL_FLOAT, sizeof(vertex_t), offsetof(vertex_t, position));
      vao_enable_attribute(mesh.vao, mesh.vbo, 1, 2, GL_FLOAT, sizeof(vertex_t), offsetof(vertex_t, tex0));
      vao_bind_ebo(mesh.vao, mesh.ebo);
      
      model.meshes.push_back(mesh);
    }

    models.push_back(model);
  } 

  // 4. store objects in internal node hierarchy
   
}
