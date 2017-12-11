#version 430

layout(set = 0, binding = 0) buffer viewUniform {
  mat4 main_mat;
  vec4 column;
  vec4 row;
  float ww;
} view;

layout(location = 0) out vec4 geo_color;

vec4 positions[8] = vec4[](
  vec4(0,0,0,0), vec4(1,0,0,0),
  vec4(0,0,0,0), vec4(0,-1,0,0),
  vec4(0,0,0,0), vec4(0,0,1,0),
  vec4(0,0,0,0), vec4(0,0,0,-1)
);

void main() {
  vec4 in_pos = positions[gl_VertexIndex];
  vec4 cam_trans = -view.column * view.main_mat;
  in_pos = in_pos;
  vec4 out_pos = view.main_mat*in_pos;
  out_pos *= 0.25;
  out_pos.wz = out_pos.zw;
  out_pos.w = 1.0;
  out_pos.x *= 0.5625; // aspect ratio
  out_pos.z = 0.5;
  out_pos.x -= 0.75;
  out_pos.y += 0.75;

  gl_Position = out_pos;

  if (gl_VertexIndex < 2) {
    geo_color = vec4(1.0, 0.0, 0.0, 1.0);
  } else if (gl_VertexIndex < 4) {
    geo_color = vec4(0.0, 1.0, 0.0, 1.0);
  } else if (gl_VertexIndex < 6) {
    geo_color = vec4(1.0, 1.0, 1.0, 1.0);
  } else {
    geo_color = vec4(0.0, 0.0, 1.0, 1.0);
  }
}
