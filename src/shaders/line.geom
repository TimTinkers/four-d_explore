#version 430

layout(lines) in;
layout(line_strip, max_vertices = 2) out;

layout(location = 0) out vec4 geo_color;

vec4 MakeColor(float c) {
  return vec4(-c, min(-c, c), c, 0) + vec4(1.0);
}

void main() {

  vec4 p[2];
  p[0] = gl_in[0].gl_Position;
  p[1] = gl_in[1].gl_Position;
  
  if (p[0].w > 1.0 && p[1].w > 1.0) {
    return;
  }
  if (p[0].w < -1.0 && p[1].w < -1.0) {
    return;
  }

  gl_Position = vec4(p[0].xyz,1);
  geo_color = MakeColor(p[0].w);
  EmitVertex();

  gl_Position = vec4(p[1].xyz,1);
  geo_color = MakeColor(p[1].w);
  EmitVertex();

  EndPrimitive();
}
