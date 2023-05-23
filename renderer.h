#pragma once
#include "./barnes-hut.h"
#include "./buffers.h"
#include "./camera.h"
#include "./shader.h"
#include "./win.h"
#include <GL/gl.h>
#include <GL/glew.h>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <vector>

class TextureRenderer {
  ShaderProgram program;
  VAO vao;
  VBO<glm::vec2> vbo;

public:
  TextureRenderer() {
    program.parse("./shaders/texture.vert");
    program.parse("./shaders/texture.frag");
    program.link();

    vao.link_vbo(vbo, 0);

    std::vector<glm::vec2> vertices{glm::vec2{-1, -1}, glm::vec2{-1, 1},
                                    glm::vec2{1, 1}, glm::vec2{1, -1}};
    vbo.load(vertices);
  }
  void render(const Texture &tex, int w, int h,
              const FBO *fbo = nullptr) const {
    if (fbo) {
      fbo->bind();
    } else {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    glViewport(0, 0, w, h);
    program.use();
    vao.bind();
    tex.bind_texture_unit(0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    if (fbo) {
      fbo->unbind();
    }
  }
};

class LumRenderer {
  int w, h;
  ShaderProgram lum;
  FBO fbo;
  std::unique_ptr<Texture> attach;

public:
  LumRenderer(int _w, int _h) {
    lum.parse("./shaders/deferred.vert");
    lum.parse("./shaders/lum.frag");
    lum.link();
    set_dimensions(_w, _h);
  }

  void set_dimensions(int _w, int _h) {
    w = _w / 2;
    h = _h / 2;
    attach = std::make_unique<Texture>();
    attach->wrap_params();
    attach->filter_params(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    int lod = 31 - __builtin_clz(std::max(w, h));
    attach->set(w, h, lod + 1, GL_R16F, 0, 0, nullptr);

    fbo.attach_texture(*attach);
  }
  const Texture &get_texture() const { return *attach; }
  void render(const Texture &initial) const {
    glViewport(0, 0, w, h);
    fbo.bind();
    lum.use();
    initial.bind_texture_unit(0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    attach->generate_mipmap();
  }
};

class BlurRenderer {
  VAO vao;
  VBO<glm::vec2> vbo;
  FBO fbos[2];
  std::unique_ptr<Texture> attach_textures[2];
  int w, h;
  ShaderProgram blur;
  TextureRenderer rend;

public:
  BlurRenderer(int _w, int _h) {
    blur.parse("./shaders/deferred.vert");
    blur.parse("./shaders/blur.frag");
    blur.link();
    set_dimensions(_w, _h);

    vao.link_vbo(vbo, 0);

    std::vector<glm::vec2> arr = {
        glm::vec2(-1, -1),
        glm::vec2(-1, 1),
        glm::vec2(1, 1),
        glm::vec2(1, -1),
    };
    vbo.load(arr);
  }

  const Texture &get_texture() const { return *attach_textures[1]; }

  void set_dimensions(int _w, int _h) {
    w = _w;
    h = _h;

    for (int i = 0; i < 2; i++) {
      attach_textures[i] = std::make_unique<Texture>();
      const Texture &tex = *attach_textures[i];
      tex.filter_params();
      tex.wrap_params();
      tex.set(w, h, 1, GL_RGBA16F, 0, 0, nullptr);

      fbos[i].attach_texture(tex);
    }
  }

  void render(const Texture &initial) const {
    glViewport(0, 0, w, h);
    blur.use();
    vao.bind();

    initial.bind_texture_unit(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fbos[0].bind();
    glProgramUniform2i(blur.get_id(), 0, 1, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    attach_textures[0]->bind_texture_unit(0);
    fbos[1].bind();
    glProgramUniform2i(blur.get_id(), 0, 0, 1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }
};

class FlareRenderer {
  VAO vao;
  ShaderProgram program;
  Texture flare_tex;
  std::unique_ptr<Texture> attach;
  std::unique_ptr<Texture> depth;
  FBO fbo;

  int w, h;
  mutable int tex_size = 64;
  constexpr static float factor = 1.0;

public:
  FlareRenderer(int _w, int _h) {
    program.parse("./shaders/flare.vert");
    program.parse("./shaders/flare.geom");
    program.parse("./shaders/flare.frag");
    program.link();
    set_dimensions(_w, _h);

    std::vector<float> data = gen_flare_tex(tex_size);

    flare_tex.filter_params();
    flare_tex.set(tex_size, tex_size, 1, GL_R32F, GL_RED, GL_FLOAT,
                  data.data());
  }

  const Texture &get_texture() const { return *attach; }
  const Texture &get_depth() const { return *depth; }

  void set_size(int _size) const {
    tex_size = _size;
    glProgramUniform1f(program.get_id(), 4, factor);
    glProgramUniform2f(program.get_id(), 3, tex_size / (float)w,
                       factor * tex_size / (float)h);
  }

  void set_dimensions(int _w, int _h) {
    w = _w;
    h = _h;
    attach = std::make_unique<Texture>();
    attach->filter_params();
    attach->wrap_params();
    attach->set(w, h, 1, GL_RGBA16F, 0, 0, nullptr);

    depth = std::make_unique<Texture>();
    depth->set(w, h, 1, GL_DEPTH24_STENCIL8, 0, 0, nullptr);

    fbo.attach_texture(*attach);

    program.use();
    glProgramUniform2f(program.get_id(), 3, factor * tex_size / (float)w,
                       factor * tex_size / (float)h);
  }
  const VAO &get_vao() const { return vao; }

  void set_view_proj(const glm::mat4 &view_proj) const {
    glProgramUniformMatrix4fv(program.get_id(), 0, 1, GL_FALSE,
                              glm::value_ptr(view_proj));
  }

  void render(int n) const {
    fbo.bind();
    glViewport(0, 0, w, h);
    fbo.clear_color();
    fbo.clear_depth();

    program.use();
    vao.bind();
    flare_tex.bind_texture_unit(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    glDrawArrays(GL_POINTS, 0, n);
    glDisable(GL_BLEND);
  }
};

class TonemapRenderer {
  int w, h;
  ShaderProgram tonemap;
  FBO fbo;
  std::unique_ptr<Texture> attach;

public:
  TonemapRenderer(int _w, int _h) {
    tonemap.parse("./shaders/deferred.vert");
    tonemap.parse("./shaders/tonemap.frag");
    tonemap.link();
    set_dimensions(_w, _h);
  }

  void set_dimensions(int _w, int _h) {
    w = _w;
    h = _h;
    int lod = 31 - __builtin_clz(std::max(w / 2, h / 2));
    glProgramUniform1i(tonemap.get_id(), 0, lod);

    attach = std::make_unique<Texture>();
    attach->set(w, h, 1, GL_RGB8, 0, 0, nullptr);
    fbo.attach_texture(*attach);
  }

  const Texture &get_texture() const { return *attach; }
  const FBO &get_fbo() const { return fbo; }

  void render(const Texture &hdr, const Texture &blur,
              const Texture &lum) const {
    glViewport(0, 0, w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fbo.bind();
    tonemap.use();
    hdr.bind_texture_unit(0);
    blur.bind_texture_unit(1);
    lum.bind_texture_unit(2);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }
};

class Renderer {
  FlareRenderer flare_renderer;
  BlurRenderer blur_renderer;
  LumRenderer lum_renderer;
  TonemapRenderer tonemap_renderer;
  TextureRenderer rend;
  int w, h;

public:
  Renderer(int _w, int _h)
      : flare_renderer(_w, _h), blur_renderer(_w, _h), lum_renderer(_w, _h),
        tonemap_renderer(_w, _h), w{_w}, h{_h} {}

  void set_dimensions(int _w, int _h) {
    w = _w;
    h = _h;
    flare_renderer.set_dimensions(w, h);
    blur_renderer.set_dimensions(w, h);
    lum_renderer.set_dimensions(w, h);
    tonemap_renderer.set_dimensions(w, h);
  }
  LumRenderer &get_lum_renderer() { return lum_renderer; }
  BlurRenderer &get_blur_renderer() { return blur_renderer; }
  const TonemapRenderer &get_tone_renderer() const { return tonemap_renderer; }
  const FlareRenderer &get_flare_renderer() const { return flare_renderer; }

  void render(int n) const {
    flare_renderer.render(n);
    blur_renderer.render(flare_renderer.get_texture());
    lum_renderer.render(flare_renderer.get_texture());

    tonemap_renderer.render(flare_renderer.get_texture(),
                            blur_renderer.get_texture(),
                            lum_renderer.get_texture());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
};

class BarnesHutRenderer {
  VAO vao;
  ShaderProgram program;
  bool red = true;
  VBO<glm::vec3> vbo;
  void _render(int i, const BarnesHutTree &bh, bool contain) const {
    for (int q = 0; q < 8; q++) {
      if (bh.children[i][q]) {
        _render(bh.children[i][q], bh, contain);
      }
      // TODO draw no existing children
      if (bh.first_child[i] && !contain) {
        const float hw = bh.sizes[i] / 2;
        float x = (q & 1) ? bh.lbf[i].x + hw : bh.lbf[i].x;
        float y = (q & 2) ? bh.lbf[i].y + hw : bh.lbf[i].y;
        float z = (q & 4) ? bh.lbf[i].z + hw : bh.lbf[i].z;
        glProgramUniform3f(program.get_id(), 0, x, y, z);
        glProgramUniform1f(program.get_id(), 1, hw);
        glProgramUniform1i(program.get_id(), 3, 0);
        glDrawArrays(GL_LINE_STRIP, 0, 16);
      }
    }
    if (bh.first_child[i] == 0 && contain) {
      glProgramUniform3f(program.get_id(), 0, bh.lbf[i].x, bh.lbf[i].y,
                         bh.lbf[i].z);
      glProgramUniform1f(program.get_id(), 1, bh.sizes[i]);
      glProgramUniform1i(program.get_id(), 3, 1);
      glDrawArrays(GL_LINE_STRIP, 0, 16);
    }
  }
  int w, h;

public:
  BarnesHutRenderer(int _w, int _h) {
    using glm::vec3;
    program.parse("./shaders/box.vert");
    program.parse("./shaders/box.frag");
    program.link();

    vao.link_vbo(vbo, 0);
    std::vector<vec3> vertices = {
        vec3{0, 0, 0}, vec3{0, 1, 0}, vec3{1, 1, 0}, vec3{1, 0, 0},
        vec3{0, 0, 0}, vec3{0, 0, 1}, vec3{0, 1, 1}, vec3{0, 1, 0},
        vec3{1, 1, 0}, vec3{1, 1, 1}, vec3{0, 1, 1}, vec3{0, 0, 1},
        vec3{1, 0, 1}, vec3{1, 1, 1}, vec3{1, 0, 1}, vec3{1, 0, 0}};
    vbo.load(vertices);

    set_dimensions(_w, _h);
    glLineWidth(10);
  }
  void set_dimensions(int _w, int _h) {
    w = _w;
    h = _h;
  }
  void set_red(bool _red) { red = _red; }
  void set_view_proj(const glm::mat4 &view_proj) const {
    glProgramUniformMatrix4fv(program.get_id(), 2, 1, GL_FALSE,
                              glm::value_ptr(view_proj));
  }
  void render(const BarnesHutTree &bhtree, const FBO &_fbo) const {
    _fbo.bind();
    glViewport(0, 0, w, h);
    program.use();
    vao.bind();
    program.use();
    if (red)
      _render(0, bhtree, false);
    _render(0, bhtree, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
};
