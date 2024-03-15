#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex0;
layout (location = 2) in vec2 tex1;
layout (location = 3) in vec2 tex2;
layout (location = 4) in vec2 tex3;
layout (location = 5) in vec3 color;
layout (location = 6) in float joints[4];
layout (location = 7) in float weights[4];

uniform int skinned;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float angle;

uniform mat4 joint_model_matricies[20];
uniform mat4 joint_inverse_bind_mats[20];

out vec2 tex_coords[4];
out vec3 vert_color;

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

  mat4 rotate_x = mat4(
    1,0,0,0,
    0,c,-s,0,
    0,s,c,0,
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
  // gl_Position = projection * model * rotate_y * rot_x_90 * vec4(pos, 1.0);
  // gl_Position = projection * model * vec4(pos, 1.0);

  // gl_Position = projection * model * vec4(pos, 1.0);

  if (skinned == 1) {
    mat4 final_model = mat4(0.0);
    for (int i = 0; i < 4; i++) {
      uint ui = uint(joints[i]);
      mat4 joint_model_mat = joint_model_matricies[ui] * joint_inverse_bind_mats[ui];
      mat4 scaled_jmm = joint_model_mat * weights[i];
      final_model += scaled_jmm;
    }
    gl_Position = projection * view * final_model * vec4(pos, 1.0);
  } else {
    gl_Position = projection * view * model * vec4(pos, 1.0);
  }

  tex_coords[0] = tex0;
  tex_coords[1] = tex1;
  tex_coords[2] = tex2;
  tex_coords[3] = tex3;
  vert_color = color;
}

