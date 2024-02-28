#version 410 core

struct shader_tex {
  sampler2D samp; 
  int tex_id;
};

in vec2 tex_coords[4];

uniform vec3 in_color;
uniform shader_tex base_color_tex;

out vec4 frag_color;

void main() {
  frag_color = vec4(in_color, 1);
  frag_color = texture(base_color_tex.samp, tex_coords[base_color_tex.tex_id]);
  // frag_color = vec4(1,0,0,1);
}
