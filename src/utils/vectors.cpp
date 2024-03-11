#include "vectors.h"

#include <math.h>

vec3 norm_vec3(vec3& v) {
  float length = sqrt(v.x*v.x + v.y*v.y + v.z*v.z); 
  vec3 res;
  res.x = v.x / length;
  res.y = v.y / length;
  res.z = v.z / length;
  return res;
}
