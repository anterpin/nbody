#pragma once
#include "./utils.h"
#include "./win.h"
#include <functional>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
  int w, h;
  float FOV, NEAR;
  glm::vec2 spher{0.0, 0.0};
  float step = 0.01f;
  float rho = 5.0f;
  bool changed = false;

public:
  Camera(int _w, int _h, float _FOV, float _NEAR)
      : w{_w}, h{_h}, FOV{_FOV}, NEAR{_NEAR} {

                                 };

  void set_dimensions(int _w, int _h) {
    w = _w;
    h = _h;
  }
  void test_on_input() {
    if (input.mouse_buttons[GLFW_MOUSE_BUTTON_LEFT] != GLFW_RELEASE) {
      if (input.d_mouse_pos[0] != 0 || input.d_mouse_pos[1] != 0) {
        spher.x += step * input.d_mouse_pos[0];
        spher.y += step * input.d_mouse_pos[1];
        const double lim = 3.141592653 / 2.5;
        if (spher.y < -lim)
          spher.y = -lim;
        if (spher.y > lim)
          spher.y = lim;
        changed = true;
      }
    }
    if (input.scroll[1] != 0) {
      rho *= (1.0 - 10 * step * input.scroll[1]);
      changed = true;
    }
  }
  void rotate_x(float vel) {
    spher.x += step * vel;
    changed = true;
  }

  float get_aspect() const { return w / (float)h; }
  glm::mat4 get_proj() const {
    return glm::infinitePerspective(glm::radians(FOV), get_aspect(), NEAR);
  }

  glm::mat4 get_view() const {
    glm::vec3 pos = rho * glm::vec3(sin(spher.x) * cos(spher.y), sin(spher.y),
                                    -cos(spher.x) * cos(spher.y));
    glm::vec3 center{0.0, 0.0, 0.0};
    return glm::lookAt(pos, center, glm::vec3{0.0, 1.0, 0.0});
  }

  bool has_changed() {
    bool tmp = changed;
    changed = false;
    return tmp;
  }
};
