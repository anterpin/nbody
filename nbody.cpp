#include "./camera.h"
#include "./renderer.h"
#include "./shader.h"
#include "./simulation.h"
#include "./utils.h"
#include "./win.h"
#include "imgui/imgui.h"
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>

using glm::vec3;
class CubeRenderer {
  ShaderProgram program;
  VAO vao;
  VBO<glm::vec3> vbo;
  FBO fbo;
  std::unique_ptr<Texture> attach;
  std::vector<vec3> vertices;

public:
  CubeRenderer(int w, int h) {
    vertices = {vec3(-0.5f, 0.5f, -0.5f),
                vec3(-0.5f, -0.5f, -0.5f),
                vec3(0.5f, -0.5f, -0.5f),
                vec3(0.5f, 0.5f, -0.5f),

                vec3(-0.5f, 0.5f, 0.5f),
                vec3(-0.5f, -0.5f, 0.5f),
                vec3(0.5f, -0.5f, 0.5f),
                vec3(0.5f, 0.5f, 0.5f),

                vec3(0.5f, 0.5f, -0.5f),
                vec3(0.5f, -0.5f, -0.5f),
                vec3(0.5f, -0.5f, 0.5f),
                vec3(0.5f, 0.5f, 0.5f),

                vec3(-0.5f, 0.5f, -0.5f),
                vec3(-0.5f, -0.5f, -0.5f),
                vec3(-0.5f, -0.5f, 0.5f),
                vec3(-0.5f, 0.5f, 0.5f),

                vec3(-0.5f, 0.5f, 0.5f),
                vec3(-0.5f, 0.5f, -0.5f),
                vec3(0.5f, 0.5f, -0.5f),
                vec3(0.5f, 0.5f, 0.5f),

                vec3(-0.5f, -0.5f, 0.5f),
                vec3(-0.5f, -0.5f, -0.5f),
                vec3(0.5f, -0.5f, -0.5f),
                vec3(0.5f, -0.5f, 0.5f)

    };
    vbo.data(vertices);
    vao.link_vbo(vbo);
    set_dimension(w, h);
  }
  void set_dimension(int _w, int _h) {
    attach = std::make_unique<Texture>();
    attach->set(_w, _h, 0, GL_RGBA8, 0, 0, nullptr);
    fbo.attach_texture(*attach);
  }
  void render() const {
    program.use();
    fbo.bind();
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
  }
};

int main() {
  Init init;
  {
    const int w = 640;
    const int h = 480;
    const float fov = 70;
    const float near = 1.f;

    App app(w, h, "N-Body");

    Camera camera(w, h, fov, near);

    Interaction inter;
    Renderer renderer(w, h);
    renderer.get_flare_renderer().set_view_proj(camera.get_view_proj());

    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    VBO<glm::vec4> positions;
    VBO<glm::vec4> velocities;
    renderer.get_flare_renderer().get_vao().link_vbo(positions);
    renderer.get_flare_renderer().get_vao().link_vbo(velocities);

    Value<glm::vec2> size(glm::vec2{w, h});
    Value<int> n(2000);
    Value<int> tex_size(6);
    Value<bool> sync(false);
    Value<bool> rotate(false);
    Value<float> sd(10.0);
    Value<float> G(0.4);
    Value<float> damping(0.8);
    Value<float> eps(0.4);
    Value<float> dt(0.005);
    Value<bool> interaction(true);
    Value<bool> initial_vel(false);

    auto init = [&]() {
      auto pos = initialize_random(n);
      positions.data(pos);

      auto vel = initial_vel ? initialize_vel(n, sd)
                             : std::vector<glm::vec4>(n, glm::vec4(0, 0, 0, 0));
      velocities.data(vel);

      positions.bind_shader_storage_buffer(0, n);
      velocities.bind_shader_storage_buffer(1, n);
    };
    init();

    while (!app.should_close()) {
      if (size.has_changed()) {
        int w = size->x;
        int h = size->y;
        app.set_dimensions(w, h);
        camera.set_dimensions(w, h);
        renderer.set_dimensions(w, h);
      }
      camera.test_on_input();
      if (rotate)
        camera.rotate_x(1.0);

      if (camera.has_changed())
        renderer.get_flare_renderer().set_view_proj(camera.get_view_proj());

      if (sd.has_changed()) {
        init();
      }
      if (G.has_changed()) {
        inter.set_G(G);
      }
      if (damping.has_changed()) {
        inter.set_damping(damping);
      }
      if (eps.has_changed()) {
        inter.set_eps(std::pow(eps, 1.0f / 6.0f));
      }
      if (dt.has_changed()) {
        inter.set_dt(dt);
      }

      if (initial_vel.has_changed()) {
        init();
      }

      if (n.has_changed()) {
        init();
      }
      if (tex_size.has_changed()) {
        renderer.get_flare_renderer().set_size(tex_size);
      }
      if (sync.has_changed())
        app.vsync(sync);

      if (interaction)
        inter.compute(n);
      renderer.render(n);

#ifdef IMGUI
      ImGui::Begin("Options");
      ImGui::Text("Just simple text");
      ImGui::Checkbox("VSync", &sync);
      ImGui::Checkbox("Rotate", &rotate);
      ImGui::Checkbox("Initial velocity", &initial_vel);
      ImGui::Checkbox("Interaction", &interaction);
      ImGui::SliderInt("N", &n, 100, 100000);
      ImGui::SliderInt("Size", &tex_size, 2, 32);
      ImGui::SliderFloat("Damping", &damping, 0.0, 1.0);
      ImGui::SliderFloat("Sd", &sd, 1.0, 30);
      ImGui::SliderFloat("G", &G, -3, 3);
      ImGui::SliderFloat("EPS squared", &eps, 0.0001, 10);
      ImGui::SliderFloat("dt", &dt, -0.01, 0.01);
      ImGui::Text("Distance: %.3f", camera.get_distance());
      ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate,
                  io.Framerate);

      {
        ImGui::BeginChild("GameRender");
        ImVec2 wsize = ImGui::GetWindowSize();
        const Texture &tmp_tex = renderer.get_tone_renderer().get_texture();
        wsize.y =
            (int)(wsize.x * (float)tmp_tex.get_height() / tmp_tex.get_width());
        ImGui::Image((ImTextureID)(uint64_t)tmp_tex.get_id(), wsize,
                     ImVec2(0, 1), ImVec2(1, 0));
        ImGui::EndChild();
      }

      ImGui::End();
      ImGui::Begin("Main Window");
      {
        ImGui::BeginChild("Render");
        input.hovered = ImGui::IsWindowHovered();
        ImVec2 wsize = ImGui::GetWindowSize();
        size->x = wsize.x;
        size->y = wsize.y;
        const Texture &tmp_tex = renderer.get_tone_renderer().get_texture();
        ImGui::Image((ImTextureID)(uint64_t)tmp_tex.get_id(), wsize,
                     ImVec2(0, 1), ImVec2(1, 0));
        ImGui::EndChild();
      }

      ImGui::End();

#endif
    }
  }
}
