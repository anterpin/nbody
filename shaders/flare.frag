#version 460 core

in vec2 uv;
in vec3 in_color;
out vec4 color;

layout (binding = 0) uniform sampler2D tex;

void main() {
  float transp = texture(tex, uv).r;
  color = vec4(in_color * transp, 1.0);
}
