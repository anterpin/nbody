#pragma once
#include "./utils.h"
#include <GL/glew.h>
#include <iostream>
#include <vector>
// do not remove this comment
#include <GL/gl.h>

class Texture {
  GLuint id;
  mutable int w, h;

public:
  Texture() { glCreateTextures(GL_TEXTURE_2D, 1, &id); }
  Texture(const Texture &) = delete;
  Texture operator=(const Texture &) = delete;
  ~Texture() { glDeleteTextures(1, &id); }
  int get_width() const { return w; }
  int get_height() const { return h; }
  void set(int _w, int _h, GLsizei levels, GLenum internalformat, GLenum format,
           GLenum type, const void *data) const {
    w = _w;
    h = _h;
    glTextureStorage2D(id, levels, internalformat, w, h);
    if (data)
      glTextureSubImage2D(id, 0, 0, 0, w, h, format, type, data);
  }
  void generate_mipmap() const { glGenerateTextureMipmap(id); }
  void bind_texture_unit(GLuint unit) const { glBindTextureUnit(unit, id); }
  void filter_params(GLint min = GL_LINEAR, GLint mag = GL_LINEAR) const {
    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, min);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, mag);
  }
  void wrap_params(GLint s = GL_CLAMP_TO_EDGE,
                   GLint t = GL_CLAMP_TO_EDGE) const {
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, s);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, t);
  }
  GLuint get_id() const { return id; }
};

class FBO {
  GLuint id;

public:
  FBO() { glCreateFramebuffers(1, &id); }
  FBO(const FBO &) = delete;
  FBO operator=(const FBO &) = delete;
  ~FBO() { glDeleteFramebuffers(1, &id); }
  void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, id); }
  void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
  void attach_depth(const Texture &tex) const {
    glNamedFramebufferTexture(id, GL_DEPTH_ATTACHMENT, tex.get_id(), 0);
    check(glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER) ==
              GL_FRAMEBUFFER_COMPLETE,
          "Cannot complete the frame buffer");
  }
  void attach_texture(const Texture &tex) const {
    glNamedFramebufferTexture(id, GL_COLOR_ATTACHMENT0, tex.get_id(), 0);
    check(glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER) ==
              GL_FRAMEBUFFER_COMPLETE,
          "Cannot complete the frame buffer");
  }
  void clear_color(float r = 0, float g = 0, float b = 0, float a = 1) const {
    float colors[4] = {r, g, b, a};
    glClearNamedFramebufferfv(id, GL_COLOR, 0, colors);
  }
  void clear_depth() const {
    float colors[4] = {0, 0, 0, 1.0};
    glClearNamedFramebufferfv(id, GL_DEPTH, 0, colors);
  }
};

template <typename T> class VBO {
  GLuint id;

public:
  VBO() { glCreateBuffers(1, &id); }
  VBO(const VBO &) = delete;
  VBO operator=(const VBO &) = delete;
  ~VBO() { glDeleteBuffers(1, &id); }
  GLuint get_id() const { return id; }
  void data(const std::vector<T> &arr, GLuint usage = GL_DYNAMIC_DRAW) const {
    glNamedBufferData(id, sizeof(T) * arr.size(), arr.data(), usage);
  }
  void load(const std::vector<T> &arr, GLenum bits = 0) const {
    glNamedBufferStorage(id, sizeof(T) * arr.size(), arr.data(), bits);
  }
  void bind_shader_storage_buffer(GLuint bind, size_t size) const {
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bind, id, 0,
                      sizeof(glm::vec4) * size);
  }
  void get_data_from_gpu(std::vector<T> &arr) const {
    glGetNamedBufferSubData(id, 0, sizeof(T) * arr.size(), arr.data());
  }
};

class VAO {
protected:
  GLuint id;

public:
  VAO() { glCreateVertexArrays(1, &id); }
  void bind() const { glBindVertexArray(id); }
  template <typename T>
  void link_vbo(const VBO<T> &vbo, GLuint attrib_index) const {
    glEnableVertexArrayAttrib(id, attrib_index);
    glVertexArrayAttribBinding(id, attrib_index, attrib_index);
    glVertexArrayAttribFormat(id, attrib_index, sizeof(T) / sizeof(float),
                              GL_FLOAT, GL_FALSE, 0);
    glVertexArrayVertexBuffer(id, attrib_index, vbo.get_id(), 0, sizeof(T));
    attrib_index++;
  }
  VAO(const VAO &) = delete;
  VAO operator=(const VAO &) = delete;
  ~VAO() { glDeleteVertexArrays(1, &id); }
};
