#pragma once
#include "./utils.h"
#include <GL/glew.h>
#include <algorithm>
#include <exception>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

static std::unordered_map<std::string, GLenum> shader_types;

class ShaderProgram {
  GLuint id = 0;
  std::vector<GLuint> shaders;
  static std::pair<bool, std::string> read_file(const std::string &filename) {
    std::ifstream is;
    is.open(filename);
    if (is.is_open()) {
      std::stringstream ss;
      ss << is.rdbuf();
      return {true, ss.str()};
    }
    return {false, ""};
  }
  void init_shader_types() {
    shader_types["vert"] = GL_VERTEX_SHADER;
    shader_types["frag"] = GL_FRAGMENT_SHADER;
    shader_types["geom"] = GL_GEOMETRY_SHADER;
    shader_types["comp"] = GL_COMPUTE_SHADER;
  }

public:
  ShaderProgram() { id = glCreateProgram(); }
  ~ShaderProgram() { glDeleteProgram(id); }
  ShaderProgram(const ShaderProgram &) = delete;
  ShaderProgram operator=(const ShaderProgram &) = delete;
  void parse(const std::string &filename) {
    GLint success;
    GLchar info_log[2048];
    auto res = read_file(filename);
    check(res.first, std::string("Cannot read the file named" + filename));

    const auto &code = res.second;
    const char *s = code.c_str();

    const int pos = std::min(filename.rfind('.') + 1, filename.size());
    const auto ext = filename.substr(pos, filename.size() - pos);
    if (shader_types.empty())
      init_shader_types();
    auto type_it = shader_types.find(ext);
    check(type_it != shader_types.end(),
          std::string("Unknown shader extension type" + ext));
    auto shader_type = type_it->second;

    GLuint shad_id = glCreateShader(shader_type);
    glShaderSource(shad_id, 1, &s, nullptr);
    glCompileShader(shad_id);
    glGetShaderiv(shad_id, GL_COMPILE_STATUS, &success);
    if (!success) {
      // error log
      glGetShaderInfoLog(shad_id, sizeof(info_log), nullptr, info_log);
      check(false, std::string("Cannot compile the file ") + filename +
                       " due to:" + info_log);
    }
    glAttachShader(id, shad_id);
    shaders.push_back(shad_id);
  }

  void use() const { glUseProgram(id); }

  void link() {
    GLint success;
    GLchar info_log[2048];

    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
      // error log
      glGetProgramInfoLog(id, sizeof(info_log), nullptr, info_log);
      check(false, std::string("Cannot link the program due to:") +
                       std::string(info_log));
    }
    for (auto shad_id : shaders)
      glDeleteShader(shad_id);
    shaders.clear();
  }
  GLuint get_id() const { return id; }
};
