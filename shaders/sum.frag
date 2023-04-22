#version 430

in vec2 coords;
out vec4 color;

layout (location = 0) uniform sampler2D tex[5];
layout (location = 10) uniform int size;

void main() {
  vec2 cd = (coords + vec2(1, -1)) / 2;
  color = vec4(0, 0, 0, 1.0);
  for (int i = 0; i < size; i++) {
    color += (size - i) * texture(tex[i], cd);
  }
}
