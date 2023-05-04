#version 460 core

in vec2 coords;

out vec4 color;

layout (binding = 0) uniform sampler2D tex;

void main() {
  color = texture(tex, (coords + vec2(1, 1)) / 2);
}
