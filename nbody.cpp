#include "./camera.h"
#include "./renderer.h"
#include "./shader.h"
#include "./utils.h"
#include "./win.h"
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>

std::function<void(GLFWwindow *, int, int)> resize;
void callback_wrap(GLFWwindow *win, int w, int h) { resize(win, w, h); }

int main() {
  Init init;
  {
    const int w = 640;
    const int h = 480;
    const float fov = 70;
    const float near = 1.f;
    App app(w, h, "N-Body");

    Camera camera(w, h, fov, near);

    TestRenderer renderer(w, h);
    renderer.set_proj(camera.get_proj());
    renderer.set_view(camera.get_view());

    resize = [&](GLFWwindow *win, int w, int h) {
      app.set_dimensions(w, h);
      camera.set_dimensions(w, h);
      renderer.set_dimensions(w, h);
      renderer.set_proj(camera.get_proj());
    };

    app.set_resize_callback(callback_wrap);

    VBO<glm::vec3> vbo;
    renderer.get_vao().link_vbo(vbo);

    auto pos = initialize_random(100);
    vbo.load(pos);

    while (!app.should_close()) {
      camera.test_on_input();
      camera.rotate_x(1.0);
      if (camera.has_changed())
        renderer.set_view(camera.get_view());

      renderer.render(pos.size());
    }
  }
}
