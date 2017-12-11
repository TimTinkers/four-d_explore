#version 430

//layout(location = 0) in vec4 in_color;
//layout(location = 0) out vec4 vs_color;

layout(set = 0, binding = 0) buffer cubeOutputVertices {
  vec4 vertex_out[N_MESHES * N_VERTICES];
};

void main() {
  vec4 vOut = vertex_out[gl_VertexIndex];
  gl_Position = vOut;
}
