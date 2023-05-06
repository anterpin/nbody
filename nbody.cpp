#include "./barnes-hut.h"
#include "./camera.h"
#include "./renderer.h"
#include "./shader.h"
#include "./simulation.h"
#include "./utils.h"
#include "./win.h"
#include "imgui/imgui.h"
#include <glm/ext/vector_float3.hpp>

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

    std::unique_ptr<VBO<glm::vec4>> positions;
    std::unique_ptr<VBO<glm::vec4>> velocities;

    VBO<float> sizes;
    VBO<glm::vec4> com;
    VBO<int> children;
    VBO<int> next;

    BarnesHutTree bhtree;
    BarnesHutRenderer bhrenderer(w, h);
    bhrenderer.set_view_proj(camera.get_view_proj());

    Value<glm::vec2> size(glm::vec2{w, h});
    Value<int> n(3, true);
    Value<int> tex_size(6);
    Value<bool> sync(false);
    Value<bool> rotate(false);
    Value<float> sd(10.0, true);
    Value<float> G(0.4);
    Value<float> damping(1);
    Value<float> eps(0.4);
    Value<float> threshold(0.5);
    Value<float> dt(0.005);
    Value<bool> interaction(false);
    Value<bool> initial_vel(false, true);
    Value<bool> boxes(true);
    Value<bool> gpu(false, true);
    Value<bool> n2(false);

    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> vel;

    auto compute_tree = [&]() {
      bhtree.create_tree(pos);
      if (!n2) {
        sizes.data(bhtree.sizes);
        com.data(bhtree.com);
        children.data(bhtree.children);
        next.data(bhtree.next);

        size_t size = bhtree.lbf.size();
        sizes.bind_shader_storage_buffer(2, size);
        com.bind_shader_storage_buffer(3, size);
        children.bind_shader_storage_buffer(4, size);
        next.bind_shader_storage_buffer(5, size);
      }
    };

    auto init = [&]() {
      positions = std::make_unique<VBO<glm::vec4>>();
      velocities = std::make_unique<VBO<glm::vec4>>();
      float d;
      pos = initialize_random(n, d);
      bhtree.set_domain(d * 3);

      vel = initial_vel ? initialize_vel(n, sd)
                        : std::vector<glm::vec4>(n, glm::vec4(0, 0, 0, 0));

      if (gpu) {
        positions->load(pos, GL_CLIENT_STORAGE_BIT | GL_MAP_READ_BIT);
        velocities->load(vel);
      } else {
        positions->data(pos);
        velocities->data(vel);
      }

      positions->bind_shader_storage_buffer(0, n);
      velocities->bind_shader_storage_buffer(1, n);

      renderer.get_flare_renderer().get_vao().link_vbo(*positions, 0);
      renderer.get_flare_renderer().get_vao().link_vbo(*velocities, 1);
      compute_tree();
    };
    init();
    while (!app.should_close()) {
      if (size.has_changed()) {
        int w = size->x;
        int h = size->y;
        app.set_dimensions(w, h);
        camera.set_dimensions(w, h);
        renderer.set_dimensions(w, h);
        bhrenderer.set_dimensions(w, h);
      }
      camera.test_on_input();
      if (rotate)
        camera.rotate_x(1.0);

      if (camera.has_changed()) {
        renderer.get_flare_renderer().set_view_proj(camera.get_view_proj());
        bhrenderer.set_view_proj(camera.get_view_proj());
      }

      if (G.has_changed()) {
        inter.set_G(G);
      }
      if (damping.has_changed()) {
        inter.set_damping(damping);
      }
      if (eps.has_changed()) {
        inter.set_eps(eps);
      }
      if (dt.has_changed()) {
        inter.set_dt(dt);
      }
      if (n2.has_changed()) {
        inter.set_n2(n2);
      }

      if (threshold.has_changed()) {
        bhtree.set_threshold(threshold);
        inter.set_threshold(threshold);
      }

      // cannot be more than one at the same time
      if (initial_vel.has_changed() || n.has_changed() || sd.has_changed() ||
          gpu.has_changed()) {
        init();
      }

      if (tex_size.has_changed()) {
        renderer.get_flare_renderer().set_size(tex_size);
      }

      if (sync.has_changed())
        app.vsync(sync);

      if (interaction) {
        if (gpu) {
          inter.compute(n);
          positions->get_data_from_gpu(pos);
        } else {
          bhtree.update_pos_and_velocities(pos, vel, G, dt);
          positions->data(pos);
          velocities->data(vel);
        }
        /* *&interaction = false; */
        compute_tree();
      }

      renderer.render(n);
      if (boxes) {
        bhrenderer.render(bhtree, renderer.get_tone_renderer().get_fbo());
      }

#ifdef IMGUI
      ImGui::Begin("Options");
      ImGui::Checkbox("VSync", &sync);
      ImGui::Checkbox("Rotate", &rotate);
      ImGui::Checkbox("Boxes", &boxes);
      ImGui::Checkbox("Gpu", &gpu);
      ImGui::Checkbox("O(n^2)", &n2);
      ImGui::Checkbox("Initial velocity", &initial_vel);
      ImGui::Checkbox("Interaction", &interaction);
      ImGui::SliderInt("N", &n, 1, 50000);
      ImGui::SliderInt("Size", &tex_size, 2, 32);
      ImGui::SliderFloat("Damping", &damping, 0.96, 1.0);
      ImGui::SliderFloat("Sd", &sd, 1.0, 30);
      ImGui::SliderFloat("G", &G, -3, 3);
      ImGui::SliderFloat("EPS squared", &eps, 0.0001, 10);
      ImGui::SliderFloat("dt", &dt, -0.01, 0.01);
      ImGui::SliderFloat("Threshold", &threshold, 0, 1);
      ImGui::Text("Distance: %.3f", camera.get_distance());
      ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate,
                  io.Framerate);

      {
        ImGui::BeginChild("GameRender");
        ImVec2 wsize = ImGui::GetWindowSize();
        const Texture &tmp_tex = renderer.get_flare_renderer().get_depth();
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
