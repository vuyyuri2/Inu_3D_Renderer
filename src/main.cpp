#include <stdio.h>

#include "windowing/window.h"
#include "model_loading/model_internal.h"
#include "model_loading/gltf/gltf.h"

extern window_t window;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  create_window(hInstance, 400, 300); 

  mesh_t mesh;
  gltf_load_file("C:\\Sarthak\\projects\\3d_anim_renderer\\resources\\box\\Box.gltf", mesh);
  while (window.running) {
    poll_events();
    swap_buffers();
  }
}
