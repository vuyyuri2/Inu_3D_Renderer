#version 410 core

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec2 tex0;
layout (location = 2) in vec2 tex1;
layout (location = 3) in vec3 vert_color;
layout (location = 4) in vec4 joints;
layout (location = 5) in vec4 weights;
layout (location = 6) in vec3 vert_normal;

uniform int skinned;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 joint_model_matricies[80];
uniform mat4 joint_inverse_bind_mats[80];

// light 4x4 matrix info
struct light_mat_data_t {
  mat4 light_view;
  mat4 light_projection;
};
uniform light_mat_data_t lights_mat_data[3];

out vec2 tex_coords[2];
out vec3 color;
out vec3 normal;

out vec4 light_rel_screen_pos0;
// out vec2 light_pass_depth_tex_coord0;

out vec4 light_rel_screen_pos1;
// out vec2 light_pass_depth_tex_coord1;

out vec4 light_rel_screen_pos2;
// out vec2 light_pass_depth_tex_coord2;

struct light_rel_data_t {
  vec4 screen_rel_pos;
  // vec2 depth_tex_coord;
};

light_rel_data_t calc_light_rel_data(mat4 light_projection, mat4 light_view, mat4 model) {
  light_rel_data_t data;
  data.screen_rel_pos = light_projection * light_view * model * vec4(vert_pos, 1.0);
  // data.screen_rel_pos = data.screen_rel_pos / data.screen_rel_pos.w;
  // data.depth_tex_coord = (data.screen_rel_pos.xy + vec2(1,1)) / 2;
  return data;
}

void main() {

  mat4 final_model = mat4(0.0);
  if (skinned == 1) {
    uint ui = 0;
    mat4 joint_model_mat = mat4(0.0);
    mat4 scaled_jmm = mat4(0.0);

    // 1st joint
    ui = uint(joints.x);
    joint_model_mat = joint_model_matricies[ui] * joint_inverse_bind_mats[ui];
    scaled_jmm = joint_model_mat * weights.x;
    final_model += scaled_jmm;

    // 2nd joint
    ui = uint(joints.y);
    joint_model_mat = joint_model_matricies[ui] * joint_inverse_bind_mats[ui];
    scaled_jmm = joint_model_mat * weights.y;
    final_model += scaled_jmm;

    // 3rd joint
    ui = uint(joints.z);
    joint_model_mat = joint_model_matricies[ui] * joint_inverse_bind_mats[ui];
    scaled_jmm = joint_model_mat * weights.z;
    final_model += scaled_jmm;

    // 4th joint
    ui = uint(joints.w);
    joint_model_mat = joint_model_matricies[ui] * joint_inverse_bind_mats[ui];
    scaled_jmm = joint_model_mat * weights.w;
    final_model += scaled_jmm; 
  } else {
    final_model = model;
  }

  gl_Position = projection * view * final_model * vec4(vert_pos, 1.0);

  tex_coords[0] = tex0;
  tex_coords[1] = tex1;
  color = vert_color;
  normal = vert_normal;

  // light_rel_screen_pos0 = light_projection * light_view * final_model * vec4(vert_pos, 1.0);
  // light_rel_screen_pos0 = light_rel_screen_pos / light_rel_screen_pos.w;
  // light_pass_depth_tex_coord = (light_rel_screen_pos0.xy + vec2(1,1)) / 2;

  light_rel_data_t light_rel_data0 = calc_light_rel_data(lights_mat_data[0].light_projection, lights_mat_data[0].light_view, final_model);
  light_rel_screen_pos0 = light_rel_data0.screen_rel_pos;
  // light_pass_depth_tex_coord0 = light_rel_data0.depth_tex_coord;

  light_rel_data_t light_rel_data1 = calc_light_rel_data(lights_mat_data[1].light_projection, lights_mat_data[1].light_view, final_model);
  light_rel_screen_pos1 = light_rel_data1.screen_rel_pos;
  // light_pass_depth_tex_coord1 = light_rel_data1.depth_tex_coord;

  light_rel_data_t light_rel_data2 = calc_light_rel_data(lights_mat_data[2].light_projection, lights_mat_data[2].light_view, final_model);
  light_rel_screen_pos2 = light_rel_data2.screen_rel_pos;
  // light_pass_depth_tex_coord2 = light_rel_data2.depth_tex_coord;
}

