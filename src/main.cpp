#include <stdio.h>

#include "windowing/window.h"
#include "model_loading/model_internal.h"
#include "model_loading/gltf/gltf.h"
#include "gfx/gfx.h"

extern window_t window;


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  create_window(hInstance, 400, 300); 

  // shader_t shader = create_shader("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\shaders\\model.vert", "C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\shaders\\model.frag");
  material_t::associated_shader = create_shader("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\shaders\\model.vert", "C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\shaders\\model.frag");

  std::vector<model_t> models;
  // gltf_load_file("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\box\\Box.gltf", models);
  // gltf_load_file("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\box_interleaved\\BoxInterleaved.gltf", models);
  // gltf_load_file("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\box_textured\\BoxTextured.gltf", models);
  // gltf_load_file("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\box_textured_non_power_of_2\\BoxTexturedNonPowerOfTwo.gltf", models);
  gltf_load_file("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\box_with_spaces\\Box With Spaces.gltf", models);

  // gltf_load_file("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\duck\\Duck.gltf", models);
  // float angle = 0;
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
