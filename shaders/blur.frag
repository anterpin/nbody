#version 460 core

layout (binding = 0) uniform sampler2D tex;

layout (location = 0) uniform ivec2 dir;

in vec2 pass_tc;

out vec4 color;

const float gaussian_25[25] = {
 0.0000037811696529388428,
 0.00002552289515733719,
 0.00013271905481815338,
 0.0005529960617423058,
 0.0018959864974021912,
 0.0054509611800313,
 0.013324571773409843,
 0.02798160072416067,
 0.05087563768029213,
 0.08055309299379587,
 0.11153505183756351,
 0.13543542008846998,
 0.14446444809436798,
 0.13543542008846998,
 0.11153505183756351,
 0.08055309299379587,
 0.05087563768029213,
 0.02798160072416067,
 0.013324571773409843,
 0.0054509611800313,
 0.0018959864974021912,
 0.0005529960617423058,
 0.00013271905481815338,
 0.00002552289515733719,
 0.0000037811696529388428
};

ivec2 get_coords(vec2 current, int i) {
  ivec2 size = textureSize(tex, 0);  
  //ivec2 size = ivec2(640, 480);
  ivec2 buffer_offset = i * dir;
  return ivec2(current.x * size.x, current.y * size.y) + buffer_offset;
}

void main() {

  color = vec4(0.0);

  float norm = 0.;
  for (int i = -12; i < 12; i++)
  {
      float k = gaussian_25[i + 12];
      color += k * texelFetch(tex, get_coords(pass_tc, i), 0); 
      norm += k;
  }
}
