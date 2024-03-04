#include "scene.h"

#include "utils/mats.h"
#include "gfx/gfx.h"
#include "model_loading/model_internal.h"

static std::vector<object_t> objs;
static scene_t scene;

extern std::vector<model_t> models;

int create_object(transform_t& transform) {
  object_t obj;
  static int i = 0;
  obj.id = i++;
  memcpy(&obj.transform, &transform, sizeof(transform_t));
  objs.push_back(obj);
  return obj.id;
}

void attach_model_to_obj(int obj_id, int model_id) {
  objs[obj_id].model_id = model_id;
}

void attach_child_obj_to_obj(int obj_id, int child_obj_id) {
  objs[obj_id].child_objects.push_back(child_obj_id);
}

void set_obj_as_parent(int obj_id) {
  scene.parent_objs.insert(obj_id);
}

void update_obj_model_mats_recursive(int obj_id, mat4& running_model) {
  mat4 model = get_model_matrix(objs[obj_id].transform);
  objs[obj_id].model_mat = mat_multiply_mat(running_model, model);
  for (int child_id : objs[obj_id].child_objects) {
    update_obj_model_mats_recursive(child_id, objs[obj_id].model_mat);
  }
}

void update_obj_model_mats() {
  for (int parent_id : scene.parent_objs) {
    mat4 running_model_mat = create_matrix(1.0f);
    update_obj_model_mats_recursive(parent_id, running_model_mat);
  }
}

void render_scene() {
  vec3 t = { 0,0,-10 };
  mat4 translate = translate_mat(t);

  // will need to change this to iterate over nodes rather than models 
    for (model_t& model : models) {
      // mat4 final_model = mat_multiply_mat(translate, scale);
      shader_set_mat4(material_t::associated_shader, "model", translate);
      for (mesh_t& mesh : model.meshes) {
        bind_material(mesh.mat_idx);

        bind_vao(mesh.vao);
        draw_ebo(mesh.ebo);
        unbind_vao();
        unbind_ebo();
      }
    }

    unbind_shader();
}
