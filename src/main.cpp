#include <stdio.h>

#include "windowing/window.h"
#include "model_loading/model_internal.h"
#include "model_loading/gltf/gltf.h"
#include "gfx/gfx.h"

extern window_t window;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  create_window(hInstance, 400, 300); 

  shader_t shader = create_shader("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\shaders\\model.vert", "C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\shaders\\model.frag");

  std::vector<model_t> models;
  // gltf_load_file("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\box\\Box.gltf", models);
  gltf_load_file("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\box_interleaved\\BoxInterleaved.gltf", models);
  float angle = 0;
  while (window.running) {
    poll_events();

    glClearColor(1.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_set_float(shader, "angle", angle);
    angle += 0.01f;
    if (angle > 360.f) {
      angle -= 360.f;
    }
    
    vec3 color;
    if (angle > 180) {
      color.x = 0;
      color.y = 1;
      color.z = 0;
    } else {
      color.x = 0;
      color.y = 0;
      color.z = 1;
    }
    shader_set_vec3(shader, "in_color", color);

    bind_shader(shader);
    for (model_t& model : models) {
      for (mesh_t& mesh : model.meshes) {
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
