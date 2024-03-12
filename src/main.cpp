#include <stdio.h>

#include "windowing/window.h"
#include "model_loading/model_internal.h"
#include "model_loading/gltf/gltf.h"
#include "gfx/gfx.h"
#include "gfx/online_renderer.h"
#include "scene/scene.h"
#include "scene/camera.h"
#include "utils/general.h"
#include "utils/app_info.h"
#include "utils/mats.h"
#include "utils/quaternion.h"

extern window_t window;
app_info_t app_info;

// static float fb_width = 1280 / 2.f;
// static float fb_height = 960 / 2.f;
static float fb_width = 1280 / 1.f;
static float fb_height = 960 / 1.f;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

#if 0
  quaternion_t q1 = create_quaternion(1,0,0,0);
  quaternion_t q2 = create_quaternion(-1,0,0,0);
  quaternion_t f = quat_multiply_quat(q1,q2);
#endif

#if 1

  struct vec3_test_combo_t {
    vec3 v1;
    vec3 v2;
    vec3 expected;
  };

#define NUM_TESTS 3

  vec3_test_combo_t tests[NUM_TESTS] = {
    {{0,1,0}, {1,0,0}, {0,0,-1}},
    {{1,1,0}, {-1,0,-1}, {-1,1,1}},
    {{-0.5,1,0}, {-1,1,-1}, {-1,-0.5,0.5}}
  };

  for (int i = 0; i < NUM_TESTS; i++) {
    vec3 v1 = tests[i].v1;
    vec3 v2 = tests[i].v2;
    vec3 f = cross_product(v1, v2);
    inu_assert(f.x == tests[i].expected.x);
    inu_assert(f.y == tests[i].expected.y);
    inu_assert(f.z == tests[i].expected.z);
  }
#undef NUM_TESTS

#endif

  create_window(hInstance, fb_width, fb_height);

  if (wcscmp(pCmdLine, L"running_in_vs") == 0) {
    app_info.running_in_vs = true;
  }

  transform_t t;
  t.pos.z = 20.f;
  create_camera(t);
  init_online_renderer();

  char resources_path[256]{};
  get_resources_folder_path(resources_path);
  printf("resources_path: %s\n", resources_path);

  framebuffer_t offline_fb = create_framebuffer(fb_width, fb_height);

  char vert_shader_path[256]{};
  sprintf(vert_shader_path, "%s\\shaders\\model.vert", resources_path);
  char frag_shader_path[256]{};
  sprintf(frag_shader_path, "%s\\shaders\\model.frag", resources_path);
  material_t::associated_shader = create_shader(vert_shader_path, frag_shader_path); 

  // const char* gltf_file_resources_folder_rel_path =  "box\\Box.gltf";
  // const char* gltf_file_resources_folder_rel_path =  "box_interleaved\\BoxInterleaved.gltf";
  // const char* gltf_file_resources_folder_rel_path = "box_textured\\BoxTextured.gltf";
  // const char* gltf_file_resources_folder_rel_path = "box_textured_non_power_of_2\\BoxTexturedNonPowerOfTwo.gltf";
  // const char* gltf_file_resources_folder_rel_path = "box_with_spaces\\Box With Spaces.gltf";
  // const char* gltf_file_resources_folder_rel_path = "box_vertex_colors\\BoxVertexColors.gltf";
  // const char* gltf_file_resources_folder_rel_path = "cube_non_smooth_face\\Cube.gltf";
  // const char* gltf_file_resources_folder_rel_path = "duck\\Duck.gltf";
  // const char* gltf_file_resources_folder_rel_path = "avacado\\Avocado.gltf";
  // const char* gltf_file_resources_folder_rel_path = "suzan\\Suzanne.gltf";
  const char* gltf_file_resources_folder_rel_path = "cartoon_car\\combined.gltf";

  char gltf_full_file_path[256]{};
  sprintf(gltf_full_file_path, "%s\\%s", resources_path, gltf_file_resources_folder_rel_path);
  gltf_load_file(gltf_full_file_path);

  while (window.running) {
    poll_events();

    // offline rendering pass
    bind_framebuffer(offline_fb);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 proj = proj_mat(60.f, 0.1f, 1000.f, static_cast<float>(window.window_dim.x) / window.window_dim.y);
    shader_set_mat4(material_t::associated_shader, "projection", proj);

    static float angle = 0;
    quaternion_t new_q = create_quaternion_w_rot({0,1,0}, angle);
    // angle += 0.05f;
    // mat4 view = get_view_mat(new_q);
    
    // vec3 diff = {0,0,-window.input.scroll_wheel_delta};
    //
    if (window.input.scroll_wheel_delta != 0) {
      cam_move_forward(window.input.scroll_wheel_delta);
    }

    bool moved = length(window.input.mouse_pos_diff) != 0 && window.input.middle_mouse_down;
    if (moved) {
      // cam_move_forward(window.input.scroll_wheel_delta);
      float lat = window.input.mouse_pos_diff.x * -2.f;
      float vert = window.input.mouse_pos_diff.y * 1.f;
      // cam_move_rotate(0, window.input.scroll_wheel_delta * 10.f);
      cam_move_rotate(lat, vert);
    }
    mat4 view = get_view_mat();
    shader_set_mat4(material_t::associated_shader, "view", view);
 
    render_scene();

    // online rendering pass
    render_online(offline_fb);

    swap_buffers();
  }
}
