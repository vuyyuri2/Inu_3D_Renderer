#version 410 core

struct shader_tex {
  sampler2D samp; 
  int tex_id;
};

uniform shader_tex base_color_tex;
uniform vec3 mesh_color;
uniform int use_mesh_color;

uniform int override_color_bool;

// uniform sampler2D light_pass_depth_tex;

struct light_data_t {
  sampler2D depth_tex;
  int light_active;
};

uniform light_data_t lights_plane_data[3];

in vec2 tex_coords[2];
in vec3 color;
in vec3 normal;

in vec4 light_rel_screen_pos0;
in vec2 light_pass_depth_tex_coord0;

in vec4 light_rel_screen_pos1;
in vec2 light_pass_depth_tex_coord1;

in vec4 light_rel_screen_pos2;
in vec2 light_pass_depth_tex_coord2;

// in vec2 light_pass_depth_tex_coord;
// in vec4 light_rel_screen_pos;

out vec4 frag_color;

vec4 quantize_color(vec4 c) {
  vec4 quantized = vec4(0,0,0,1);
  quantized.x = (16 * (int(c.x * 255) / int(16))) / 255.0;
  quantized.y = (16 * (int(c.y * 255) / int(16))) / 255.0;
  quantized.z = (16 * (int(c.z * 255) / int(16))) / 255.0;
  return quantized;
}

uint is_in_light(sampler2D depth_tex, vec2 tex_coords, vec4 light_rel_pos) {
  // depth buffer stores 0 to 1, for near to far respectively
  // so min_z_threshold is between 0 to 1
  if (tex_coords.x < 0 || tex_coords.x > 1 || tex_coords.y < 0 || tex_coords.y > 1) return 0;
  float closest_z = texture(depth_tex, tex_coords).r;
  // z position of the vertex relative to the light, still [-1,1] for near to far
  float z_pos = light_rel_pos.z / light_rel_pos.w;
  z_pos = (z_pos+1)/2;
  // float diff = z_pos - closest_z;
  // float s = sign(diff);
  
  float bias = 0.000001;
  if (z_pos > (closest_z + bias)) {
    // shadow
    return 0;
  }
  return 1;
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

  /*
  // depth buffer stores 0 to 1, for near to far respectively
  // so min_z_threshold is between 0 to 1
  float min_z_threshold = texture(light_pass_depth_tex, light_pass_depth_tex_coord).r;
  // z position of the vertex relative to the light, still [-1,1] for near to far
  float z_pos = light_rel_screen_pos.z / light_rel_screen_pos.w;
  z_pos = (z_pos+1)/2;
  if (z_pos > min_z_threshold) {
    // shadow
    frag_color.x *= 0.4;
    frag_color.y *= 0.4;
    frag_color.z *= 0.4;
  }
  */

  uint in_light0, in_light1, in_light2;

  // light_data_t lp0 = lights_plane_data[0];
  in_light0 = is_in_light(lights_plane_data[0].depth_tex, light_pass_depth_tex_coord0, light_rel_screen_pos0) & lights_plane_data[0].light_active;

  // light_data_t lp1 = lights_plane_data[1];
  in_light1 = is_in_light(lights_plane_data[1].depth_tex, light_pass_depth_tex_coord1, light_rel_screen_pos1) & lights_plane_data[1].light_active;

  // light_data_t lp2 = lights_plane_data[2];
  in_light2 = is_in_light(lights_plane_data[2].depth_tex, light_pass_depth_tex_coord2, light_rel_screen_pos2) & lights_plane_data[2].light_active;

  uint final_in_light = uint(sign(in_light0 + in_light1 + in_light2));
#if 1

  float shadow_damp_factor = 0.4;
  float multiplier = shadow_damp_factor + ((1-shadow_damp_factor) * final_in_light);
  // float multiplier = 1.0 - (total * (1.0 - shadow_damp_factor));

  frag_color.x *= multiplier;
  frag_color.y *= multiplier;
  frag_color.z *= multiplier;
#else
  if (final_in_light == 0) {
    frag_color.x *= 0.4;
    frag_color.y *= 0.4;
    frag_color.z *= 0.4;
  }
#endif
  // frag_color = vec4(in_shadow0,0,0,1);

  frag_color = quantize_color(frag_color);

  // float light_rel_z_pos = -light_rel_screen_pos.w;
  // float linear_z = (-(light_rel_z_pos + light_near)) / (light_far - light_near);
  // frag_color = vec4(linear_z, linear_z, linear_z, 1);
  
  if (override_color_bool == 1) {
    frag_color = vec4(1,1,1,1);
  }
}
