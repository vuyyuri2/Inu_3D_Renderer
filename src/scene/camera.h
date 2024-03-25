
#include "utils/transform.h"
#include "utils/mats.h"

struct camera_t {
  // scale doesn't matter
  transform_t transform;
  vec3 focal_pt;
  mat4 view;
  mat4 proj;
  float near_plane;
  float far_plane;
};

void create_camera(transform_t& t);
mat4 get_cam_view_mat();
mat4 get_view_mat(vec3 pos, vec3 focal_pt);
void cam_move_forward(float amount);
void cam_move_rotate(float lat_amount, float vert_amount);
mat4 get_cam_view_mat(vec3& diff);
void update_cam();


