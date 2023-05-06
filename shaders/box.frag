#version 460 core

layout (location = 3) uniform int contain;
out vec4 color;

void main() {
  if (contain == 1)
    color = vec4(0.0, 1.0, 0.0, 1.0);
  else
    color = vec4(1.0, 0.0, 0.0, 1.0);
}
