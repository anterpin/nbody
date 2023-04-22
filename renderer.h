#include "./shader.h"
#include <GL/gl.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <vector>

class Texture {
  GLuint id;
  mutable int w, h;

public:
  Texture() { glGenTextures(1, &id); }
  Texture(const Texture &) = delete;
  Texture operator=(const Texture &) = delete;
  ~Texture() { glDeleteTextures(1, &id); }
  void bind() const { glBindTexture(GL_TEXTURE_2D, id); }
  void activate(int i = 0) const {
    glActiveTexture(GL_TEXTURE0 + i);
    bind();
  }
  int get_width() const { return w; }
  int get_height() const { return h; }
  void set_888(int _w, int _h) const {
    w = _w;
    h = _h;
    bind();
    param();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 nullptr);
  }
  void set_11_11_10(int w, int h) const {
    bind();
    param();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, w, h, 0, GL_RGB, GL_FLOAT,
                 nullptr);
  }
  void param() const {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT); // set texture wrapping to GL_REPEAT (defau
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    /* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, */
    /*                 GL_LINEAR_MIPMAP_LINEAR); */
    /* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); */
  }
  GLuint get_id() const { return id; }
};

class FBO {
  GLuint id;
  Texture tex;

public:
  int get_id() const { return id; };
  FBO() { glCreateFramebuffers(1, &id); }
  void set(int w, int h) const {
    bind();
    /* tex.set_11_11_10(w, h); */
    tex.set_888(w, h);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           tex.get_id(), 0);
    check(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
          "Cannot complete the frame buffer");
    unbind();
  }
  FBO(const FBO &) = delete;
  FBO operator=(const FBO &) = delete;
  ~FBO() { glDeleteFramebuffers(1, &id); }
  void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, id); }
  void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
  const Texture &get_texture() const { return tex; }
};

template <typename T> class VBO {
  GLuint id;

public:
  VBO() { glCreateBuffers(1, &id); }
  VBO(const VBO &) = delete;
  VBO operator=(const VBO &) = delete;
  ~VBO() { glDeleteBuffers(1, &id); }
  void bind() const { glBindBuffer(GL_ARRAY_BUFFER, id); }
  void load(const std::vector<T> &arr, GLuint usage = GL_DYNAMIC_DRAW) const {
    bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(T) * arr.size(),
                 (const void *)arr.data(), usage);
  }
  /* GLuint get_id() const { return id; } */
};

class VAO {
protected:
  GLuint id;
  GLuint layout = 0;

public:
  VAO() { glCreateVertexArrays(1, &id); }
  void bind() const { glBindVertexArray(id); }
  template <typename T> void link_vbo(const VBO<T> &vbo) {
    bind();
    vbo.bind();
    /* glVertexArrayAttribFormat(vao_part, layout, sizeof(T), GL_FLOAT,
     * GL_FALSE, */
    /*                           0); */
    /* glVertexArrayAttribBinding(vao_part, layout, 0); */
    glVertexAttribPointer(layout, sizeof(T) / sizeof(float), GL_FLOAT, GL_FALSE,
                          sizeof(T), nullptr);
    glEnableVertexAttribArray(layout);
    layout++;
  }
  VAO(const VAO &) = delete;
  VAO operator=(const VAO &) = delete;
  /* ~VAO() { glDeleteVertexArrays(1, &id); } */
};

