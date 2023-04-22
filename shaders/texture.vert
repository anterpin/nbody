#version 430

layout (location = 0) in vec2 pos;

out vec2 coords;

void main() {
  gl_Position = vec4(pos.xy, 0.0 , 1.0);
  coords = pos;
}
