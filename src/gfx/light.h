#pragma once

#include "gfx/gfx.h"
#include "scene/transform.h"

#define SHOW_LIGHTS 1

struct light_t {
  int id = -1;
  transform_t transform;
  float radius = 1.f;
  vec3 dir;
  vec3 color;

#if SHOW_LIGHTS
  static int LIGHT_MESH_ID;
#endif
  static shader_t light_shader;
};

void init_light_data();
int create_light(vec3 pos);
void set_lights_in_shader();
light_t get_light();
