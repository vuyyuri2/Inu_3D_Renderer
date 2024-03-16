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

int skin_t::BONE_MODEL_ID = -1;

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

static std::unordered_set<int> updated_idxs;
void update_obj_model_mats_recursive(int obj_id, mat4& running_model) {
  object_t& obj = objs[obj_id];
  if (obj.is_joint_obj) {
      int a = 5;
  }
  if (isnan(obj.transform.pos.x) || isnan(obj.transform.pos.y) || isnan(obj.transform.pos.z)) {
    inu_assert_msg("obj transform pos is nan");
  }
  mat4 model = get_model_matrix(objs[obj_id].transform);
  for (int i = 0; i < 16; i++) {
    if (isnan(model.vals[i])) {
      inu_assert_msg("obj model matrix from transform is nan");
    }
  }
  objs[obj_id].model_mat = mat_multiply_mat(running_model, model);
  updated_idxs.insert(obj_id);
  for (int child_id : objs[obj_id].child_objects) {
    update_obj_model_mats_recursive(child_id, objs[obj_id].model_mat);
  }
}

object_t* get_obj(int obj_id) {
  return &objs[obj_id];
}

// when doing skinned animation, 
//  first pass is: non-skinned objs need to be updated
//  second pass is : then joints + their children (if not handled in first pass) - (essentially parent bone nodes which have no parents)
void update_obj_model_mats() {
  updated_idxs.clear();
  for (int parent_id : scene.parent_objs) {
    mat4 running_model_mat = create_matrix(1.0f);
    update_obj_model_mats_recursive(parent_id, running_model_mat);
  }

  for (object_t& obj : objs) {
    if (updated_idxs.find(obj.id) == updated_idxs.end() && obj.is_joint_obj && obj.parent_obj == -1) {
      mat4 running_model_mat = create_matrix(1.0f);
      update_obj_model_mats_recursive(obj.id, running_model_mat);
    }
  }

  for (object_t& obj: objs) {
    if (obj.is_joint_obj && updated_idxs.find(obj.id) == updated_idxs.end()) {
      inu_assert_msg("joint was not updated");
    }
  }

}

void attach_anim_chunk_ref_to_obj(int obj_id, animation_chunk_data_ref_t& ref) {
  object_t& obj = objs[obj_id];
  obj.anim_chunk_refs.push_back(ref);

#if 0
  animation_data_chunk_t* data = get_anim_data_chunk(ref.chunk_id);
  vec3* v_anim_data = (vec3*)data->keyframe_data;
  quaternion_t* q_anim_data = (quaternion_t*)data->keyframe_data;
  printf("\n\nobj name: %s\n", obj.name.c_str());
  for (int i = 0; i < data->num_timestamps; i++) {
    printf("timestamp: %f frame: %i ", data->timestamps[i], i+1);
    if (ref.target == ANIM_TARGET_ON_NODE::ROTATION) {
      printf("rot: ");
      print_quat(q_anim_data[i]);
    } else if (ref.target == ANIM_TARGET_ON_NODE::POSITION) {
      printf("pos: ");
      print_vec3(v_anim_data[i]);
    } else if (ref.target == ANIM_TARGET_ON_NODE::SCALE) {
      printf("scale: ");
      print_vec3(v_anim_data[i]);
    }
    printf("\n");
  }
#endif 
}

void attach_name_to_obj(int obj_id, std::string& name) {
  objs[obj_id].name = name;
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

#if 0
  mat4 translate = create_matrix(1.0f);
  if (parent) {
    vec3 t = { 0,0,-0.5f };
    // translate = translate_mat(t);
  }
  // mat4 final_model = mat_multiply_mat(translate, scale);
  mat4 final_model = mat_multiply_mat(translate, obj.model_mat);
#endif
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
      // shader_set_mat4(material_t::associated_shader, "model", final_model);
      shader_set_mat4(material_t::associated_shader, "model", obj.model_mat);

      // transform_t final_transform = get_transform_from_matrix(final_model);
      transform_t final_transform = get_transform_from_matrix(obj.model_mat);
      if (isnan(final_transform.pos.x) || isnan(final_transform.pos.y) || isnan(final_transform.pos.z)) {
        inu_assert_msg("final transform is nan");
      }
    }

    for (mesh_t& mesh : model.meshes) {
      material_t& m = bind_material(mesh.mat_idx);

      if (app_info.render_only_textured && m.base_color_tex.tex_handle == -1) {
        continue;
      }

#if SHOW_BONES
      // only shows bones
      if (obj.is_joint_obj) {
        bind_vao(mesh.vao);
        draw_ebo(mesh.ebo);
        unbind_vao();
        unbind_ebo();
      }
#else
      bind_vao(mesh.vao);
      draw_ebo(mesh.ebo);
      unbind_vao();
      unbind_ebo();
#endif
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
  printf("Skin %s at idx %i has %i bones\n", skin.name.c_str(), skin.id, skin.num_bones);
  for (int i = 0; i < skin.num_bones; i++) {
    int node_idx = skin.joint_obj_ids[i];
    objs[node_idx].is_joint_obj = true;
#if SHOW_BONES
    // objs[node_idx].model_id = skin_t::BONE_MODEL_ID;
    attach_model_to_obj(skin_t::BONE_MODEL_ID);
#endif
  }
  skins.push_back(skin);
  return skin.id;
}

skin_t get_skin(int skin_id) {
  return skins[skin_id];
}

void print_joint_transform_info_helper(int obj_id) {
  object_t& obj = objs[obj_id];
  if (obj.is_joint_obj) {
    printf("------bone name: %s----------\n", obj.name.c_str()) ;
    printf("\n--local transform--\n");
    print_transform(obj.transform);
    printf("\n--global model matrix--\n");
    print_mat4(obj.model_mat);
    transform_t decoded = get_transform_from_matrix(obj.model_mat);
    printf("\n--global transform--\n");
    print_transform(decoded);
    printf("\n");
  }
  for (int c : objs[obj_id].child_objects) {
    print_joint_transform_info_helper(c);
  }
}

void print_joint_transform_info() {
  for (int parent_id : scene.parent_objs) {
    print_joint_transform_info_helper(parent_id);
  }

  for (object_t& obj : objs) {
    if (obj.is_joint_obj && obj.parent_obj == -1) {
      print_joint_transform_info_helper(obj.id);
    }
  }  
}

std::vector<int> get_bone_objs() {
  std::vector<int> bones;
  for (object_t& obj : objs) {
    if (obj.is_joint_obj) {
      bones.push_back(obj.id);
    }
  }
  return bones;
}
