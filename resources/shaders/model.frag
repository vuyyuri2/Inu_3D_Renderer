#version 410 core

uniform vec3 in_color;

in vec2 tex;

out vec4 frag_color;

void main() {
  frag_color = vec4(tex, 0, 1);
  frag_color = vec4(in_color, 1);
}
