#version 430

layout(location = 0) in vec4 in_color;
layout(location = 0) flat out vec4 fs_color;

layout(set = 0, binding = 0) buffer cubeOutputVertices {
  vec4 vertex_out[N_VERTICES];
};

void main() {
  vec4 vOut = vertex_out[gl_VertexIndex];
  fs_color = vec4(gl_VertexIndex / float(N_VERTICES), 0, 0, 1);
  //fs_color = vertex_out[gl_VertexIndex];
  //vec4 vOut = vec4(gl_VertexIndex / float(N_VERTICES), 0, 0, 1);
  gl_Position = vOut;
}
