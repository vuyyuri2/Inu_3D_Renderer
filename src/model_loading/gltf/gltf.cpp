#include "gltf.h"

#include <ctype.h>
#include <unordered_set>

#include "utils/log.h"

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

    if (peeked == opening) {
      looking_for_comma = false;
      num_opening_seen++;
    } else if (peeked == closing) {
      num_opening_seen--;
      if (looking_for_comma) {
        break;
      }
    }
    gltf_eat();
  }
  if (!looking_for_comma && gltf_peek() == ',') {
    gltf_eat();
  }
}

void gltf_parse_asset_section() {
  while (gltf_peek() != '}') {
    gltf_eat();
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

void gltf_parse_scene() {

  gltf_scene_t scene;

  inu_assert(gltf_peek() == '{');
  gltf_eat();
  while (gltf_peek() != '}') {
#if 0
    inu_assert(gltf_peek() == '\"');
    gltf_eat();
    char* key_start = data + offset;
    while (gltf_peek() != '\"') {
      gltf_eat();
    }
    gltf_eat();
    data[offset-1] = 0;
#else
    std::string key = gltf_parse_string();
#endif

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

#if 0
    inu_assert(gltf_peek() == '\"');
    gltf_eat();
    char* key_start = data + offset;
    while (gltf_peek() != '\"') {
      gltf_eat();
    }
    gltf_eat();
    data[offset-1] = 0;
#else
    std::string key = gltf_parse_string();
#endif

    inu_assert(gltf_peek() == ':');
    gltf_eat();

    if (key == "children") {
      // get the root nodes
      node.child_node_idxs = gltf_parse_integer_array();
      if (gltf_peek() == ',') gltf_eat();
    } else if (key == "mesh") {
#if 0
      char* mesh_idx_str = data + offset;
      while (gltf_peek() != '}' && gltf_peek() != ',') {
        gltf_eat();
      }
      if (gltf_peek() == ',') {
        gltf_eat();
        data[offset-1] = 0;
        node.gltf_mesh_handle = atoi(mesh_idx_str);
      } else {
        data[offset] = 0;
        node.gltf_mesh_handle = atoi(mesh_idx_str);
        data[offset] = '}';
      }
#else
      node.gltf_mesh_handle = gltf_parse_integer();
      if (gltf_peek() == ',') gltf_eat();
#endif
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
#if 0
    char* key_start = data + offset;
    while (gltf_peek() != '\"') {
      gltf_eat();
    }
    gltf_eat();
    data[offset-1] = 0;
#else
    std::string key = gltf_parse_string();
#endif

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

void gltf_parse_section() {

#if 0
  inu_assert(gltf_peek() == '\"');
  gltf_eat();
  char* key_start = data + offset;
  while (gltf_peek() != '\"') {
    gltf_eat();
  }
  gltf_eat();
  data[offset-1] = 0;
#else
  std::string key = gltf_parse_string();
#endif

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
  fseek(uri_file, start_offset, SEEK_SET);

  for (int i = 0; i < size_of_acc_data; i++) {
    data[i] = fgetc(uri_file);
  }
  
  fclose(uri_file);
  
  return (void*)data;
}

void gltf_load_file(const char* filepath, mesh_t& mesh) {


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
  const char* last_slash = strrchr(filepath, '\\');
  memset(folder_path, 0, 256);
  memcpy(folder_path, filepath, last_slash - filepath);
  for (gltf_mesh_t& gltf_mesh : gltf_meshes) {
    for (gltf_primitive_t& prim : gltf_mesh.primitives) {
      if (prim.indicies_accessor_idx != -1) {
        void* index_data = gltf_read_accessor_data(prim.indicies_accessor_idx);
      }
      if (prim.attribs.normals_accessor_idx != -1) {
        void* normals_data = gltf_read_accessor_data(prim.attribs.normals_accessor_idx);
      }
      if (prim.attribs.positions_accessor_idx != -1) {
        void* positions_data = gltf_read_accessor_data(prim.attribs.positions_accessor_idx);
      }
    }
  }

  // 4. store objects in internal node hierarchy
    
}
