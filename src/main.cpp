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
#include "scene/transform.h"

extern window_t window;
app_info_t app_info;
extern animation_globals_t animation_globals;

static float fb_width = 1280 / 1.f;
static float fb_height = 960 / 1.f;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

  create_window(hInstance, fb_width, fb_height);

  if (wcscmp(pCmdLine, L"running_in_vs") == 0) {
    app_info.running_in_vs = true;
  }

  transform_t t;
  t.pos.z = 10.f;
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
  // const char* gltf_file_resources_folder_rel_path = "animated_cube\\AnimatedCube.gltf";
  // const char* gltf_file_resources_folder_rel_path = "box_animated\\BoxAnimated.gltf";
  // const char* gltf_file_resources_folder_rel_path = "two_cylinder_engine\\2CylinderEngine.gltf";
  // const char* gltf_file_resources_folder_rel_path = "box_with_spaces\\Box With Spaces.gltf";
  // const char* gltf_file_resources_folder_rel_path = "box_vertex_colors\\BoxVertexColors.gltf";
  // const char* gltf_file_resources_folder_rel_path = "cube_non_smooth_face\\Cube.gltf";
  // const char* gltf_file_resources_folder_rel_path = "duck\\Duck.gltf";
  // const char* gltf_file_resources_folder_rel_path = "avacado\\Avocado.gltf";
  // const char* gltf_file_resources_folder_rel_path = "suzan\\Suzanne.gltf";
  // const char* gltf_file_resources_folder_rel_path = "cartoon_car\\combined.gltf";
  // const char* gltf_file_resources_folder_rel_path = "stylized_ww1_plane\\scene.gltf";
  // const char* gltf_file_resources_folder_rel_path = "ferrari_enzo\\scene.gltf";
  // const char* gltf_file_resources_folder_rel_path = "buggy\\Buggy.gltf";
  // const char* gltf_file_resources_folder_rel_path = "stylized_mushrooms\\scene.gltf";
  // const char* gltf_file_resources_folder_rel_path = "little_chestnut\\scene.gltf";
  // const char* gltf_file_resources_folder_rel_path = "milk_truck\\CesiumMilkTruck.gltf";
  // const char* gltf_file_resources_folder_rel_path = "rigged_simple\\RiggedSimple.gltf";
  // const char* gltf_file_resources_folder_rel_path = "rigged_figure\\RiggedFigure.gltf";
  // const char* gltf_file_resources_folder_rel_path = "rigged_figure\\blender_export.gltf";
  // const char* gltf_file_resources_folder_rel_path = "cesium_man\\CesiumMan.gltf";
  // const char* gltf_file_resources_folder_rel_path = "brain_stem\\BrainStem.gltf";
  // const char* gltf_file_resources_folder_rel_path = "fox\\Fox.gltf";
  // const char* gltf_file_resources_folder_rel_path = "virtual_city\\VC.gltf";
  // const char* gltf_file_resources_folder_rel_path = "medieval_fantasy_book\\scene.gltf";
  // const char* gltf_file_resources_folder_rel_path = "low-poly_truck_car_drifter\\scene.gltf";
  // const char* gltf_file_resources_folder_rel_path = "yusuke_urameshi\\scene.gltf";
  // const char* gltf_file_resources_folder_rel_path = "junkrat\\scene.gltf";
  const char* gltf_file_resources_folder_rel_path = "reap_the_whirlwind\\scene.gltf";

  if (strcmp(gltf_file_resources_folder_rel_path, "stylized_ww1_plane\\scene.gltf") == 0
      || strcmp(gltf_file_resources_folder_rel_path, "ferrari_enzo\\scene.gltf") == 0 //) {
      || strcmp(gltf_file_resources_folder_rel_path, "rigged_figure\\RiggedFigure.gltf") == 0) {
    app_info.render_only_textured = true;
  }

#if SHOW_BONES
  char bone_mesh_full_file_path[256]{};
  // this file pretty much just has a mesh, no nodes
  sprintf(bone_mesh_full_file_path, "%s\\bone_mesh\\custom_bone_mesh.gltf", resources_path);
  gltf_load_file(bone_mesh_full_file_path);
  skin_t::BONE_MODEL_ID = latest_model_id();
#endif

  char gltf_full_file_path[256]{};
  sprintf(gltf_full_file_path, "%s\\%s", resources_path, gltf_file_resources_folder_rel_path);
  gltf_load_file(gltf_full_file_path);

  play_next_anim();

 
  while (window.running) {
    inu_timer_t frame_timer;
    start_timer(frame_timer);

    // WINDOW + INPUT PASS
    poll_events();

    // UPDATE PASS
#if 1
    if (window.input.right_mouse_up) {
      play_next_anim();
    }
#endif
    
    update_cam();
    update_animations();

    // RENDERING PASS

    // offline rendering pass
    bind_framebuffer(offline_fb);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 proj = proj_mat(60.f, 0.01f, 1000.f, static_cast<float>(window.window_dim.x) / window.window_dim.y);
    shader_set_mat4(material_t::associated_shader, "projection", proj);

    mat4 view = get_view_mat();
    shader_set_mat4(material_t::associated_shader, "view", view);
 
    render_scene();

    // online rendering pass
    render_online(offline_fb);

    swap_buffers();

    end_timer(frame_timer);
    app_info.delta_time = frame_timer.elapsed_time_sec;
  }
}
