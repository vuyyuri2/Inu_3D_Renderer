#include "animation_internal.h"

#include "utils/app_info.h"
#include "scene/scene.h"
#include "utils/log.h"
#include "interpolation.h"

extern std::vector<object_t> objs;

animation_globals_t animation_globals;
extern app_info_t app_info;

static std::vector<animation_data_chunk_t> anim_data_chunks;

int register_anim_data_chunk(animation_data_chunk_t& data) {
  data.id = anim_data_chunks.size();
  animation_globals.anim_end_time = fmax(animation_globals.anim_end_time, data.timestamps[data.num_timestamps-1]);
  anim_data_chunks.push_back(data);
  return data.id;
}

void update_animations() {
  animation_globals.anim_time += app_info.delta_time;
  if (animation_globals.anim_time > animation_globals.anim_end_time) {
    animation_globals.anim_time = animation_globals.anim_start_time;
    printf("animation_globals.anim_time was reset\n");
  }

  for (object_t& obj : objs) {
    for (animation_chunk_data_ref_t& ref : obj.anim_chunk_refs) {
      animation_data_chunk_t* chunk = get_anim_data_chunk(ref.chunk_id);

      quaternion_t* rot_anim_data = static_cast<quaternion_t*>(chunk->keyframe_data); 
		  vec3* vec3_data = static_cast<vec3*>(chunk->keyframe_data);

      int left_anim_frame_idx = -1;
      int right_anim_frame_idx = -1;
      for (int i = 0; i < chunk->num_timestamps; i++) {
        if (animation_globals.anim_time >= chunk->timestamps[i]) {
          left_anim_frame_idx = i;
          right_anim_frame_idx = i+1;
        }
      }

      if (left_anim_frame_idx == -1) {
        // OUTSIDE ANIMATION FRAMES, CLOSEST ONE IS THE FIRST FRAME

        // quaternion interpolation
        if (ref.target == ANIM_TARGET_ON_NODE::ROTATION) {
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
        float t = time_into_frame / frame_duration;

        // quaternion interpolation
        if (ref.target == ANIM_TARGET_ON_NODE::ROTATION) {
          if (chunk->interpolation_mode == ANIM_INTERPOLATION_MODE::LINEAR) {
            obj.transform.rot = spherical_linear(rot_anim_data[left_anim_frame_idx], rot_anim_data[right_anim_frame_idx], t);
          } else if (chunk->interpolation_mode == ANIM_INTERPOLATION_MODE::STEP) {
            obj.transform.rot = rot_anim_data[left_anim_frame_idx];
          }
          obj.transform.rot = norm_quat(obj.transform.rot);
        }
		    // position interpolation
		    else if (ref.target == ANIM_TARGET_ON_NODE::POSITION) {
		      if (chunk->interpolation_mode == ANIM_INTERPOLATION_MODE::LINEAR) {
			      obj.transform.pos = vec3_linear(vec3_data[left_anim_frame_idx], vec3_data[right_anim_frame_idx], t);
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
  }

  // update object transforms
  update_obj_model_mats();
}

animation_data_chunk_t* get_anim_data_chunk(int id) {
  return &anim_data_chunks[id];
}
