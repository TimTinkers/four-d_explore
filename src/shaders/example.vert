#version 430

layout(location = 0)      in  vec4 in_color;
layout(location = 0) flat out vec4 fs_color;

layout(set = 0, binding = 0) buffer cubeOutputVertices {
	vec4 vertex_out[11];
};

void main() {
	vec4 vOut = vertex_out[gl_VertexIndex];
	fs_color = vec4(gl_VertexIndex / 8.0f, 0, 0, 1);
	gl_Position = vOut;
}
