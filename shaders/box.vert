#version 460 core

layout (location = 0) in vec3 pos;

layout (location = 0) uniform vec3 lbf;
layout (location = 1) uniform float width;
layout (location = 2) uniform mat4 proj_view;

void main() {
  gl_Position = proj_view * vec4(lbf + (width * pos), 1.0);
}
