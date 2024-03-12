#include "camera.h"

#include "utils/mats.h"
#include "utils/vectors.h"

static camera_t cam;

void create_camera(transform_t& t) {
  cam.transform.pos.x = t.pos.x;
  cam.transform.pos.y = t.pos.y;
  cam.transform.pos.z = t.pos.z;

  // scale doesn't matter
  cam.transform.scale.x = 1.f;
  cam.transform.scale.y = 1.f;
  cam.transform.scale.z = 1.f;

  cam.transform.rot.x = t.rot.x;
  cam.transform.rot.y = t.rot.y;
  cam.transform.rot.z = t.rot.z;
  cam.transform.rot.w = t.rot.w;

  cam.forward = {0,0,-1};
  cam.up = {0,1,0};
  cam.right = {1,0,0};

  cam.focal_pt_up = {0,1,0};
}

mat4 get_view_mat() {
  vec3 inv_t = {-cam.transform.pos.x, -cam.transform.pos.y, -cam.transform.pos.z};
  mat4 inv_translate = translate_mat(inv_t);
#if 0
  quaternion_t inv_rot = quat_inverse(cam.transform.rot);
  mat4 inv_rot_mat = quat_as_mat4(inv_rot);
  // mat4 view = mat_multiply_mat(inv_translate, inv_rot_mat);
  mat4 view = mat_multiply_mat(inv_rot_mat, inv_translate);
  // return view;
  return inv_translate;
#else

  vec3 to_fp;
  to_fp.x = cam.focal_pt.x - cam.transform.pos.x;
  to_fp.y = cam.focal_pt.y - cam.transform.pos.y;
  to_fp.z = cam.focal_pt.z - cam.transform.pos.z;
  to_fp = norm_vec3(to_fp);

  vec3 world_up = {0,1,0};
  vec3 right = cross_product(to_fp, world_up);

  vec3 neg_to_fp = {-to_fp.x, -to_fp.y, -to_fp.z};
  vec3 up = cross_product(right, to_fp);

  mat4 rot_mat = create_matrix(1.0f);

  rot_mat.sep_cols.first_col.x = right.x;
  rot_mat.sep_cols.first_col.y = right.y;
  rot_mat.sep_cols.first_col.z = right.z;

  rot_mat.sep_cols.second_col.x = up.x;
  rot_mat.sep_cols.second_col.y = up.y;
  rot_mat.sep_cols.second_col.z = up.z;

  rot_mat.sep_cols.third_col.x = neg_to_fp.x;
  rot_mat.sep_cols.third_col.y = neg_to_fp.y;
  rot_mat.sep_cols.third_col.z = neg_to_fp.z;

  mat4 inv_rot = transpose(rot_mat);
  return mat_multiply_mat(inv_rot, inv_translate);

#endif
}

mat4 get_view_mat(quaternion_t& new_q) {
  cam.transform.rot.x = new_q.x;
  cam.transform.rot.y = new_q.y;
  cam.transform.rot.z = new_q.z;
  cam.transform.rot.w = new_q.w;
  return get_view_mat();
}

mat4 get_view_mat(vec3& diff) {
  cam.transform.pos.x += diff.x;
  cam.transform.pos.y += diff.y;
  cam.transform.pos.z += diff.z;
  return get_view_mat();
}

void cam_move_forward(float amount) {
  vec3 diff;

  vec3 to_fp;
  to_fp.x = cam.focal_pt.x - cam.transform.pos.x;
  to_fp.y = cam.focal_pt.y - cam.transform.pos.y;
  to_fp.z = cam.focal_pt.z - cam.transform.pos.z;
  to_fp = norm_vec3(to_fp);

  diff.x = to_fp.x * amount;
  diff.y = to_fp.y * amount;
  diff.z = to_fp.z * amount;

  cam.transform.pos.x += diff.x;
  cam.transform.pos.y += diff.y;
  cam.transform.pos.z += diff.z;
}

void cam_move_rotate(float lat_amount, float vert_amount) {
#if 0
  quaternion_t& cur = cam.transform.rot; 
  quaternion_t rot = create_quaternion_w_rot(cam.up, amount);
  quaternion_t new_q = quat_multiply_quat(rot, cur);

  cam.transform.rot.x = new_q.x;
  cam.transform.rot.y = new_q.y;
  cam.transform.rot.z = new_q.z;
  cam.transform.rot.w = new_q.w; 
#else
  // vec3 cam_forward = {};
  // quaternion_t& cur = cam.transform.rot; 

  vec3 to_fp;
  to_fp.x = cam.focal_pt.x - cam.transform.pos.x;
  to_fp.y = cam.focal_pt.y - cam.transform.pos.y;
  to_fp.z = cam.focal_pt.z - cam.transform.pos.z;
  to_fp = norm_vec3(to_fp);

  vec3 right = cross_product(to_fp, cam.focal_pt_up);
  quaternion_t q_right = create_quaternion_w_rot(right, vert_amount);
  quaternion_t q_up = create_quaternion_w_rot(cam.focal_pt_up, lat_amount);
  quaternion_t q = quat_multiply_quat(q_right, q_up);

  cam.transform.pos = get_rotated_position(cam.transform.pos, q);

#if 0
  quaternion_t new_q = quat_multiply_quat(q, cur);
  cam.transform.rot.x = new_q.x;
  cam.transform.rot.y = new_q.y;
  cam.transform.rot.z = new_q.z;
  cam.transform.rot.w = new_q.w;
#endif
#endif

}
