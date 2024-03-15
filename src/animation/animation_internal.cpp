#include "animation_internal.h"

#include "utils/app_info.h"
#include "scene/scene.h"
#include "utils/log.h"
#include "interpolation.h"
#include "utils/general.h"
#include "windowing/window.h"

#include <unordered_set>
#include <algorithm>

extern std::vector<object_t> objs;

animation_globals_t animation_globals;
extern app_info_t app_info;
extern window_t window;

static std::vector<animation_data_chunk_t> anim_data_chunks;

static std::unordered_set<float> timestamps_set;
static int idx = 30;
static std::vector<float> timestamps;

animation_data_chunk_t* get_anim_data_chunk(int data_id) {
  return &anim_data_chunks[data_id];
}

int register_anim_data_chunk(animation_data_chunk_t& data) {
  data.id = anim_data_chunks.size();
  animation_globals.anim_end_time = fmax(animation_globals.anim_end_time, data.timestamps[data.num_timestamps-1]);
  anim_data_chunks.push_back(data);
  timestamps_set.insert(data.timestamps.begin(), data.timestamps.end());
  timestamps.clear();
  timestamps.insert(timestamps.end(), timestamps_set.begin(), timestamps_set.end());
  std::sort(timestamps.begin(), timestamps.end());
  return data.id;
}

