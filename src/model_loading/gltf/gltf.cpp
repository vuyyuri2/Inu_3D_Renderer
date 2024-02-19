#include "gltf.h"

#include "utils/log.h"

/*
 https://github.com/KhronosGroup/glTF-Sample-Models/blob/main/2.0/README.md#showcase
 */

uint8_t* data = NULL;
int offset = -1;
int data_len = -1;

void gltf_parse_section() {
  inu_assert(gltf_peek() == "\"");
  gltf_eat();
  char* key_start = data;
  while (gltf_peek() != "\"") {
    gltf_eat();
  }
  data[offset] = 0;

  if (strcmp(key_start, "asset") == 0) {
    // skip logic
  } else if (strcmp(key_start, "nodes") == 0) {

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
  uint8_t* buffer = (uint8_t*)malloc(buffer_len * sizeof(uint8_t));
  FILE* gltf_file = fopen(filepath, "r");
  inu_assert(gltf_file, "gltf file not found");
  int count = 0;
  while (!feof(gltf_file)) {
    char c = getc(gltf_file);
    if (!isspace(c)) {
      buffer[count] = c;
      count++;
      if (count == buffer_len) {
        buffer_len *= 2;
        buffer = (uint8_t*)realloc(buffer, buffer_len);
      }
    }
    if (buffer_len > count) {
      memset(buffer + count, 0, buffer_len - count);
    }
  }
  fclose(gltf_file);

  data = buffer;
  data_len = count; 
  offset = 0;
}

void gltf_load_file(const char* filepath, mesh_t& mesh) {
  gltf_preprocess(filepath);
  inu_assert("{" == gltf_peek(), "initial character is not opening curly brace");
  gltf_eat();
  free(data);
  data = NULL;
}
