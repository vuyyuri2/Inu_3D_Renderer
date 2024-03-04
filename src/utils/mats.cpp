#include "mats.h"

#include <memory>
#include <cmath>

mat4::mat4() {
  memset(cols, 0, sizeof(cols));
}

mat4 create_matrix(float diag_val) {
  mat4 m;
  m.sep_cols.first_col.x = diag_val;
  m.sep_cols.second_col.y = diag_val;
  m.sep_cols.third_col.z = diag_val;
  m.sep_cols.fourth_col.w = diag_val;
  return m;
}

mat4 mat_multiply_mat(mat4& m1, mat4& m2) {
  mat4 res;
  res.sep_cols.first_col = mat_multiply_vec(m1, m2.sep_cols.first_col);
  res.sep_cols.second_col = mat_multiply_vec(m1, m2.sep_cols.second_col);
  res.sep_cols.third_col = mat_multiply_vec(m1, m2.sep_cols.third_col);
  res.sep_cols.fourth_col = mat_multiply_vec(m1, m2.sep_cols.fourth_col);
  return res;
}

vec4 vec_multiply_float(vec4& v, float f) {
  vec4 res;
  res.x = v.x * f;
  res.y = v.y * f;
  res.z = v.z * f;
  res.w = v.w * f;
  return res;
}

vec4 mat_multiply_vec(mat4& m, vec4& v) {

  vec4 c0 = vec_multiply_float(m.cols[0], v.x);
  vec4 c1 = vec_multiply_float(m.cols[1], v.y);
  vec4 c2 = vec_multiply_float(m.cols[2], v.z);
  vec4 c3 = vec_multiply_float(m.cols[3], v.w);

  vec4 res;
  res.x = c0.x + c1.x + c2.x + c3.x;
  res.y = c0.y + c1.y + c2.y + c3.y;
  res.z = c0.z + c1.z + c2.z + c3.z;
  res.w = c0.w + c1.w + c2.w + c3.w;

  return res;
}

proj_mats_t proj_mat(float fov, float near, float far, float aspect_ratio) {
  float multipler = 3.141526f / 180.f;
  float top = near * tan(fov * multipler / 2.f);
  float right = top * aspect_ratio;

  proj_mats_t proj_mat;

  // brings to origin
  mat4 translate = create_matrix(1.0f);
  translate.sep_cols.fourth_col.z = (far+near) / 2.f;

  mat4 scale = create_matrix(1.0f);
  scale.cols[0].x = 1/right;
  scale.cols[1].y = 1/top;
  scale.cols[2].z = -2.f/(far-near);
  
  proj_mat.ortho = mat_multiply_mat(scale, translate);

  mat4& pers = proj_mat.persp;
  pers = create_matrix(1.0f);
  pers.cols[0].x = near;

  pers.cols[1].y = near;

  pers.cols[2].z = near + far;
  pers.cols[2].w = -1.f;

  pers.cols[3].z = near * far;
  pers.cols[3].w = 0.f;

  // mat4 proj = mat_multiply_mat(ortho, pers);

  return proj_mat;
}

mat4 scale_mat(float s) {
  mat4 scale = create_matrix(1.0f);
  scale.cols[0].x = s;
  scale.cols[1].y = s;
  scale.cols[2].z = s;
  return scale;
}

mat4 translate_mat(vec3& p) {
  mat4 translate = create_matrix(1.0f);
  translate.cols[3].x = p.x;
  translate.cols[3].y = p.y;
  translate.cols[3].z = p.z;
  return translate;
}
