#include "scene.h"

#include <algorithm>

#include "utils/mats.h"
#include "gfx/gfx.h"
#include "model_loading/model_internal.h"
#include "utils/app_info.h"

std::vector<object_t> objs;
std::vector<skin_t> skins;
static scene_t scene;

extern std::vector<model_t> models;
extern app_info_t app_info;

int create_object(transform_t& transform) {
  object_t obj;
  static int i = 0;
  obj.id = i++;
  memcpy(&obj.transform, &transform, sizeof(transform_t));
  objs.push_back(obj);
  return obj.id;
}

void attach_model_to_obj(int obj_id, int model_id) {
  printf("attached model id %i to obj: %i\n", model_id, obj_id);
  objs[obj_id].model_id = model_id;
}

void attach_child_obj_to_obj(int obj_id, int child_obj_id) {
  if (child_obj_id < objs.size() && objs[child_obj_id].is_skinned) {
    return;
  }
  objs[obj_id].child_objects.push_back(child_obj_id);
}

void populate_parent_field_of_nodes_helper(int parent, int child) {
  objs[child].parent_obj = parent;
  for (int gc : objs[child].child_objects) {
    populate_parent_field_of_nodes_helper(child, gc);
  }
}

void populate_parent_field_of_nodes() {
  for (int parent_id : scene.parent_objs) {
    objs[parent_id].parent_obj = -1;
    for (int child : objs[parent_id].child_objects) {
      populate_parent_field_of_nodes_helper(parent_id, child);
    }
  }
}

void set_obj_as_parent(int obj_id) {
  scene.parent_objs.insert(obj_id);
}

mat4 get_obj_model_mat(int obj_id) {
  return objs[obj_id].model_mat;
}

void update_obj_model_mats_recursive(int obj_id, mat4& running_model) {
  object_t& obj = objs[obj_id];
  if (isnan(obj.transform.pos.x) || isnan(obj.transform.pos.y) || isnan(obj.transform.pos.z)) {
    int a = 5;
  }
  mat4 model = get_model_matrix(objs[obj_id].transform);
  for (int i = 0; i < 16; i++) {
    if (isnan(model.vals[i])) {
      int a = 5;
    }
  }
  objs[obj_id].model_mat = mat_multiply_mat(running_model, model);
  for (int child_id : objs[obj_id].child_objects) {
    update_obj_model_mats_recursive(child_id, objs[obj_id].model_mat);
  }
}

// when doing skinned animation, 
//  first pass is: non-skin objs need to be updated
//  second pass is : then joints + their children (if not handled in first pass)
//  third pass is: skinned objs that rely on the joints (for now this will be done entirely in the shader)
void update_obj_model_mats() {
  for (int parent_id : scene.parent_objs) {
    mat4 running_model_mat = create_matrix(1.0f);
    update_obj_model_mats_recursive(parent_id, running_model_mat);
  }

  for (object_t& obj : objs) {
    if (obj.is_joint_obj && obj.parent_obj == -1) {
      mat4 running_model_mat = create_matrix(1.0f);
      update_obj_model_mats_recursive(obj.id, running_model_mat);
    }
  }

}

void attach_anim_chunk_ref_to_obj(int obj_id, animation_chunk_data_ref_t& ref) {
  object_t& obj = objs[obj_id];
  obj.anim_chunk_refs.push_back(ref);
}

void attach_skin_to_obj(int obj_id, int skin_id) {
  object_t& obj = objs[obj_id];
  obj.skin_id = skin_id;
  objs[obj_id].is_skinned = true;
  for (int i = 0; i < objs.size(); i++) {
    auto child_it = std::find(objs[i].child_objects.begin(), objs[i].child_objects.end(), obj_id);
    if (child_it != objs[i].child_objects.end()) {
      // int idx = child_it - objs[i].child_objects.begin();
      objs[i].child_objects.erase(child_it);
    }
  }
}

void render_scene_obj(int obj_id, bool parent) {
  object_t& obj = objs[obj_id];
  mat4 translate = create_matrix(1.0f);
  if (parent) {
    vec3 t = { 0,0,-0.5f };
    // translate = translate_mat(t);
  }
  // mat4 final_model = mat_multiply_mat(translate, scale);
  mat4 final_model = mat_multiply_mat(translate, obj.model_mat);
  if (obj.model_id != -1) {
    model_t& model = models[obj.model_id];
    if (obj.is_skinned) {
      skin_t skin = get_skin(obj.skin_id);
      shader_set_int(material_t::associated_shader, "skinned", 1);
      for (int i = 0; i < skin.num_bones; i++) {
        char mat4_name[64]{};
        sprintf(mat4_name, "joint_inverse_bind_mats[%i]", i);
        shader_set_mat4(material_t::associated_shader, mat4_name, skin.inverse_bind_matricies[i]);

        memset(mat4_name, 0, sizeof(mat4_name));
        sprintf(mat4_name, "joint_model_matricies[%i]", i);
        mat4 joint_model_matrix = get_obj_model_mat(skin.joint_obj_ids[i]);
        shader_set_mat4(material_t::associated_shader, mat4_name, joint_model_matrix);
      }

      // setting to defaults
      for (int i = skin.num_bones; i < BONES_PER_SKIN_LIMIT; i++) {
        char mat4_name[64]{};
        sprintf(mat4_name, "joint_inverse_bind_mats[%i]", i);
        shader_set_mat4(material_t::associated_shader, mat4_name, create_matrix(1.0f));

        memset(mat4_name, 0, sizeof(mat4_name));
        sprintf(mat4_name, "joint_model_matricies[%i]", i);
        shader_set_mat4(material_t::associated_shader, mat4_name, create_matrix(1.0f));
      }
    } else {
      shader_set_int(material_t::associated_shader, "skinned", 0);
      shader_set_mat4(material_t::associated_shader, "model", final_model);

      transform_t final_transform = get_transform_from_matrix(final_model);
      if (isnan(final_transform.pos.x) || isnan(final_transform.pos.y) || isnan(final_transform.pos.z)) {
        int a = 5;
      }
    }

    for (mesh_t& mesh : model.meshes) {
      material_t m = bind_material(mesh.mat_idx);

      if (app_info.render_only_textured && m.base_color_tex.tex_handle == -1) {
        continue;
      }

      bind_vao(mesh.vao);
      draw_ebo(mesh.ebo);
      unbind_vao();
      unbind_ebo();
    }
  }

  for (int child : obj.child_objects) {
    render_scene_obj(child, false);
  }
}

void render_scene() {
  for (int parent_id : scene.parent_objs) {
    render_scene_obj(parent_id, true);
  }
  for (object_t& obj : objs) {
    if (obj.is_skinned) {
      render_scene_obj(obj.id, false);
    }
  }
  unbind_shader();
}

skin_t::skin_t() {
  id = -1;
  upper_most_joint_node_idx = -1;
  memset(joint_obj_ids, 0, sizeof(joint_obj_ids));
  for (int i = 0; i < BONES_PER_SKIN_LIMIT; i++) {
    inverse_bind_matricies[i] = create_matrix(1.0f);
  }
}

int register_skin(skin_t& skin) {
  skin.id = skins.size();
  for (int node_idx : skin.joint_obj_ids) {
    objs[node_idx].is_joint_obj = true;
  }
  skins.push_back(skin);
  return skin.id;
}

skin_t get_skin(int skin_id) {
  return skins[skin_id];
}
