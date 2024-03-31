#include "light.h"

#include "gfx/gfx.h"
#include "windowing/window.h"
#include "utils/general.h"
#include "scene/scene.h"
#include "scene/camera.h"
#include "model_loading/gltf/gltf.h"
#include "scene/scene.h"
#include "animation/interpolation.h"

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
shader_t dir_light_t::light_shader;
shader_t dir_light_t::debug_shader;

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

  char geom_shader_path[256]{};

  memset(vert_shader_path, 0, sizeof(vert_shader_path));
  memset(geom_shader_path, 0, sizeof(geom_shader_path));
  memset(frag_shader_path, 0, sizeof(frag_shader_path));
  sprintf(vert_shader_path, "%s\\shaders\\dir_light.vert", resources_path);
  sprintf(geom_shader_path, "%s\\shaders\\dir_light.geo", resources_path);
  sprintf(frag_shader_path, "%s\\shaders\\dir_light.frag", resources_path);
  dir_light_t::light_shader = create_shader(vert_shader_path, geom_shader_path, frag_shader_path);

  memset(vert_shader_path, 0, sizeof(vert_shader_path));
  memset(frag_shader_path, 0, sizeof(frag_shader_path));
  sprintf(vert_shader_path, "%s\\shaders\\light.vert", resources_path);
  sprintf(frag_shader_path, "%s\\shaders\\light.frag", resources_path);
  dir_light_t::debug_shader = create_shader(vert_shader_path, frag_shader_path);
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
  light.light_pass_fb = create_framebuffer(light_t::SHADOW_MAP_WIDTH, light_t::SHADOW_MAP_HEIGHT, FB_TYPE::TEXTURE_DEPTH_STENCIL);
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
  light.view = get_view_mat(light.transform.pos, fp);
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
  light.dir = norm_vec3(dir);
  light.id = dir_lights.size();
  light.debug_light_pass_fbs[0] = create_framebuffer(dir_light_t::SHADOW_MAP_WIDTH, dir_light_t::SHADOW_MAP_HEIGHT, FB_TYPE::TEXTURE_DEPTH_STENCIL);
  light.debug_light_pass_fbs[1] = create_framebuffer(dir_light_t::SHADOW_MAP_WIDTH, dir_light_t::SHADOW_MAP_HEIGHT, FB_TYPE::TEXTURE_DEPTH_STENCIL);
  light.debug_light_pass_fbs[2] = create_framebuffer(dir_light_t::SHADOW_MAP_WIDTH, dir_light_t::SHADOW_MAP_HEIGHT, FB_TYPE::TEXTURE_DEPTH_STENCIL);
  light.light_pass_fb = create_framebuffer(dir_light_t::SHADOW_MAP_WIDTH, dir_light_t::SHADOW_MAP_HEIGHT, FB_TYPE::NO_COLOR_ATT_MULTIPLE_DEPTH_TEXTURE);
  dir_lights.push_back(light);
  return light.id;
}

#define NUM_CUBE_CORNERS 8
struct frustum_t {
  vec3 frustum_corners[NUM_CUBE_CORNERS]{};
};

