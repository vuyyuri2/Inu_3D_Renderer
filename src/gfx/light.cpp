#include "light.h"

#include "gfx/gfx.h"
#include "windowing/window.h"
#include "utils/general.h"
#include "scene/scene.h"
#include "scene/camera.h"
#include "model_loading/gltf/gltf.h"
#include "scene/scene.h"

#include <vector>

extern window_t window;

static std::vector<light_t> lights;
static std::vector<dir_light_t> dir_lights;

#if SHOW_LIGHTS
int light_t::LIGHT_MESH_ID = -1;
#endif

extern float fb_width;
extern float fb_height;

shader_t light_t::light_shader;
const float light_t::NEAR_PLANE = 0.1f;
const float light_t::FAR_PLANE = 50.f;
const float light_t::SHADOW_MAP_WIDTH = fb_width / 4.f;
const float light_t::SHADOW_MAP_HEIGHT = fb_height / 4.f;

const float dir_light_t::SHADOW_MAP_WIDTH = 4096.f;
const float dir_light_t::SHADOW_MAP_HEIGHT = 4096.f;
const int NUM_SM_CASCADES = 3;
shader_t dir_light_t::light_shader;

void init_light_data() {
  char resources_path[256]{};
  get_resources_folder_path(resources_path);
  char vert_shader_path[256]{};
  sprintf(vert_shader_path, "%s\\shaders\\light.vert", resources_path);
  char frag_shader_path[256]{};
  sprintf(frag_shader_path, "%s\\shaders\\light.frag", resources_path);
  light_t::light_shader = create_shader(vert_shader_path, frag_shader_path);

#if SHOW_LIGHTS
  char light_mesh_full_file_path[256]{};
  // this file pretty much just has a mesh, no nodes
  sprintf(light_mesh_full_file_path, "%s\\custom_light\\light_mesh.gltf", resources_path);
  gltf_load_file(light_mesh_full_file_path);
  light_t::LIGHT_MESH_ID = latest_model_id();
#endif

  memset(vert_shader_path, 0, sizeof(vert_shader_path));
  memset(frag_shader_path, 0, sizeof(frag_shader_path));
  sprintf(vert_shader_path, "%s\\shaders\\dir_light.vert", resources_path);
  sprintf(frag_shader_path, "%s\\shaders\\dir_light.frag", resources_path);
  dir_light_t::light_shader = create_shader(vert_shader_path, frag_shader_path);
}

int create_light(vec3 pos) {
  if (lights.size() >= NUM_LIGHTS_SUPPORTED_IN_SHADER) {
    char buffer[256]{};
    sprintf(buffer, "can't support more than %i", NUM_LIGHTS_SUPPORTED_IN_SHADER);
    inu_assert_msg(buffer);
  }
  light_t light;
  light.id = lights.size();
  light.transform.pos = pos;
  light.dir = {0,-1,0};
  light.light_pass_fb = create_framebuffer(light_t::SHADOW_MAP_WIDTH, light_t::SHADOW_MAP_HEIGHT, true);
  lights.push_back(light);
  transform_t obj_t;
  obj_t.pos = pos;
  obj_t.scale = {1,1,1};
  int obj_id = create_object(obj_t);
  attach_name_to_obj(obj_id, std::string("light pos"));
  set_obj_as_parent(obj_id);
#if SHOW_LIGHTS
  attach_model_to_obj(obj_id, light_t::LIGHT_MESH_ID);
#endif

  return light.id;
}

void set_lights_in_shader() {
  light_t& light = lights[0];
  // shader_set_vec3(light_t::light_shader, "light.pos", light.transform.pos);
}

light_t get_light(int light_id) {
  return lights[light_id];
}

int get_num_lights() {
  return lights.size();
}

void setup_light_for_rendering(int light_id) {
  light_t& light = lights[light_id];

  bind_framebuffer(light.light_pass_fb);
  clear_framebuffer(light.light_pass_fb);

  vec3 fp = {light.transform.pos.x, light.transform.pos.y - 1, light.transform.pos.z};
  // fp.x = ;
  light.view = get_view_mat(light.transform.pos, fp);
  // float light_near_plane = 0.1f;
  // float light_far_plane = 50.f;
  light.proj = proj_mat(60.f, light_t::NEAR_PLANE, light_t::FAR_PLANE, static_cast<float>(window.window_dim.x) / window.window_dim.y);
  shader_set_mat4(light_t::light_shader, "light_view", light.view);
  shader_set_mat4(light_t::light_shader, "light_projection", light.proj); 

  bind_shader(light_t::light_shader);
}

void remove_light_from_rendering() {
  unbind_shader();
  unbind_framebuffer();
}

GLuint get_light_fb_depth_tex(int light_id) {
  return lights[light_id].light_pass_fb.depth_att;
}

mat4 get_light_proj_mat(int light_id) {
  return lights[light_id].proj;
}

mat4 get_light_view_mat(int light_id) {
  return lights[light_id].view;
}

vec3 get_light_pos(int light_id) {
  return lights[light_id].transform.pos;
}

int create_dir_light(vec3 dir) {
  dir_light_t light;
  light.dir = dir;
  light.id = dir_lights.size();
  dir_lights.push_back(light);
  return light.id;
}

struct view_frustum_t {
  vec3 frustum_corners[6]{};
};

void setup_dir_light_for_rendering(int light_id, camera_t* camera) {
  light_t& light = lights[light_id];

  bind_framebuffer(light.light_pass_fb);
  clear_framebuffer(light.light_pass_fb);

  vec4 cam_frustum_ndc_corners[6] = {
    // back bottom left
    {-1,-1,-1,1},
    // front bottom left
    {-1,-1,1,1},
    // back top left
    {-1,1,-1,1},
    // front top left 
    {-1,1,1,1},
    // back bottom right
    {1,-1,-1,1},
    // front bottom right
    {1,-1,1,1},
    // back top right
    {1,1,-1,1},
    // front top right
    {1,1,1,1}
  };
  view_frustum_t world_cam_frustum;
  for (int i = 0; i < 6; i++) {
    mat4 c = mat_multiply_mat(mat_camera->projection, camera->view);
    vec4 world_unnorm = mat_multiply_vec(mat_inverse(c), cam_frustum_ndc_corners[i]);
    world_norm = world_unnorm / world_unnorm.w;
    world_cam_frustum.frustum_corners[i] = {world_norm.x, world_norm.y, world_norm.z};
  }

  int N = dir_light_t::NUM_SM_CASCADES;
  view_frustum_t cascaded_frustums[N];
  for (int i = 0; i < dir_light_t::NUM_SM_CASCADES; i++) {
    float n = camera->near_plane;
    float f = camera->far_plane;

    float z_near = (0.5f*n*pow(f/n, i/N)) + (0.5f*(n+(i/N*(f-n))));
    float z_far = (0.5f*n*pow(f/n, (i+1)/N)) + (0.5f*(n+((i+1)/N*(f-n))));
    

  }

  vec3 fp = {0,0,0};
  light.view = get_view_mat(light.transform.pos, fp);
  shader_set_mat4(light_t::light_shader, "light_view", light.view);
  shader_set_mat4(light_t::light_shader, "light_projection", light.proj); 

  bind_shader(light_t::light_shader);
}
