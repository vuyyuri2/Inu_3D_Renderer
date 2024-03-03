#version 410 core

in vec2 out_tex;

uniform sampler2D img;

out vec4 frag_color;

void main() {
  frag_color = texture(img, out_tex);
}
