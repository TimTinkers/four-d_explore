#version 430

layout (location = 0)      in  vec3 color;
layout (location = 1) flat in  int  instance_id;
layout (location = 0)      out vec4 result;

layout (push_constant) uniform PCLuminance
{
	vec4 value0;
	vec4 value1;
	vec4 value2;
	vec4 value3;
} pcLuminance;

void main() {
	int  index = instance_id / 4;
	vec4 luminance;
	result = vec4(color.xyz, 1.0);

	if (index == 0)
		luminance = pcLuminance.value0;
	else if (index == 1)
		luminance = pcLuminance.value1;
	else if (index == 2)
		luminance = pcLuminance.value2;
	else if (index == 3)
		luminance = pcLuminance.value3;

	result.w = luminance[instance_id % 4];
};