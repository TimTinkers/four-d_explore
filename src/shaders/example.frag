#version 430

layout(location = 0) flat in  vec4 fs_color;
layout(location = 0)      out vec4 result;

void main() {
	result = fs_color;
}