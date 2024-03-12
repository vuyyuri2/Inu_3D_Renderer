
#include "transform.h"

struct camera_t {
  // scale doesn't matter
  transform_t transform;
  vec3 up;
  vec3 forward;
  vec3 right;

  vec3 focal_pt;
  vec3 focal_pt_up;
};

void create_camera(transform_t& t);
mat4 get_view_mat();
mat4 get_view_mat(quaternion_t& new_q);
void cam_move_forward(float amount);
void cam_move_rotate(float lat_amount, float vert_amount);
mat4 get_view_mat(vec3& diff);
void update_cam();


