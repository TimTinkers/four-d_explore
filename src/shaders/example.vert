#version 430

layout(location = 0)      in  vec4 in_color;
layout(location = 0) flat out vec4 fs_color;

layout(std430, set = 0, binding = 0) restrict readonly buffer evenSineSB {
	vec4 vertex_sine1[N_VERTICES_PER_SINE];
};

layout(std430, set = 1, binding = 0) restrict readonly buffer oddSineSB {
	vec4 vertex_sine2[N_VERTICES_PER_SINE];
};

void main() {
	fs_color = in_color;
	switch (gl_InstanceIndex % 2) {
		case 0: gl_Position = vertex_sine1[gl_VertexIndex]; break;
		case 1: gl_Position = vertex_sine2[gl_VertexIndex];
	}
}