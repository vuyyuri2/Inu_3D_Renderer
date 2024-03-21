#version 410 core

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec2 tex0;
layout (location = 2) in vec2 tex1;
layout (location = 3) in vec3 vert_color;
layout (location = 4) in vec4 joints;
layout (location = 5) in vec4 weights;
layout (location = 6) in vec3 vert_normal;

// layout (location = 3) in vec2 tex2;
// layout (location = 4) in vec2 tex3;
// layout (location = 5) in vec3 color;
// layout (location = 6) in vec4 joints;
// layout (location = 7) in vec4 weights;

uniform int skinned;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float angle;

uniform mat4 light_view;
uniform mat4 light_projection;

uniform mat4 joint_model_matricies[80];
uniform mat4 joint_inverse_bind_mats[80];

out vec2 tex_coords[2];
out vec3 color;
out vec3 normal;
out vec4 light_rel_screen_pos;
out vec2 light_pass_depth_tex_coord;

void main() {

  float c = cos(radians(angle));
  float s = sin(radians(angle));

  float c90 = cos(radians(90));
  float s90 = sin(radians(90));
  mat4 rotate_z = mat4(
    c90,s90,0,0,
    -s90,c90,0,0,
    0,0,1,0,
    0,0,0,1
  );


  mat4 rot_x_90 = mat4(
    1,0,0,0,
    0,c90,-s90,0,
    0,s90,c90,0,
    0,0,0,1
  );

  /*
  mat4 rotate_y = mat4(
    c,0,-s,0,
    0,1,0,0,
    s,0,c,0,
    0,0,0,1
  );
  */
  mat4 rotate_y = mat4(
    c,0,s,0,
    0,1,0,0,
    -s,0,c,0,
    0,0,0,1
  );
  // gl_Position = projection * model * rotate_y * rot_x_90 * vec4(vert_pos, 1.0);
  // gl_Position = projection * model * vec4(vert_pos, 1.0);

  // gl_Position = projection * model * vec4(vert_pos, 1.0);

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
  light_rel_screen_pos = light_projection * light_view * final_model * vec4(vert_pos, 1.0);
  light_rel_screen_pos = light_rel_screen_pos / light_rel_screen_pos.w;
  // z_pos = light_rel_screen_pos.z;

  light_pass_depth_tex_coord = (light_rel_screen_pos.xy + vec2(1,1)) / 2;

  tex_coords[0] = tex0;
  tex_coords[1] = tex1;
  // tex_coords[2] = tex2;
  // tex_coords[3] = tex3;
  color = vert_color;
  normal = vert_normal;
}

