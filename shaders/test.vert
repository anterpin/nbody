#version 430 core

layout (location = 0) in vec3 pos;

layout (location = 1) uniform mat4 view;
layout (location = 0) uniform mat4 proj;


void main() {
  gl_Position = proj * view * vec4(pos.xyz, 1.0);
}

