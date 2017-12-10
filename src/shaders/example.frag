#version 430

layout(location = 0) in  vec4 geo_color;
layout(location = 0) out vec4 result;

void main() {
  result = geo_color;
}
