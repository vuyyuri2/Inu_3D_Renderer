#include "vectors.h"

#include <math.h>
#include <stdio.h>

float length(ivec2& v) {
  return sqrt(v.x*v.x + v.y*v.y);
}

vec3 norm_vec3(vec3& v) {
  float length = sqrt(v.x*v.x + v.y*v.y + v.z*v.z); 
  vec3 res;
  res.x = v.x / length;
  res.y = v.y / length;
  res.z = v.z / length;
  return res;
}

void print_vec3(vec3& v) {
  printf("%f, %f, %f", v.x, v.y, v.z);
}

vec3 cross_product(vec3& v1, vec3& v2) {
  vec3 c;
  float a1 = v1.x;
  float a2 = v1.y;
  float a3 = v1.z;
  float b1 = v2.x;
  float b2 = v2.y;
  float b3 = v2.z;
  c.x = (a2 * b3) - (a3 * b2);
  c.y = -( (a1*b3) - (a3*b1) );
  c.z = (a1*b2 - a2*b1);
  return c;
}

float dot(vec3& v1, vec3& v2) {
  return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

float length(vec3& v) {
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

bool operator==(const vec3& v1, const vec3& v2) {
  return (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z);
}

float vec4_length(vec4& v) {
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

vec4 vec4::operator/(float divider) {
  vec4 r;
  r.x = this->x / divider;
  r.y = this->y / divider;
  r.z = this->z / divider;
  r.w = this->w / divider;
  return r;
}
