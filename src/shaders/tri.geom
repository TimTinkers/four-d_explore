#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in  vec4 vs_color[];
layout(location = 0) out vec4 geo_color;

void main() {
  /*
  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;
  vec4 p2 = gl_in[2].gl_Position;
  if (p0.w > 1.0 && p1.w > 1.0 && p2.w > 1.0) {
    return;
  }
  if (p0.w < -1.0 && p1.w < -1.0 && p2.w < -1.0) {
    return;
  }
  gl_Position = vec4(p0, 1);
  geo_color = 
  */
  float w0 = vs_color[0][2]-1;
  float w1 = vs_color[1][2]-1;
  float w2 = vs_color[2][2]-1;

  if (w0 > 1.0 && w1 > 1.0 && w2 > 1.0) {
    return;
  }
  if (w0 < -1.0 && w1 < -1.0 && w2 < -1.0) {
    return;
  }

  gl_Position = gl_in[0].gl_Position;
  geo_color = vs_color[0];
  EmitVertex();

  gl_Position = gl_in[1].gl_Position;
  geo_color = vs_color[1];
  EmitVertex();

  gl_Position = gl_in[2].gl_Position;
  geo_color = vs_color[2];
  EmitVertex();

  EndPrimitive();
}
