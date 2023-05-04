#version 460 core

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 vel;

layout (location = 0) uniform mat4 proj_view;

out vec4 pass_pos;
out vec3 colors;

void main() {
  pass_pos = proj_view * pos;
  colors = mix(vec3(0,0.4,1),vec3(1,0.2,1),clamp(dot(vel,vel)*0.0006,0,1));
}
