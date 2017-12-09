#version 430

layout(location = 0) in vec4 in_color;
layout(location = 0) flat out vec4 fs_color;

layout(set = 0, binding = 0) buffer cubeOutputVertices {
  vec4 vertex_out[N_MESHES * N_VERTICES];
};

void main() {
  vec4 vOut = vertex_out[gl_VertexIndex];
  //fs_color = vec4(gl_VertexIndex / float(N_MESHES * N_VERTICES), 0, 0, 1);
  float c = vOut.w;
  if (c > 0) {
    fs_color = vec4(0, 0, c, 0);
  } else {
    fs_color = vec4(-c, 0, 0, 0);
  }
  gl_Position = vec4(vOut.xyz, 1);
}
