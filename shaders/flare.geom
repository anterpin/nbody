#version 460 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

layout (location = 3) uniform vec2 flare_size;
layout (location = 4) uniform float factor;

in vec4 pass_pos[];
in vec3 colors[];

out vec2 uv;
out vec3 in_color;

void main() {
  vec4 pos = pass_pos[0];
  vec2 offset = pos.w * flare_size;
  in_color = colors[0];

  gl_Position = pos + vec4(-offset.x, offset.y, 0, 0);
  uv = vec2(0, 1);
  EmitVertex();

  gl_Position = pos + vec4(-offset.x, -offset.y, 0, 0);
  uv = vec2(0, 0);
  EmitVertex();

  gl_Position = pos + vec4(offset.x, offset.y, 0, 0);
  uv = vec2(1, 1);
  EmitVertex();

  gl_Position = pos + vec4(offset.x, -offset.y, 0, 0);
  uv = vec2(1, 0);
  EmitVertex();

  EndPrimitive();
}
