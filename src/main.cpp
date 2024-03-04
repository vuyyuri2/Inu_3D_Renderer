#include <stdio.h>

#include "windowing/window.h"
#include "model_loading/model_internal.h"
#include "model_loading/gltf/gltf.h"
#include "gfx/gfx.h"
#include "utils/general.h"
#include "utils/app_info.h"
#include "utils/mats.h"
#include "gfx/online_renderer.h"

extern window_t window;
app_info_t app_info;

static float fb_width = 1280 / 1.f;
static float fb_height = 960 / 1.f;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

  mat4 a = create_matrix(2.0f);
  mat4 b = create_matrix(3.0f);
  mat4 c = mat_multiply_mat(a, b);

  create_window(hInstance, fb_width, fb_height);

  if (wcscmp(pCmdLine, L"running_in_vs") == 0) {
    app_info.running_in_vs = true;
  }

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

  std::vector<model_t> models;
  char gltf_full_file_path[256]{};
  sprintf(gltf_full_file_path, "%s\\%s", resources_path, gltf_file_resources_folder_rel_path);
  gltf_load_file(gltf_full_file_path, models);

  while (window.running) {
    poll_events();

    // offline rendering pass
    bind_framebuffer(offline_fb);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 proj = proj_mat(60.f, 0.1f, 1000.f, static_cast<float>(window.window_dim.x) / window.window_dim.y);
    shader_set_mat4(material_t::associated_shader, "projection", proj);

    vec3 t = { 0,-5,-100 };
    mat4 translate = translate_mat(t);
#if 0
    static float scale_val = 5.f;
    static float multiplier = 1.f;
    scale_val += multiplier * 0.0002f;
    float upper = 5.5f;
    float lower = 4.5f;
    if (scale_val > upper) {
      scale_val = upper;
      multiplier *= -1;
    } else if (scale_val < lower) {
      scale_val = lower;
      multiplier *= -1;
    }
#endif
    mat4 scale = scale_mat(5.f);
    mat4 model = mat_multiply_mat(translate, scale);
    shader_set_mat4(material_t::associated_shader, "model", model);

    for (model_t& model : models) {
      for (mesh_t& mesh : model.meshes) {
        bind_material(mesh.mat_idx);

        bind_vao(mesh.vao);
        draw_ebo(mesh.ebo);
        unbind_vao();
        unbind_ebo();
      }
    }
    unbind_shader();

    // online rendering pass
    render_online(offline_fb);

    swap_buffers();
  }
}
