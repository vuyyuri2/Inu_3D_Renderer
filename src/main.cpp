#include <stdio.h>

#include "windowing/window.h"
#include "model_loading/model_internal.h"
#include "model_loading/gltf/gltf.h"
#include "gfx/gfx.h"
#include "utils/general.h"
#include "utils/app_info.h"
#include "gfx/online_renderer.h"

extern window_t window;
app_info_t app_info;

static float fb_width = 1280;
static float fb_height = 960;
// static float fb_width = 400;
// static float fb_height = 300;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

  create_window(hInstance, fb_width, fb_height);
  // create_window(hInstance, 400, 300);

  if (wcscmp(pCmdLine, L"running_in_vs") == 0) {
    app_info.running_in_vs = true;
  }

  init_online_renderer();

  char resources_path[256]{};
  get_resources_folder_path(resources_path);
  printf("resources_path: %s\n", resources_path);

#if 0
  char vert_offline_to_online_path[256]{};
  sprintf(vert_offline_to_online_path, "%s\\shaders\\offline_to_online.vert", resources_path);
  char frag_offline_to_online_path[256]{};
  sprintf(frag_offline_to_online_path, "%s\\shaders\\offline_to_online.frag", resources_path);
  shader_t offline_to_online_shader = create_shader(vert_offline_to_online_path, frag_offline_to_online_path);
  shader_set_int(offline_to_online_shader, "img", 0);

  mesh_t offline_to_online_quad; 
  offline_to_online_vertex_t verts[4];
  unsigned int indicies[6] {
    0,2,3,
    1,2,0
  };

  offline_to_online_quad.vao = create_vao();
  offline_to_online_quad.vbo = create_dyn_vbo(sizeof(verts));
  // update_vbo_data(offline_to_online_quad.vbo, (float*)verts, sizeof(verts));
  update_online_vertices(offline_to_online_quad);
  offline_to_online_quad.ebo = create_ebo(indicies, sizeof(indicies));
  vao_enable_attribute(offline_to_online_quad.vao, offline_to_online_quad.vbo, 0, 2, GL_FLOAT, sizeof(offline_to_online_vertex_t), offsetof(offline_to_online_vertex_t, position));
  vao_enable_attribute(offline_to_online_quad.vao, offline_to_online_quad.vbo, 1, 2, GL_FLOAT, sizeof(offline_to_online_vertex_t), offsetof(offline_to_online_vertex_t, tex)); 
  vao_bind_ebo(offline_to_online_quad.vao, offline_to_online_quad.ebo);
#endif

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
  const char* gltf_file_resources_folder_rel_path = "avacado\\Avocado.gltf";

  std::vector<model_t> models;
  char gltf_full_file_path[256]{};
  sprintf(gltf_full_file_path, "%s\\%s", resources_path, gltf_file_resources_folder_rel_path);
  gltf_load_file(gltf_full_file_path, models);

  while (window.running) {
    poll_events();

    bind_framebuffer(offline_fb);
    glViewport(0, 0, fb_width, fb_height);

    glClearColor(0.f, 0.f, 0.f, 1.f);
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

#if 0
    if (window.resized) {
      update_online_vertices(offline_to_online_quad);
    }
#endif

    unbind_framebuffer();

#if 0
    glViewport(0, 0, window.window_dim.x, window.window_dim.y);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, offline_fb.color_att);
    bind_shader(offline_to_online_shader);
    bind_vao(offline_to_online_quad.vao);
    draw_ebo(offline_to_online_quad.ebo);
    unbind_vao();
    unbind_ebo();
#else
    render_online(offline_fb);
#endif

    swap_buffers();
  }
}