void update_animations() {
#if 1
  animation_globals.anim_time += app_info.delta_time;
  if (animation_globals.anim_time > animation_globals.anim_end_time) {
    animation_globals.anim_time = animation_globals.anim_start_time;
  }
#else
  if (window.input.left_mouse_up) {
    idx++;
    if (idx >= timestamps.size()) {
      idx = 0;
    }
    printf("anim idx: %i\n", idx);
    animation_globals.anim_time = timestamps[idx];
  }
#endif

  for (object_t& obj : objs) { 
    vec3 orig = obj.transform.pos;
    quaternion_t orig_rot = obj.transform.rot;
    for (animation_chunk_data_ref_t& ref : obj.anim_chunk_refs) {	
      animation_data_chunk_t* chunk = get_anim_data_chunk(ref.chunk_id);

      quaternion_t* rot_anim_data = static_cast<quaternion_t*>((void*)chunk->keyframe_data); 
      vec3* vec3_data = static_cast<vec3*>((void*)chunk->keyframe_data);

      int left_anim_frame_idx = -1;
      int right_anim_frame_idx = -1;
      for (int i = 0; i < chunk->num_timestamps; i++) {
        if (animation_globals.anim_time >= chunk->timestamps[i]) {
          left_anim_frame_idx = i;
          right_anim_frame_idx = i+1;
        }
      }

      if (left_anim_frame_idx != -1) {
        right_anim_frame_idx = clamp(right_anim_frame_idx, 0, chunk->num_timestamps-1);
      }

      if (left_anim_frame_idx == -1) {
        // OUTSIDE ANIMATION FRAMES, CLOSEST ONE IS THE FIRST FRAME

        // quaternion interpolation
        if (ref.target == ANIM_TARGET_ON_NODE::ROTATION) {
          // printf("rot to left of leftmost anim frame\n");
          obj.transform.rot = rot_anim_data[0];
          obj.transform.rot = norm_quat(obj.transform.rot);
        }
        // position interpolation
		    else if (ref.target == ANIM_TARGET_ON_NODE::POSITION) {
          obj.transform.pos = vec3_data[0];
		    }
        // scale interpolation
		    else if (ref.target == ANIM_TARGET_ON_NODE::SCALE) {
          obj.transform.scale = vec3_data[0];
		    }
      } else if (left_anim_frame_idx == chunk->num_timestamps-1) {
        // OUTSIDE ANIMATION FRAMES, CLOSEST ONE IS THE LAST FRAME

        // quaternion interpolation
        if (ref.target == ANIM_TARGET_ON_NODE::ROTATION) {
          // printf("rot to right of rightmost anim frame\n");
          obj.transform.rot = rot_anim_data[left_anim_frame_idx];
          obj.transform.rot = norm_quat(obj.transform.rot);
        }
        // position interpolation
		    else if (ref.target == ANIM_TARGET_ON_NODE::POSITION) {
          obj.transform.pos = vec3_data[left_anim_frame_idx];
		    }
        // scale interpolation
		    else if (ref.target == ANIM_TARGET_ON_NODE::SCALE) {
          obj.transform.scale = vec3_data[left_anim_frame_idx];
		    }
      } else {
        // ANIMATING BETWEEN FRAMES
        float frame_duration = chunk->timestamps[right_anim_frame_idx] - chunk->timestamps[left_anim_frame_idx];
        float time_into_frame = animation_globals.anim_time - chunk->timestamps[left_anim_frame_idx];
        float t = 0;
        // if there is not just one frame
        if (frame_duration != 0) {
          t = time_into_frame / frame_duration;
        }

        // quaternion interpolation
        if (ref.target == ANIM_TARGET_ON_NODE::ROTATION) {
          if (chunk->interpolation_mode == ANIM_INTERPOLATION_MODE::LINEAR) {
            // printf("spherical lin    left quat: ");
            // print_quat(rot_anim_data[left_anim_frame_idx]);
            // printf("right quat: ");
            // print_quat(rot_anim_data[right_anim_frame_idx]);
              if (strstr(obj.name.c_str(), "neck") != NULL) {
                  int a = 5;
            }
            obj.transform.rot = spherical_linear(rot_anim_data[left_anim_frame_idx], rot_anim_data[right_anim_frame_idx], t);

            float quat_len = quat_mag(obj.transform.rot);
            if (isnan(obj.transform.rot.x) || isnan(obj.transform.rot.y) || isnan(obj.transform.rot.z) || isnan(obj.transform.rot.w) || quat_len > 1.02f) {
              int a = 5;
              quaternion_t l = rot_anim_data[left_anim_frame_idx];
              quaternion_t r = rot_anim_data[right_anim_frame_idx];
              quaternion_t test = spherical_linear(l, r, t);
            }
              

          } else if (chunk->interpolation_mode == ANIM_INTERPOLATION_MODE::STEP) {
            obj.transform.rot = rot_anim_data[left_anim_frame_idx];
          }
          obj.transform.rot = norm_quat(obj.transform.rot);
        }
		    // position interpolation
		    else if (ref.target == ANIM_TARGET_ON_NODE::POSITION) {
		      if (chunk->interpolation_mode == ANIM_INTERPOLATION_MODE::LINEAR) {
			      obj.transform.pos = vec3_linear(vec3_data[left_anim_frame_idx], vec3_data[right_anim_frame_idx], t);
            if (isnan(obj.transform.pos.x) || isnan(obj.transform.pos.y) || isnan(obj.transform.pos.z)) {
              int a = 5;
			        vec3 l = vec3_data[left_anim_frame_idx];
			        vec3 r = vec3_data[right_anim_frame_idx];
			        vec3 f = vec3_linear(l, r, t);
            }
		      } else if (chunk->interpolation_mode == ANIM_INTERPOLATION_MODE::STEP) {
			      obj.transform.pos = vec3_data[left_anim_frame_idx];
		      }
		    }
		    // scale interpolation
		    else if (ref.target == ANIM_TARGET_ON_NODE::SCALE) {
          if (chunk->interpolation_mode == ANIM_INTERPOLATION_MODE::LINEAR) {
			      obj.transform.scale = vec3_linear(vec3_data[left_anim_frame_idx], vec3_data[right_anim_frame_idx], t);
		      } else if (chunk->interpolation_mode == ANIM_INTERPOLATION_MODE::STEP) {
			      obj.transform.scale = vec3_data[left_anim_frame_idx];
		      }
		    }
		  }
    } 
    vec3 diff = {obj.transform.pos.x - orig.x, obj.transform.pos.y - orig.y, obj.transform.pos.z - orig.z };
    inu_assert(length(obj.transform.scale) != 0, "scale is 0");
    if (isnan(obj.transform.pos.x) || isnan(obj.transform.pos.y) || isnan(obj.transform.pos.z)) {
      int a = 5;
    }
    if (isnan(obj.transform.rot.x) || isnan(obj.transform.rot.y) || isnan(obj.transform.rot.z) || isnan(obj.transform.rot.w)) {
      int a = 5;
    }
  }

  // update object transforms
  update_obj_model_mats();
}

