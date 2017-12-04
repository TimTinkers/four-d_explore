#version 430

layout(location = 0)      in  vec4 in_color;
layout(location = 0) flat out vec4 fs_color;

layout(set = 0, binding = 0) buffer cubeOutputVertices {
	vec4 vertex_out[8];
};

void main() {
	fs_color = in_color;
}