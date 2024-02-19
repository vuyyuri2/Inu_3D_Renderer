#include "gltf.h"

#include <ctype.h>

#include "utils/log.h"

/*
 https://github.com/KhronosGroup/glTF-Sample-Models/blob/main/2.0/README.md#showcase
 */

char* data = NULL;
int offset = -1;
int data_len = -1;

static std::vector<gltf_scene_t> gltf_scenes; 
static std::vector<gltf_node_t> gltf_nodes; 

void gltf_skip_section() {
  // skip logic
  
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
    inu_assert(gltf_peek() == '\"');
    gltf_eat();
    char* key_start = data + offset;
    while (gltf_peek() != '\"') {
      gltf_eat();
    }
    gltf_eat();
    data[offset-1] = 0;
    if (strcmp(key_start, "nodes") == 0) {
      // get the root nodes
      inu_assert(gltf_peek() == ':');
      gltf_eat();
      scene.root_nodes = gltf_parse_integer_array();
      if (gltf_peek() == ',') gltf_eat();
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
}

void gltf_parse_node() {
  gltf_node_t node;

  inu_assert(gltf_peek() == '{');
  gltf_eat();

  while (gltf_peek() != '}') {
    inu_assert(gltf_peek() == '\"');
    gltf_eat();
    char* key_start = data + offset;
    while (gltf_peek() != '\"') {
      gltf_eat();
    }
    gltf_eat();
    data[offset-1] = 0;
    if (strcmp(key_start, "children") == 0) {
      // get the root nodes
      inu_assert(gltf_peek() == ':');
      gltf_eat();
      node.child_node_handles = gltf_parse_integer_array();
      if (gltf_peek() == ',') gltf_eat();
    } else if (strcmp(key_start, "mesh") == 0) {
      inu_assert(gltf_peek() == ':');
      gltf_eat();
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
}

void gltf_parse_section() {
  inu_assert(gltf_peek() == '\"');
  gltf_eat();
  char* key_start = data + offset;
  while (gltf_peek() != '\"') {
    gltf_eat();
  }
  gltf_eat();
  data[offset-1] = 0;

  inu_assert(gltf_peek() == ':', "colon is not seen");
  gltf_eat();

  if (strcmp(key_start, "asset") == 0) {
    gltf_parse_asset_section();
  } else if (strcmp(key_start, "nodes") == 0) {
    gltf_parse_nodes_section();
  } else if (strcmp(key_start, "scenes") == 0) {
    gltf_parse_scenes_section();
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

void gltf_load_file(const char* filepath, mesh_t& mesh) {
  gltf_preprocess(filepath);
  inu_assert('{' == gltf_peek(), "initial character is not opening curly brace");
  gltf_eat();

  while (gltf_peek() != '}') {
    gltf_parse_section();
  }

  free(data);
  data = NULL;
}
