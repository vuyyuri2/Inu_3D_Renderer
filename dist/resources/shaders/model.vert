#version 410 core

layout (location = 0) in vec3 pos;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;
uniform float angle;

void main() {
  // gl_Position = projection * view * model * vec4(pos, 1.0);
  // float angle = 45;
  float c = cos(radians(angle));
  float s = sin(radians(angle));
  mat4 model = mat4(
    c, 0.0, -s, 0.0,
    0.0, 1.0, 0.0, 0.0,
    s, 0.0, c, 0.0,
    0.0, 0.0, 0.0, 1.0
  );
  gl_Position = model * vec4(pos, 1.0);
}

