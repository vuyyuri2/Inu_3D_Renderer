#include "quaternion.h"

#include <math.h>

#include "log.h"
#include "utils/mats.h"

quaternion_t create_quaternion(float x, float y, float z, float w) {
  quaternion_t q;
  q.x = x;
  q.y = y;
  q.z = z;
  q.w = w;
  return q;
}

quaternion_t create_quaternion_w_rot(vec3 axis, float angle) {
  quaternion_t q;
  vec3 norm_axis = norm_vec3(axis); 
  float radians = (angle / 2.f) * 3.1415926535f / 180.f;
  float s = sin(radians);
  float c = cos(radians);
  q.x = norm_axis.x * s;
  q.y = norm_axis.y * s;
  q.z = norm_axis.z * s;
  q.w = c;
  return q;
}

quaternion_t quat_inverse(quaternion_t& q) {
  quaternion_t q_inv;
  q_inv.x = -q.x;
  q_inv.y = -q.y;
  q_inv.z = -q.z;
  q_inv.w = q.w;
  return q_inv;
}

vec3 get_rotated_position(vec3& pos, quaternion_t& q) {
  quaternion_t pos_q; 
  pos_q.x = pos.x;
  pos_q.y = pos.y;
  pos_q.z = pos.z;
  pos_q.w = 0;

  quaternion_t inv = quat_inverse(q);
  quaternion_t m = quat_multiply_quat(q, pos_q); 
  quaternion_t f = quat_multiply_quat(m, inv); 

  inu_assert(f.w <= 001.f);
  
  vec3 rotated_pos;
  rotated_pos.x = f.x;
  rotated_pos.y = f.y;
  rotated_pos.z = f.z;

  return rotated_pos;
}

vec3 get_rotated_position_raw(vec3& pos, quaternion_t& q) {
  float a = q.x;
  float b = q.y;
  float c = q.z;
  float d = q.w; 

  float x = pos.x;
  float y = pos.y;
  float z = pos.z;

  vec3 rotated_pos;
  rotated_pos.x = (x * (a*a - b*b - c*c + d*d)) + (y * (2*a*b - 2*c*d)) + (z * (2*a*c + 2*b*d));
  rotated_pos.y = (x * (2*a*b + 2*c*d)) + (y * (-a*a + b*b - c*c + d*d)) + (z * (-2*a*d + 2*b*c));
  rotated_pos.z = (x * (2*a*c - 2*b*d)) + (y * (2*a*d + 2*b*c)) + (z * (-a*a - b*b + c*c + d*d));

  return rotated_pos;
}

mat4 quat_as_mat4(quaternion_t& q) {
  mat4 m = create_matrix(1.0f);

  float a = q.x;
  float b = q.y;
  float c = q.z;
  float d = q.w;

  m.sep_cols.first_col.x = a*a - b*b - c*c + d*d; 
  m.sep_cols.first_col.y = 2*a*b + 2*c*d; 
  m.sep_cols.first_col.z = 2*a*c - 2*b*d; 

  m.sep_cols.second_col.x = 2*a*b - 2*c*d;
  m.sep_cols.second_col.y = -a*a + b*b - c*c + d*d;
  m.sep_cols.second_col.z = 2*a*d + 2*b*c;

  m.sep_cols.third_col.x = 2*a*c + 2*b*d;
  m.sep_cols.third_col.y = -2*a*b + 2*b*c;
  m.sep_cols.third_col.z = -a*a - b*b + c*c + d*d;

  return m;
}

quaternion_t quat_multiply_quat(quaternion_t& q1, quaternion_t& q2) {
  float a = q1.x;
  float b = q1.y;
  float c = q1.z;
  float d = q1.w;
  float e = q2.x;
  float f = q2.y;
  float g = q2.z;
  float h = q2.w;

  quaternion_t res;
  res.x = (h*a) + (b*g) - (c*f) + (d*e);
  res.y = (-a*g) + (h*b) + (c*e) + (d*f);
  res.z = (a*f) - (b*e) + (c*h) + (d*g);
  res.w = (-a*e) - (b*f) - (c*g) + (d*h);

  return res;
}
