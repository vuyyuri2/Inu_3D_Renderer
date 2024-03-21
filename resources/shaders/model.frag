#version 410 core

struct shader_tex {
  sampler2D samp; 
  int tex_id;
};

uniform shader_tex base_color_tex;
uniform vec3 mesh_color;
uniform int use_mesh_color;

uniform int override_color_bool;

uniform sampler2D light_pass_depth_tex;

uniform float light_near;
uniform float light_far;

in vec2 tex_coords[2];
in vec3 color;
in vec3 normal;
in vec2 light_pass_depth_tex_coord;
// in float z_pos;
in vec4 light_rel_screen_pos;

out vec4 frag_color;

vec4 quantize_color(vec4 c) {
  vec4 quantized = vec4(0,0,0,1);
  quantized.x = (16 * (int(c.x * 255) / int(16))) / 255.0;
  quantized.y = (16 * (int(c.y * 255) / int(16))) / 255.0;
  quantized.z = (16 * (int(c.z * 255) / int(16))) / 255.0;
  return quantized;
}

void main() {

  if (base_color_tex.tex_id == -1) {
    if (use_mesh_color == 1) {
      frag_color = vec4(mesh_color, 1);
    } else {
      frag_color = vec4(color, 1);
    }
  } else {
    frag_color = texture(base_color_tex.samp, tex_coords[base_color_tex.tex_id]);
  }

  // depth buffer stores 0 to 1, for near to far respectively
  // so min_z_threshold is between 0 to 1
  float min_z_threshold = texture(light_pass_depth_tex, light_pass_depth_tex_coord).r;
  // float v5 = pow(v,5);
  // frag_color = vec4(v5,v5,v5,1);
  // z position of the vertex relative to the light, still [-1,1] for near to far
  float z_pos = light_rel_screen_pos.z / light_rel_screen_pos.w;
  z_pos = (z_pos+1)/2;
  if (z_pos > min_z_threshold) {
    // shadow
    frag_color.x *= 0.4;
    frag_color.y *= 0.4;
    frag_color.z *= 0.4;
    // frag_color = vec4(1,0,0,1);
  } else {
    // not shadow
    // frag_color = vec4(0,1,0,1);
  }
  frag_color = quantize_color(frag_color);
  // float new_z = (z_pos+1) / 2;
  // float v5 = pow(new_z,300);
  // frag_color = vec4(v5,v5,v5,1);
  // frag_color = vec4(new_z, new_z, new_z, 1);

  float light_rel_z_pos = -light_rel_screen_pos.w;
  float linear_z = (-(light_rel_z_pos + light_near)) / (light_far - light_near);
  // frag_color = vec4(linear_z, linear_z, linear_z, 1);
  
  // frag_color = vec4(normal, 1);
  // frag_color = vec4(mesh_color, 1);
  // frag_color = vec4(1,0,0,1); 

  if (override_color_bool == 1) {
    frag_color = vec4(1,1,1,1);
  }
}