void gen_dir_light_matricies(int light_id, camera_t* camera) {
  dir_light_t& dir_light = dir_lights[light_id];

  // calculate camera frustum in world coordinates
#if 1
  frustum_t cam_frustum_ndc_corners = {
    {
      // near bottom left
      {-1,-1,-1},
      // far bottom left
      {-1,-1,1},
      // near top left
      {-1,1,-1},
      // far top left 
      {-1,1,1},
      // near bottom right
      {1,-1,-1},
      // far bottom right
      {1,-1,1},
      // near top right
      {1,1,-1},
      // far top right
      {1,1,1}
    }
  };
#else
  vec4 cam_frustum_ndc_corners[NUM_CUBE_CORNERS] = {
    // near bottom left
    {-1,-1,-1,1},
    // far bottom left
    {-1,-1,1,1},
    // near top left
    {-1,1,-1,1},
    // far top left 
    {-1,1,1,1},
    // near bottom right
    {1,-1,-1,1},
    // far bottom right
    {1,-1,1,1},
    // near top right
    {1,1,-1,1},
    // far top right
    {1,1,1,1}
  };
#endif
  frustum_t world_cam_frustum;
  mat4 cam_view = get_cam_view_mat();
  mat4 cam_proj = get_cam_proj_mat();
  for (int i = 0; i < NUM_CUBE_CORNERS; i++) {
    mat4 c = mat_multiply_mat(cam_proj, cam_view);
#if 1
    vec4 corner = vec4(cam_frustum_ndc_corners.frustum_corners[i], 1.0f);
    vec4 world_unnorm = mat_multiply_vec(mat4_inverse(c), corner);
#else
    vec4 world_unnorm = mat_multiply_vec(mat4_inverse(c), cam_frustum_ndc_corners[i]);
#endif
    vec4 world_norm = world_unnorm / world_unnorm.w;
    world_cam_frustum.frustum_corners[i] = {world_norm.x, world_norm.y, world_norm.z};
  }

  // split camera frustum in cascades
  const int N = NUM_SM_CASCADES;
  const float N_f = static_cast<float>(N);
  frustum_t cascaded_frustums[N];
  for (int i = 0; i < NUM_SM_CASCADES; i++) {
    frustum_t& vf = cascaded_frustums[i];

    float n = camera->near_plane;
    float f = camera->far_plane;

    float z_near = (0.5f*n*pow(f/n, i/N_f)) + (0.5f*(n+(i/N_f*(f-n))));
    float z_far = (0.5f*n*pow(f/n, (i+1)/N_f)) + (0.5f*(n+((i+1)/N_f*(f-n))));

    dir_light.cacade_depths[i] = z_near;
    dir_light.cacade_depths[i+1] = z_far;
    
    float zs[2] = {z_near, z_far};
    for (int i = 0; i < 2; i++) {
      float inter_n = (z_near - n) / (f - n);
      float inter_f = (z_far - n) / (f - n);

      for (int j = 0; j < NUM_CUBE_CORNERS / 2; j++) {
        int fc_near_idx = j*2;
        int fc_far_idx = fc_near_idx + 1;
        vf.frustum_corners[fc_near_idx] = vec3_linear(world_cam_frustum.frustum_corners[fc_near_idx], world_cam_frustum.frustum_corners[fc_far_idx], inter_n);
        vf.frustum_corners[fc_far_idx] = vec3_linear(world_cam_frustum.frustum_corners[fc_near_idx], world_cam_frustum.frustum_corners[fc_far_idx], inter_f);
      }
    }
  }

  // calculate view and ortho mats for each cascaded frustum
  for (int cascade = 0; cascade < NUM_SM_CASCADES; cascade++) {
    // get center
    frustum_t& vf = cascaded_frustums[cascade];
    vec3 frustum_center{};
    for (int i = 0; i < NUM_CUBE_CORNERS; i++) {
      frustum_center.x += vf.frustum_corners[i].x;
      frustum_center.y += vf.frustum_corners[i].y;
      frustum_center.z += vf.frustum_corners[i].z;
    }
    frustum_center.x /= static_cast<float>(NUM_CUBE_CORNERS);
    frustum_center.y /= static_cast<float>(NUM_CUBE_CORNERS);
    frustum_center.z /= static_cast<float>(NUM_CUBE_CORNERS);

    // calc view mat
    dir_light.light_views[cascade] = get_view_mat(frustum_center, vec3_add(frustum_center, dir_light.dir));

    // get mins and maxs
#if 0 
    float x_min = vf.frustum_corners[0].x, x_max = vf.frustum_corners[0].x;
    float y_min = vf.frustum_corners[0].y, y_max = vf.frustum_corners[0].y;
    float z_min = vf.frustum_corners[0].z, z_max = vf.frustum_corners[0].z;
#else
    float x_min = FLT_MAX, x_max = -FLT_MAX;
    float y_min = FLT_MAX, y_max = -FLT_MAX;
    float z_min = FLT_MAX, z_max = -FLT_MAX;
#endif
    for (int i = 0; i < NUM_CUBE_CORNERS; i++) {
      vec4 pt = {vf.frustum_corners[i].x, vf.frustum_corners[i].y, vf.frustum_corners[i].z, 1.0f};
      vec4 light_rel_view_pt = mat_multiply_vec(dir_light.light_views[cascade], pt);
      x_min = min(x_min, light_rel_view_pt.x);
      x_max = max(x_max, light_rel_view_pt.x);
      y_min = min(y_min, light_rel_view_pt.y);
      y_max = max(y_max, light_rel_view_pt.y);
      z_min = min(z_min, light_rel_view_pt.z);
      z_max = max(z_max, light_rel_view_pt.z);
    }
    // z_min and z_max will likely both be neative since we are looking down the negative z axis
    float z_multiplier = 1.f;
    if (z_max < 0) {
      z_max = z_max / z_multiplier;
    } else {
      z_max = z_max * z_multiplier;
    }

    if (z_min < 0) {
      z_min = z_min * z_multiplier;
    } else {
      z_min = z_min / z_multiplier;
    }

    // calc ortho mat
    dir_light.light_orthos[cascade] = ortho_mat(x_min, x_max, y_min, y_max, z_min, z_max);
  }
}

