#include "model_internal.h"

offline_to_online_vertex_t create_offline_to_online_vertex(vec2 pos, vec2 tex) {
  offline_to_online_vertex_t v;
  v.position = pos;
  v.tex = tex;
  return v;
}
