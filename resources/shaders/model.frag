#version 410 core

uniform vec3 in_color;

in vec2 tex;

uniform sampler2D tex0;

out vec4 frag_color;

void main() {
  frag_color = vec4(tex, 0, 1);
  frag_color = vec4(in_color, 1);
  frag_color = texture(tex0, tex);
  // frag_color = vec4(1,0,0,1);
}
