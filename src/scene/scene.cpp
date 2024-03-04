#include "scene.h"

static std::vector<object_t> objs;

int create_object(transform_t& transform) {
  object_t obj;
  static int i = 0;
  obj.id = i++;
  memcpy(&obj.transform.pos, &transform, sizeof(transform_t));
  objs.push_back(obj);
  return obj.id;
}

void attach_mesh_to_obj(int obj_id, int mesh_idx) {
  objs[obj_id].mesh_idx = mesh_idx;
}

void attach_child_obj_to_obj(int obj_id, int child_obj_id) {
  objs[obj_id].child_objects.push_back(child_obj_id);
}
