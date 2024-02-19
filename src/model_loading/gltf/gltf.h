#pragma once

#include "model_loading/model_internal.h"

void gltf_eat();
void gltf_parse_section();
char gltf_peek();
void gltf_preprocess(const char* filepath);
void gltf_load_file(const char* filepath, mesh_t& mesh);
