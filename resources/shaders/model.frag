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
// in vec2 light_pass_depth_tex_coord0;

in vec4 light_rel_screen_pos1;
// in vec2 light_pass_depth_tex_coord1;

in vec4 light_rel_screen_pos2;
// in vec2 light_pass_depth_tex_coord2;

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

struct is_in_light_info_t {
  float amount_in_light;
  float closest_depth;
  float depth;
};

is_in_light_info_t is_in_light(light_data_t light_data, vec4 light_rel_pos) {
  is_in_light_info_t info;

  vec2 tex_coords = ((light_rel_pos.xy / light_rel_pos.w) + vec2(1,1)) / 2;

#if 0
  if ((light_data.light_active == 0) || tex_coords.x < 0 || tex_coords.x > 1 || tex_coords.y < 0 || tex_coords.y > 1) {
    info.amount_in_light = 0;
    info.closest_depth = 1;
    info.depth = 1;
    return info;
  }
#endif

  float amount_in_light = 0.0;
  float central_bias = 0.00001;
  // float non_central_bias = 0.001;
  float non_central_bias = central_bias;
  // float bias = 0.0;

  // z position of the vertex relative to the light, still [-1,1] for near to far
  info.depth = light_rel_pos.z / light_rel_pos.w;
  // depth [0,1] relative to light
  info.depth = (info.depth+1)/2;

#if 0
  info.tc = tex_coords;
  info.closest_depth = texture(light_data.depth_tex, info.tc).r;
  // info.closest_depth = texture(light_data.depth_tex, vec2(0.5,0.5)).r;
  if (info.depth < (info.closest_depth + bias)) {
    info.amount_in_light = 1 * light_data.light_active;
  } else {
    info.amount_in_light = 0;
  }
#else
  int pcf = 3;

  // float tex_bias = 0.001;
  float tex_bias = 0;
#if 1
  for (int x_offset = -(pcf/2); x_offset <= (pcf/2); x_offset++) {
    for (int y_offset = -(pcf/2); y_offset <= (pcf/2); y_offset++) {
#else
  pcf = 0;
  int x_off = 0;
  int y_off = -1;
  for (int x_offset = x_off; x_offset <= x_off; x_offset++) {
    for (int y_offset = y_off; y_offset <= y_off; y_offset++) {
#endif
      int bias_t = int(sign(abs(x_offset) + abs(y_offset)));
      float bias = (central_bias * (1-bias_t)) + (non_central_bias * bias_t);
      vec2 new_tex_coords = tex_coords + (vec2(x_offset * 0.00078125, y_offset * 0.00138889));
      // vec2 new_tex_coords = tex_coords;
      if (new_tex_coords.x < tex_bias || new_tex_coords.x > (1 - tex_bias) || new_tex_coords.y < tex_bias || new_tex_coords.y > (1-tex_bias)) continue;

      // depth buffer stores 0 to 1, for near to far respectively
      // so closest_depth is between 0 to 1
      // info.closest_depth = texture(light_data.depth_tex, new_tex_coords).r;
      info.closest_depth = textureOffset(light_data.depth_tex, tex_coords, ivec2(x_offset, y_offset)).r;
      // info.closest_depth = textureOffset(light_data.depth_tex, tex_coords, ivec2(0,0)).r;

      // z pos is closer to light than the texture sample says
      if (info.depth < (info.closest_depth + bias)) {
        // light
        amount_in_light += (1.0 / max(0.1, float(pcf * pcf)));
      }
    }
  }

  info.amount_in_light = min(max(0.0, amount_in_light), 1.0) * light_data.light_active;
#endif

  return info;
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
  float depth = light_rel_screen_pos.z / light_rel_screen_pos.w;
  depth = (depth+1)/2;
  if (depth > min_z_threshold) {
    // shadow
    frag_color.x *= 0.4;
    frag_color.y *= 0.4;
    frag_color.z *= 0.4;
  }
  */

  // float in_light0 = 0, in_light1 = 0, in_light2 = 0;

  // light_data_t lp0 = lights_plane_data[0];
  vec2 light_pass_depth_tex_coord0 = ((light_rel_screen_pos0.xy / light_rel_screen_pos0.w) + vec2(1,1)) / 2;
  is_in_light_info_t in_light0 = is_in_light(lights_plane_data[0],  light_rel_screen_pos0);
  // * lights_plane_data[0].light_active;
  // float in_light0 = is_in_light(lights_plane_data[0].depth_tex, light_rel_screen_pos0) * lights_plane_data[0].light_active;

  // light_data_t lp1 = lights_plane_data[1];
  vec2 light_pass_depth_tex_coord1 = (light_rel_screen_pos1.xy + vec2(1,1)) / 2;
  is_in_light_info_t in_light1 = is_in_light(lights_plane_data[1], light_rel_screen_pos1);
  //* lights_plane_data[1].light_active;
  // float in_light1 = is_in_light(lights_plane_data[1].depth_tex, light_rel_screen_pos1) * lights_plane_data[1].light_active;

  // light_data_t lp2 = lights_plane_data[2];
  vec2 light_pass_depth_tex_coord2 = (light_rel_screen_pos2.xy + vec2(1,1)) / 2;
  is_in_light_info_t in_light2 = is_in_light(lights_plane_data[2], light_rel_screen_pos2);
  // * lights_plane_data[2].light_active;
  // float in_light2 = is_in_light(lights_plane_data[2].depth_tex, light_rel_screen_pos2) * lights_plane_data[2].light_active;

  // uint final_in_light = uint(sign(in_light0 + in_light1 + in_light2));
#if 1

  float max_in_light = max(max(in_light0.amount_in_light, in_light1.amount_in_light), in_light2.amount_in_light);
  // max_in_light = max(min(max_in_light, 1.0), 0.0);
  // max_in_light = 1;

  float shadow_damp_factor = 0.4;
  // float multiplier = shadow_damp_factor + ((1-shadow_damp_factor) * final_in_light);
  // float multiplier = (max_in_light * shadow_damp_factor) + ((1-shadow_damp_factor) * max_in_light);
  // float multiplier = 1.0 - (total * (1.0 - shadow_damp_factor));
  float multiplier = ((1.0 - max_in_light) * shadow_damp_factor) + max_in_light;
  // multiplier = 0.5;

  frag_color.x *= multiplier;
  frag_color.y *= multiplier;
  frag_color.z *= multiplier;

  // frag_color = vec4(max_in_light, max_in_light, max_in_light, 1);
  // frag_color = vec4(multiplier, multiplier, multiplier, 1);
  // float v = pow(in_light0.depth, 100);
  // frag_color = vec4(v,v,v,1);

  // z position of the vertex relative to the light, still [-1,1] for near to far
  // float z_pos = light_rel_screen_pos0.z / light_rel_screen_pos0.w;
  // depth [0,1] relative to light
  // z_pos = (z_pos+1)/2;

#else
  if (final_in_light == 0) {
    frag_color.x *= 0.4;
    frag_color.y *= 0.4;
    frag_color.z *= 0.4;
  }
#endif
  // frag_color = vec4(in_light0.tc.x,0,0,1);
  // frag_color = vec4(0,in_light0.tc.y,0,1);
  // frag_color = vec4(light_pass_depth_tex_coord0.xy,0,1);
  float v = pow(in_light0.closest_depth, 80);
  // frag_color = vec4(v,v,v,1);

  // frag_color = quantize_color(frag_color);

  // float light_rel_z_pos = -light_rel_screen_pos.w;
  // float linear_z = (-(light_rel_z_pos + light_near)) / (light_far - light_near);
  // frag_color = vec4(linear_z, linear_z, linear_z, 1);
  
  if (override_color_bool == 1) {
    frag_color = vec4(1,1,1,1);
  }
}
