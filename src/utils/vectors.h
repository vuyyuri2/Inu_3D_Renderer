#pragma once

struct vec2 {
  float x = 0;
  float y = 0;
};

struct ivec2 {
  int x = 0;
  int y = 0;
};
float length(ivec2& v);

struct vec3 {
  float x = 0;
  float y = 0;
  float z = 0;
};
vec3 norm_vec3(vec3& v);
vec3 cross_product(vec3& v1, vec3& v2);

struct vec4 {
  float x = 0;
  float y = 0;
  float z = 0;
  float w = 0;
};
