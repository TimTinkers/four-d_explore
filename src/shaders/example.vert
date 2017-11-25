#version 430

layout(location = 0) in vec4 vertexData;
layout(location = 1) in vec3 colorData;

layout(location = 0)      out vec3 result_color;
layout(location = 1) flat out int  result_instance_id;

layout(std140, binding = 0) uniform dataUB {
  ivec4 frame_index;
  vec4 position_rotation[N_TRIANGLES]; /* XY position, XY rotation */
  vec4 size[N_TRIANGLES / 4];
};

layout(push_constant) uniform PCLuminance {
  vec4 value0;
  vec4 value1;
  vec4 value2;
  vec4 value3;
} pcLuminance;

void main() {
  int index = gl_InstanceIndex / 4;
  vec4 luminance;

  if (index == 0)
    luminance = pcLuminance.value0;
  else if (index == 1)
    luminance = pcLuminance.value1;
  else if (index == 2)
    luminance = pcLuminance.value2;
  else if (index == 3)
    luminance = pcLuminance.value3;

  result_color =
      colorData + vec3(0.0, 0.0, 1.0 - luminance[gl_InstanceIndex % 4]);
  result_instance_id = gl_InstanceIndex;

  vec4 result_position = vec4(vertexData.xy, 0.0, 1.0);
  vec2 cos_factor = cos(position_rotation[gl_InstanceIndex].zw);
  vec2 sin_factor = sin(position_rotation[gl_InstanceIndex].zw);

  result_position.xy =
      vec2(dot(vertexData.xy, vec2(cos_factor.x, - sin_factor.y)),
           dot(vertexData.xy, vec2(sin_factor.x, cos_factor.y)));

  switch (gl_InstanceIndex % 4) {
    case 0:
      result_position.xy *= vec2(size[index].x);
      break;
    case 1:
      result_position.xy *= vec2(size[index].y);
      break;
    case 2:
      result_position.xy *= vec2(size[index].z);
      break;
    case 3:
      result_position.xy *= vec2(size[index].w);
      break;
  }

  result_position.xy += position_rotation[gl_InstanceIndex].xy;
  gl_Position = result_position;
}
