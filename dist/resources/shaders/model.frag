#version 410 core

struct shader_tex {
  sampler2D samp; 
  int tex_id;
};

in vec2 tex_coords[4];

uniform shader_tex base_color_tex;
uniform vec3 mesh_color;
uniform int use_mesh_color;

in vec3 vert_color;

out vec4 frag_color;

void main() {
  if (base_color_tex.tex_id == -1) {
    if (use_mesh_color == 1) {
      frag_color = vec4(mesh_color, 1);
    } else {
      frag_color = vec4(vert_color, 1);
    }
  } else {
    frag_color = texture(base_color_tex.samp, tex_coords[base_color_tex.tex_id]);
  }
  // frag_color = vec4(mesh_color, 1);
  // frag_color = vec4(1,0,0,1);
}
