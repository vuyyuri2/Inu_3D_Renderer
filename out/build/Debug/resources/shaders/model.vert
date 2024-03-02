#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex0;
layout (location = 2) in vec2 tex1;
layout (location = 3) in vec2 tex2;
layout (location = 4) in vec2 tex3;
layout (location = 5) in vec3 color;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;
uniform float angle;

out vec2 tex_coords[4];
out vec3 vert_color;

void main() {
  // gl_Position = projection * view * model * vec4(pos, 1.0);
  // float angle = 45;
  float c = cos(radians(angle));
  float s = sin(radians(angle));
  /*
  mat4 model = mat4(
    c, 0.0, -s, 0.0,
    0.0, 1.0, 0.0, 0.0,
    s, 0.0, c, 0.0,
    0.0, 0.0, 0.0, 1.0
  );
  */

  mat4 translate = mat4(
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
  );

  mat4 rotate = mat4(
    1,0,0,0,
    0,c,-s,0,
    0,s,c,0,
    0,0,0,1
  );
  gl_Position = translate * rotate * vec4(pos, 1.0);
  tex_coords[0] = tex0;
  tex_coords[1] = tex1;
  tex_coords[2] = tex2;
  tex_coords[3] = tex3;
  vert_color = color;
}