void setup_dir_light_for_rendering(int light_id, camera_t* camera) {
  dir_light_t& dir_light = dir_lights[light_id];

  bind_framebuffer(dir_light.light_pass_fb);
  clear_framebuffer(dir_light.light_pass_fb);

  
  gen_dir_light_matricies(light_id, camera);

#if 0
  vec3 fp = {0,0,0};
  light.view = get_view_mat(light.transform.pos, fp);
  shader_set_mat4(light_t::light_shader, "light_view", light.view);
  shader_set_mat4(light_t::light_shader, "light_projection", light.proj); 
#endif

  // set up shader uniforms
  for (int i = 0; i < NUM_SM_CASCADES; i++) {

#if 0
    mat4 light_space_mat = mat_multiply_mat(dir_light.light_orthos[i], dir_light.light_views[i]);
    char var_name[256]{};
    sprintf(var_name, "lightSpaceMatrices[%i]", i);
    shader_set_mat4(dir_light_t::light_shader, var_name, dir_light.light_views[i]);
#else

    char var_name[256]{};
    sprintf(var_name, "light_views[%i]", i);
    shader_set_mat4(dir_light_t::light_shader, var_name, dir_light.light_views[i]);

    memset(var_name, 0, sizeof(var_name));
    sprintf(var_name, "light_projs[%i]", i);
    shader_set_mat4(dir_light_t::light_shader, var_name, dir_light.light_orthos[i]);
#endif
  }
  bind_shader(dir_light_t::light_shader);
}

void remove_dir_light_from_rendering() {
  unbind_shader();
  unbind_framebuffer();
}

void setup_dir_light_for_rendering_debug(int light_id, camera_t* camera, int cascade) {
  dir_light_t& dir_light = dir_lights[light_id];

  bind_framebuffer(dir_light.debug_light_pass_fbs[cascade]);
  clear_framebuffer(dir_light.debug_light_pass_fbs[cascade]);

  gen_dir_light_matricies(light_id, camera);

  // set up shader uniforms
  shader_set_mat4(dir_light_t::debug_shader, "light_view", dir_light.light_views[cascade]);
  shader_set_mat4(dir_light_t::debug_shader, "light_projection", dir_light.light_orthos[cascade]);
  bind_shader(dir_light_t::debug_shader);
}

void remove_dir_light_from_rendering_debug() {
  unbind_shader();
  unbind_framebuffer();
}

dir_light_t* get_dir_light(int id) {
  return &dir_lights[id];
}
