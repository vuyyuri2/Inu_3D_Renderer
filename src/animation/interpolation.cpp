#include "interpolation.h"

#include "math.h"

#include "utils/general.h"

quaternion_t spherical_linear(quaternion_t& qa, quaternion_t& qb, float t) {
  float d = quat_dot(qa, qb);
  float d_abs = fabs(d);
  float a = acos(d_abs);
  float s = sgn(a);

  float scale_of_qa = sin(a * (1-t)) / sin(a);
  quaternion_t qa_scaled = quat_multiply_float(qa, scale_of_qa);

  float scale_of_qb = s * sin(a*t) / sin(a);
  quaternion_t qb_scaled = quat_multiply_float(qb, scale_of_qb);

  quaternion_t inter = quat_add_quat(qa_scaled, qb_scaled);
  return inter;
}

float linear(float a, float b, float t) {
  return (a*(1-t)) + (t*b);
}

vec3 vec3_linear(vec3 a, vec3 b, float t) {
  vec3 i;
  i.x = linear(a.x, b.x, t);
  i.y = linear(a.y, b.y, t);
  i.z = linear(a.z, b.z, t);
  return i;
}
