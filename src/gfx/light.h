#pragma once

#include "gfx/gfx.h"
#include "utils/transform.h"
#include "utils/mats.h"

#define SHOW_LIGHTS 1

struct light_t {
  int id = -1;
  transform_t transform;
  float radius = 1.f;
  static const float NEAR_PLANE;
  static const float FAR_PLANE;
  static const float SHADOW_MAP_WIDTH;
  static const float SHADOW_MAP_HEIGHT;
  vec3 dir;
  vec3 color;

#if SHOW_LIGHTS
  static int LIGHT_MESH_ID;
#endif
  static shader_t light_shader;
  framebuffer_t light_pass_fb;

  mat4 view;
  mat4 proj;
};

void init_light_data();
int create_light(vec3 pos);
void setup_light_for_rendering(int light_id);
void remove_light_from_rendering();
light_t get_light(int light_id);
int get_num_lights();
GLuint get_light_fb_depth_tex(int light_id);
mat4 get_light_proj_mat(int light_id);
mat4 get_light_view_mat(int light_id);
