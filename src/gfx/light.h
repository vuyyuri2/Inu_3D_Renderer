#pragma once

#include "gfx/gfx.h"
#include "utils/transform.h"
#include "utils/mats.h"
#include "utils/vectors.h"

#define SHOW_LIGHTS 1
#define NUM_SM_CASCADES 3

// will hopefully use shadow volumes at some pt
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
vec3 get_light_pos(int light_id);

// will use cascading shadow maps
struct dir_light_t {
  int id = -1;
  static const float SHADOW_MAP_WIDTH;
  static const float SHADOW_MAP_HEIGHT;
  vec3 dir;

  static shader_t light_shader;
  static shader_t debug_shader;

  framebuffer_t debug_light_pass_fbs[NUM_SM_CASCADES];
  // framebuffer_t debug_light_pass_fb1;
  // framebuffer_t debug_light_pass_fb2;

  framebuffer_t light_pass_fb;

  mat4 light_views[NUM_SM_CASCADES];
  mat4 light_orthos[NUM_SM_CASCADES];
};

int create_dir_light(vec3 dir);
struct camera_t;
void setup_dir_light_for_rendering(int light_id, camera_t* camera);
void remove_dir_light_from_rendering();

void setup_dir_light_for_rendering_debug(int light_id, camera_t* camera, int cascade);
void remove_dir_light_from_rendering_debug();
