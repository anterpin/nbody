#include "./barnes-hut.h"
#include "./camera.h"
#include "./renderer.h"
#include "./shader.h"
#include "./simulation.h"
#include "./utils.h"
#include "./win.h"
#include "imgui/imgui.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <glm/ext/vector_float3.hpp>

std::function<void(int, int)> resize_callback;
void resize_callback_wrap(GLFWwindow *win, int w, int h) {
  resize_callback(w, h);
}

int main() {
  Init init;
  {
    int w = 640;
    int h = 480;
    const float fov = 70;
    const float near = 1.f;

    App app(w, h, "N-Body");
    app.get_sizes(w, h);

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
    VBO<int> sorted;

    BarnesHutTree bhtree;
    BarnesHutRenderer bhrenderer(w, h);
    bhrenderer.set_view_proj(camera.get_view_proj());

    Value<glm::vec2> size(glm::vec2{w, h}, glm::vec2{w + 1, h});
    Value<int> n(3, 3); // just a random value to not be equal
    Value<int> tex_size(6, 5);
    Value<bool> sync(false, true);
    Value<float> sd(10.0, 10.0);
    Value<float> G(0.4, 0.3);
    Value<float> damping(9998.0, 123);
    Value<float> eps2(0.4, 123);
    Value<float> threshold(0.5, 123);
    Value<float> dt(0.005, 123);
    Value<bool> interaction(false, false);
    Value<bool> boxes(true, false);
    Value<bool> red(false, true);
    Value<bool> gpu(true, true);
    Value<bool> n2(false, true);
    Value<bool> sort(true, false);
    Value<float> eps(0.01, true);
    bhtree.set_sort(sort);

    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> vel;

    auto compute_tree = [&]() {
      bhtree.create_tree(pos, vel);
      if (!n2) {
        sizes.data(bhtree.sizes);
        com.data(bhtree.com);
        children.data(bhtree.first_child);
        next.data(bhtree.next);
        sorted.data(bhtree.sorted);

        size_t size = bhtree.lbf.size();
        sizes.bind_shader_storage_buffer(2, size);
        com.bind_shader_storage_buffer(3, size);
        children.bind_shader_storage_buffer(4, size);
        next.bind_shader_storage_buffer(5, size);
        sorted.bind_shader_storage_buffer(6, n);
      }
    };

    auto init = [&]() {
      positions = std::make_unique<VBO<glm::vec4>>();
      velocities = std::make_unique<VBO<glm::vec4>>();
      float d;
      pos = initialize_pos(n, d);
      bhtree.set_domain(d * 3);

      vel = initialize_vel(pos);

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

    bool gui = false;
    resize_callback = [&](int _w, int _h) {
      app.get_sizes(w, h);
      size->x = w;
      size->y = h;
    };
    app.set_resize_callback(resize_callback_wrap);

    TextureRenderer texture_renderer;
    auto start = std::chrono::high_resolution_clock::now();
    int fps = 0;
    while (!app.should_close(gui)) {
      static bool press = false;
      if (input.keys[GLFW_KEY_SPACE] != GLFW_RELEASE) {
        if (!press) {
          start = std::chrono::high_resolution_clock::now();
          fps = 0;
          gui = !gui;
          app.get_sizes(w, h);
          size->x = w;
          size->y = h;
          press = true;
          input.hovered = false;
        }
      } else {
        input.hovered = true;
        press = false;
      }
      if (size.has_changed()) {
        int w = size->x;
        int h = size->y;
        app.set_dimensions(w, h);
        camera.set_dimensions(w, h);
        renderer.set_dimensions(w, h);
        bhrenderer.set_dimensions(w, h);
      }

      camera.test_on_input();

      if (camera.has_changed()) {
        renderer.get_flare_renderer().set_view_proj(camera.get_view_proj());
        bhrenderer.set_view_proj(camera.get_view_proj());
      }
      if (red.has_changed()) {
        bhrenderer.set_red(red);
      }

      if (G.has_changed()) {
        inter.set_G(G);
      }
      if (damping.has_changed()) {
        inter.set_damping(damping / 10000.0);
        bhtree.set_damping(damping / 10000.0);
      }
      if (eps2.has_changed()) {
        inter.set_eps(eps2);
        bhtree.set_eps2(eps2);
      }
      if (eps.has_changed()) {
        bhtree.set_eps(eps);
      }

      if (dt.has_changed()) {
        inter.set_dt(dt);
      }
      if (n2.has_changed()) {
        inter.set_n2(n2);
      }

      if (sort.has_changed()) {
        bhtree.set_sort(sort);
        inter.set_sort(sort);
      }
      if (threshold.has_changed()) {
        bhtree.set_threshold(threshold);
        inter.set_threshold(threshold);
      }

      // cannot be more than one at the same time
      if (n.has_changed() || gpu.has_changed()) {
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
          float ma = 0;
          for (int i = 0; i < n; i++) {
            ma = std::max(ma, max(glm::vec3{pos[i].x, pos[i].y, pos[i].z}));
          }
          bhtree.set_domain(ma * 3);
        } else {
          bhtree.update_pos_and_velocities(pos, vel, G, dt, n2);
          positions->data(pos);
          velocities->data(vel);
        }
        compute_tree();
      }

      renderer.render(n);
      if (boxes) {
        bhrenderer.render(bhtree, renderer.get_tone_renderer().get_fbo());
      }

      if (!gui) {
        const auto &tex = renderer.get_tone_renderer().get_texture();
        texture_renderer.render(tex, w, h);
        auto end = std::chrono::high_resolution_clock::now();
        fps++;
        if (end - start >= std::chrono::duration<double>(1.0)) {
          std::cout << "FPS " << fps << '\n';
          start = end;
          fps = 0;
        }
        continue;
      }
#ifdef IMGUI
      ImGui::Begin("Options");
      if (ImGui::CollapsingHeader("General")) {
        ImGui::Checkbox("Gpu", &gpu);
        ImGui::Checkbox("O(n^2)", &n2);
        ImGui::Checkbox("Sort", &sort);
        ImGui::Checkbox("Interaction", &interaction);
        ImGui::SliderInt("N", &n, 1, 100000);
      }
      static bool first = true;
      if (first) {
        ImGui::GetStateStorage()->SetInt(ImGui::GetID("General"), 1);
        first = false;
      }

      if (ImGui::CollapsingHeader("Render")) {
        ImGui::Checkbox("VSync", &sync);
        ImGui::Checkbox("Boxes", &boxes);
        ImGui::Checkbox("With red boxes", &red);
        ImGui::SliderInt("Flare size", &tex_size, 2, 32);
      }
      if (ImGui::CollapsingHeader("Barnes Hut")) {
        ImGui::SliderFloat("Eps", &eps, 0.01, 0.1);
        ImGui::SliderFloat("Threshold", &threshold, 0, 1);
        ImGui::Text("Tree Size: %d", (int)bhtree.next.size());
      }
      if (ImGui::CollapsingHeader("Physics")) {
        ImGui::SliderFloat("Damping", &damping, 9990, 10000);
        ImGui::SliderFloat("G", &G, 0, 3);
        ImGui::SliderFloat("EPS squared", &eps2, 0.01, 10);
        ImGui::SliderFloat("dt", &dt, 0.0001, 0.01);
      }
      ImGui::Text("Camera distance: %.3f", camera.get_distance());
      ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate,
                  io.Framerate);

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
