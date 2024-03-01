#include <stdio.h>

#include "windowing/window.h"
#include "model_loading/model_internal.h"
#include "model_loading/gltf/gltf.h"
#include "gfx/gfx.h"
#include "utils/general.h"
#include "utils/app_info.h"

extern window_t window;
app_info_t app_info;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

  create_window(hInstance, 400, 300); 

  if (wcscmp(pCmdLine, L"running_in_vs") == 0) {
    app_info.running_in_vs = true;
  }

  char resources_path[256]{};
  get_resources_folder_path(resources_path);
  printf("resources_path: %s\n", resources_path);

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
  const char* gltf_file_resources_folder_rel_path = "duck\\Duck.gltf";

  std::vector<model_t> models;
  char gltf_full_file_path[256]{};
  sprintf(gltf_full_file_path, "%s\\%s", resources_path, gltf_file_resources_folder_rel_path);
  gltf_load_file(gltf_full_file_path, models);

  while (window.running) {
    poll_events();

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    swap_buffers();
  }
}
