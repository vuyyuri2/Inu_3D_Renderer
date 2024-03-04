#pragma once

#include "vectors.h"
#include "scene/transform.h"

struct mat4 {
  union {
    vec4 cols[4];
    struct {
      vec4 first_col;
      vec4 second_col;
      vec4 third_col;
      vec4 fourth_col;
    } sep_cols;
  };
  mat4();
};

mat4 create_matrix(float diag_val);
mat4 mat_multiply_mat(mat4& m1, mat4& m2);
vec4 vec_multiply_float(vec4& v, float f);
vec4 mat_multiply_vec(mat4& m, vec4& v);

mat4 proj_mat(float fov, float near, float far, float aspect_ratio);
mat4 scale_mat(float s);
mat4 scale_mat(vec3& scale);
mat4 translate_mat(vec3& p);
mat4 get_model_matrix(transform_t& t);