class SumRenderer {
  ShaderProgram sum_program;
  VAO sum_vao;
  VBO<glm::vec2> quad_vbo;

public:
  SumRenderer() {
    sum_program.parse("./shaders/sum.vert");
    sum_program.parse("./shaders/sum.frag");
    sum_program.link();

    glProgramUniform1i(sum_program.get_id(), 0, 0);
    glProgramUniform1i(sum_program.get_id(), 1, 1);
    glProgramUniform1i(sum_program.get_id(), 2, 2);
    glProgramUniform1i(sum_program.get_id(), 3, 3);
    glProgramUniform1i(sum_program.get_id(), 4, 4);
    glProgramUniform1i(sum_program.get_id(), 5, 5);

    sum_vao.link_vbo(quad_vbo);

    std::vector<glm::vec2> vertices{glm::vec2{-1, -1}, glm::vec2{-1, 1},
                                    glm::vec2{1, 1},   glm::vec2{-1, -1},
                                    glm::vec2{1, 1},   glm::vec2{1, -1}};
    quad_vbo.load(vertices);
  }
  void render(const std::vector<std::unique_ptr<FBO>> &fbos, int w, int h,
              const FBO *fbo = nullptr) const {
    if (fbo) {
      fbo->bind();
    }
    glViewport(0, 0, w, h);
    sum_program.use();
    for (int i = 0; i < fbos.size(); i++) {
      fbos[i]->get_texture().activate(i);
    }
    glProgramUniform1i(sum_program.get_id(), 10, fbos.size());
    sum_vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    if (fbo) {
      fbo->unbind();
    }
  }
};

class TextureRenderer {
  ShaderProgram tex_program;
  VAO tex_vao;
  VBO<glm::vec2> quad_vbo;

public:
  TextureRenderer() {
    tex_program.parse("./shaders/texture.vert");
    tex_program.parse("./shaders/texture.frag");
    tex_program.link();

    glProgramUniform1i(tex_program.get_id(), 0, 0);

    tex_vao.link_vbo(quad_vbo);

    std::vector<glm::vec2> vertices{glm::vec2{-1, -1}, glm::vec2{-1, 1},
                                    glm::vec2{1, 1},   glm::vec2{-1, -1},
                                    glm::vec2{1, 1},   glm::vec2{1, -1}};
    quad_vbo.load(vertices);
  }
  void render(const Texture &tex, int w, int h,
              const FBO *fbo = nullptr) const {
    if (fbo) {
      fbo->bind();
    }
    glViewport(0, 0, w, h);
    /* glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); */
    /* glClearColor(0.0, 0.0, 0.0, 1.0); */
    tex_program.use();
    tex.activate();
    tex_vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    if (fbo) {
      fbo->unbind();
    }
  }
};

class TestRenderer {
  ShaderProgram test;
  ShaderProgram sum;
  std::vector<std::unique_ptr<FBO>> fbos;
  VAO test_vao;
  int w;
  int h;

  TextureRenderer tex_rend;
  SumRenderer sum_rend;

public:
  TestRenderer(int _w, int _h) : w{_w}, h{_h} {
    test.parse("./shaders/test.vert");
    test.parse("./shaders/test.frag");
    test.link();

    set_dimensions(w, h);
  }

  void set_dimensions(int _w, int _h) {
    fbos.clear();
    w = _w;
    h = _h;
    while (_w > 10) {
      fbos.emplace_back(new FBO());
      fbos.back()->set(_w, _h);
      _w /= 2;
      _h /= 2;
    }
  }

  VAO &get_vao() { return test_vao; }

  void set_proj(const glm::mat4 &proj) const {
    glProgramUniformMatrix4fv(test.get_id(), 0, 1, GL_FALSE,
                              glm::value_ptr(proj));
  }

  void set_view(const glm::mat4 &view) const {
    glProgramUniformMatrix4fv(test.get_id(), 1, 1, GL_FALSE,
                              glm::value_ptr(view));
  }
  void render(int size) const {
    check(!fbos.empty(), "FBOS empty");
    fbos[0]->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    test.use();
    test_vao.bind();
    glDrawArrays(GL_POINTS, 0, size);
    fbos[0]->unbind();

    for (int i = 1; i < fbos.size(); i++) {
      int w = fbos[i]->get_texture().get_width();
      int h = fbos[i]->get_texture().get_height();
      tex_rend.render(fbos[i - 1]->get_texture(), w, h, fbos[i].get());
    }
    /* tex_rend.render(fbos[0]->get_texture(), w, h); */
    sum_rend.render(fbos, w, h);
  }
};
