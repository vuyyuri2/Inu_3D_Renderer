#version 410 core

struct shader_tex {
  sampler2D samp; 
  int tex_id;
};

in vec2 tex_coords[4];

// uniform vec3 in_color;
uniform shader_tex base_color_tex;

in vec3 vert_color;

out vec4 frag_color;

void main() {
  if (base_color_tex.tex_id == -1) {
    frag_color = vec4(vert_color, 0);
  } else {
    frag_color = texture(base_color_tex.samp, tex_coords[base_color_tex.tex_id]);
  }
  // frag_color = vec4(1,0,0,1);
}
