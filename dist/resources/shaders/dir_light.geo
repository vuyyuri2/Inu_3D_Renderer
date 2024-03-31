#version 410 core

#define NUM_CASCADES 3

// layout (triangles, invocations = NUM_CASCADES) in;
layout (triangles) in;
// layout (triangles_strip, max_vertices = 3) out;
layout (points, max_vertices = 1) out;

// uniform mat4 light_views[NUM_CASCADES];
// uniform mat4 light_projs[NUM_CASCADES];

void main() {
  gl_Position = vec4(0,0,0,1);
  EmitVertex();
  EmitPrimitive();
}
