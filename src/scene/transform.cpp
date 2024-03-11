#include "transform.h"

mat4 get_model_matrix(transform_t& t) {
  mat4 scale = scale_mat(t.scale);
  mat4 translate = translate_mat(t.pos);
  mat4 rot = quat_as_mat4(t.rot);
  mat4 inter = mat_multiply_mat(translate, scale);
  mat4 model = mat_multiply_mat(inter, rot);
  return model;
}
