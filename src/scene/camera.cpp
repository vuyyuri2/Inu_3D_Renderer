#include "camera.h"

#include "utils/mats.h"
#include "utils/vectors.h"
#include "windowing/window.h"
#include "utils/general.h"
#include "utils/log.h"

static camera_t cam;

extern window_t window;

void update_cam() {
  if (window.input.scroll_wheel_delta != 0) {
    vec3 diff;
    diff.x = cam.transform.pos.x - cam.focal_pt.x;
    diff.y = cam.transform.pos.y - cam.focal_pt.y;
    diff.z = cam.transform.pos.z - cam.focal_pt.z;
    float l = length(diff);
    cam_move_forward(window.input.scroll_wheel_delta * l / 20.f);
  }

  if (length(window.input.mouse_pos_diff) != 0 && window.input.middle_mouse_down) {
    float sensitivity = 0.3f;
    float lat = window.input.mouse_pos_diff.x * -2.f * sensitivity;
    float vert = window.input.mouse_pos_diff.y * 1.f * sensitivity;
    cam_move_rotate(lat, vert);
  }

  // cam.view = get_cam_view_mat();
  cam.view = get_view_mat(cam.transform.pos, cam.focal_pt);
  cam.proj = proj_mat(60.f, cam.near_plane, cam.far_plane, static_cast<float>(window.window_dim.x) / window.window_dim.y);
}

void create_camera(transform_t& t) {
  cam.near_plane = 0.01f;
  cam.far_plane = 30.f;

  cam.transform.pos.x = t.pos.x;
  cam.transform.pos.y = t.pos.y;
  cam.transform.pos.z = t.pos.z;

  // scale doesn't matter
  cam.transform.scale.x = 1.f;
  cam.transform.scale.y = 1.f;
  cam.transform.scale.z = 1.f;

  // quat rot doesn't matter either b/c using on focal pt based rot logic
  cam.transform.rot.x = 0;
  cam.transform.rot.y = 0;
  cam.transform.rot.z = 0;
  cam.transform.rot.w = 1;
}

mat4 get_view_mat(vec3 pos, vec3 focal_pt) {
  vec3 inv_t = {-pos.x, -pos.y, -pos.z};
  mat4 inv_translate = translate_mat(inv_t);

  vec3 to_fp;
  to_fp.x = focal_pt.x - pos.x;
  to_fp.y = focal_pt.y - pos.y;
  to_fp.z = focal_pt.z - pos.z;
  to_fp = norm_vec3(to_fp);

  vec3 world_up = {0,1,0};
  vec3 right;
  if (to_fp.x == 0 && to_fp.y == -1.f && to_fp.z == 0) {
    right = {-1,0,0};
  } else if (to_fp.x == 0 && to_fp.y == 1.f && to_fp.z == 0) {
    right = {1,0,0};
  } else {
    right = cross_product(to_fp, world_up);
  }

  vec3 up = cross_product(right, to_fp);

  mat4 rot_mat = create_matrix(1.0f);

  rot_mat.first_col.x = right.x;
  rot_mat.first_col.y = right.y;
  rot_mat.first_col.z = right.z;

  rot_mat.second_col.x = up.x;
  rot_mat.second_col.y = up.y;
  rot_mat.second_col.z = up.z;

  vec3 neg_to_fp = {-to_fp.x, -to_fp.y, -to_fp.z};
  rot_mat.third_col.x = neg_to_fp.x;
  rot_mat.third_col.y = neg_to_fp.y;
  rot_mat.third_col.z = neg_to_fp.z;

  mat4 inv_rot = transpose(rot_mat);
  return mat_multiply_mat(inv_rot, inv_translate);
}

mat4 get_cam_proj_mat() {
  return cam.proj;
}

mat4 get_cam_view_mat() {
#if 0
  vec3 inv_t = {-cam.transform.pos.x, -cam.transform.pos.y, -cam.transform.pos.z};
  mat4 inv_translate = translate_mat(inv_t);

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

  rot_mat.first_col.x = right.x;
  rot_mat.first_col.y = right.y;
  rot_mat.first_col.z = right.z;

  rot_mat.second_col.x = up.x;
  rot_mat.second_col.y = up.y;
  rot_mat.second_col.z = up.z;

  rot_mat.third_col.x = neg_to_fp.x;
  rot_mat.third_col.y = neg_to_fp.y;
  rot_mat.third_col.z = neg_to_fp.z;

  mat4 inv_rot = transpose(rot_mat);
  return mat_multiply_mat(inv_rot, inv_translate);
#elif 0
  return get_view_mat(cam.transform.pos, cam.focal_pt);
#else
  return cam.view;
#endif
}

#if 0
mat4 get_cam_view_mat(vec3& diff) {
  cam.transform.pos.x += diff.x;
  cam.transform.pos.y += diff.y;
  cam.transform.pos.z += diff.z;
  return get_cam_view_mat();
}
#endif

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

  vec3 neg_to_fp = {-to_fp.x, -to_fp.y, -to_fp.z};
  float s = dot(cam.transform.pos, neg_to_fp);
  int side = sgn(s);

  inu_assert(side == 1);

  cam.transform.pos.x += diff.x;
  cam.transform.pos.y += diff.y;
  cam.transform.pos.z += diff.z;

  float new_s = dot(cam.transform.pos, neg_to_fp);
  float new_side = sgn(new_s);

  if (new_side != side) {
    cam.transform.pos.x -= diff.x;
    cam.transform.pos.y -= diff.y;
    cam.transform.pos.z -= diff.z;
  }
}

void cam_move_rotate(float lat_amount, float vert_amount) {
  vec3 to_fp;
  to_fp.x = cam.focal_pt.x - cam.transform.pos.x;
  to_fp.y = cam.focal_pt.y - cam.transform.pos.y;
  to_fp.z = cam.focal_pt.z - cam.transform.pos.z;
  to_fp = norm_vec3(to_fp);

  vec3 focal_pt_up = {0,1,0};
  vec3 right = cross_product(to_fp, focal_pt_up);
  quaternion_t q_right = create_quaternion_w_rot(right, vert_amount);
  quaternion_t q_up = create_quaternion_w_rot(focal_pt_up, lat_amount);
  quaternion_t q = quat_multiply_quat(q_right, q_up);

  cam.transform.pos = get_rotated_position(cam.transform.pos, q);
}

camera_t* get_cam() {
  return &cam; 
}

