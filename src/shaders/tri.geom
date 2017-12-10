#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 5) out;

//layout(location = 0) in  vec4 vs_color[];
layout(location = 0) out vec4 geo_color;

vec4 MakeColor(float c) {
  return vec4(-c, min(-c, c), c, 0) + vec4(1.0);
}

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
  //float w0 = vs_color[0][2]-1;
  //float w1 = vs_color[1][2]-1;
  //float w2 = vs_color[2][2]-1;

  vec4 p[5];
  p[0] = gl_in[0].gl_Position;
  p[1] = gl_in[1].gl_Position;
  p[2] = gl_in[2].gl_Position;
  p[3] = vec4(-10.0);
  p[4] = vec4(-10.0);
  
  // Handle if all vertices are off in same direction
  // Drop entire triangle.
  if (p[0].w > 1.0 && p[1].w > 1.0 && p[2].w > 1.0) {
    return;
  }
  if (p[0].w < -1.0 && p[1].w < -1.0 && p[2].w < -1.0) {
    return;
  }

  // Handle if two vertices are off in same direction.
  // Move vertices to edge. No extra vertices.
  /*if ((p[0].w > 1.0 && p[1].w > 1.0) || (p[0].w < -1.0 && p[1].w < -1.0)) {
    float s0 = (sign(p[0].w) - p[0].w) / (p[2].w - p[0].w);
    vec4 n0 = p[0] + s0 * (p[2] - p[0]);

    float s1 = (sign(p[1].w) - p[1].w) / (p[2].w - p[1].w);
    vec4 n1 = p[1] + s1 * (p[2] - p[1]);
    p[0] = n0;
    p[1] = n1;
  }

  if ((p[1].w > 1.0 && p[2].w > 1.0) || (p[1].w < -1.0 && p[2].w < -1.0)) {
    float s1 = (sign(p[1].w) - p[1].w) / (p[0].w - p[1].w);
    vec4 n1 = p[1] + s1 * (p[0] - p[1]);

    float s2 = (sign(p[2].w) - p[2].w) / (p[0].w - p[2].w);
    vec4 n2 = p[2] + s2 * (p[0] - p[2]);
    p[1] = n1;
    p[2] = n2;
  }

  if ((p[0].w > 1.0 && p[2].w > 1.0) || (p[0].w < -1.0 && p[2].w < -1.0)) {
    float s0 = (sign(p[0].w) - p[0].w) / (p[1].w - p[0].w);
    vec4 n0 = p[0] + s0 * (p[1] - p[0]);

    float s2 = (sign(p[2].w) - p[2].w) / (p[1].w - p[2].w);
    vec4 n2 = p[2] + s2 * (p[1] - p[2]);
    p[0] = n0;
    p[2] = n2;
  }

  // Handle if only one vertex is off or they are off in different directions.
  int v_count = 3;
  for (int i = 0; i < v_count; ++i) {
    if (p[i].w > 1 || p[i].w < -1.0) {
      int prev_index = (i + v_count - 1) % v_count;
      int next_index = (i + 1) % v_count;
      float s1 = (sign(p[i].w) - p[i].w) / (p[next_index].w - p[i].w);
      vec4 n1 = p[i] + s1 * (p[next_index] - p[i]);

      float s2 = (sign(p[i].w) - p[i].w) / (p[prev_index].w - p[i].w);
      vec4 n2 = p[i] + s2 * (p[prev_index] - p[i]);
      for(int j = v_count-1; j >= i; --j) {
        p[j] = p[j-1];
      }
      p[i+1] = n2;
      p[i] = n1;
    }
  }*/

  // Emit all vertices that exist.
  // Order is because of triangle strip.
  gl_Position = vec4(p[1].xyz,1);
  geo_color = MakeColor(p[1].w);
  EmitVertex();

  gl_Position = vec4(p[2].xyz,1);
  geo_color = MakeColor(p[2].w);
  EmitVertex();

  gl_Position = vec4(p[0].xyz,1);
  geo_color = MakeColor(p[0].w);
  EmitVertex();

  if (p[3] != vec4(-10)) {
    gl_Position = vec4(p[3].xyz, 1);
    geo_color = MakeColor(p[3].w);
    EmitVertex();
  }

  if (p[4] != vec4(-10)) {
    gl_Position = vec4(p[4].xyz, 1);
    geo_color = MakeColor(p[4].w);
    EmitVertex();
  }

  EndPrimitive();
}
