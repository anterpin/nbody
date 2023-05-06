#pragma once
#include <bits/stdc++.h>
#include <exception>
#include <glm/ext/vector_float3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

void check(bool cond, const std::string &message) {
  if (!cond)
    throw std::runtime_error(message);
}

std::vector<glm::vec4> initialize_vel(int N, float sd = 1.0) {
  std::vector<glm::vec4> arr(N);
  std::random_device r;
  std::default_random_engine gen(r());
  std::normal_distribution<float> dist(0, sd);
  float f = 1.0f;
  for (auto &v : arr) {
    /* v = glm::vec4(0.0, 0, 0, 0.0); */
    v = glm::vec4(f * dist(gen), f * dist(gen), f * dist(gen), 0.0);
  }
  return arr;
}

float max(const glm::vec3 &v) {
  return std::max(std::abs(v.x), std::max(std::abs(v.y), std::abs(v.z)));
}

std::vector<glm::vec4> initialize_random(int N, float &d, float sd = 1.0) {
  std::random_device r;
  std::default_random_engine gen;

  std::vector<glm::vec4> arr;
  std::uniform_real_distribution<float> mean(-30, 30);
  std::normal_distribution<float> dist(0, sd);

  d = 0;
  float f = 3.0;
  std::vector<glm::vec4> centers = {
      glm::vec4(10, -2, 20, 0), glm::vec4(10, 0, 0, 0),
      glm::vec4(10, 3, 0, 0),   glm::vec4(-10, 0, 0, 0),
      glm::vec4(0, 0, 10, 0),   glm::vec4(0, 0, -10, 0)};
  for (int k = 0; k < (int)centers.size() - 1; k++) {
    for (int i = 0; i < N / centers.size(); i++) {
      arr.push_back(
          glm::vec4(centers[k] + glm::vec4(f * dist(gen), f * dist(gen),
                                           f * dist(gen), 1.0)));
      d = std::max(max(arr.back()), d);
    }
  }
  for (int i = arr.size(); i < N; i++) {
    arr.push_back(
        glm::vec4(centers.back() +
                  glm::vec4(f * dist(gen), f * dist(gen), f * dist(gen), 1.0)));
    d = std::max(max(arr.back()), d);
  }
  return arr;
}

static std::ostream &operator<<(std::ostream &os, const glm::mat4 &m) {
  for (int i = 0; i < 4; i++) {
    os << '|';
    for (int j = 0; j < 4; j++) {
      os << m[i][j];
      if (j != 3)
        os << ' ';
    }
    os << "|\n";
  }
  return os;
}

static std::ostream &operator<<(std::ostream &os, const glm::vec2 &v) {
  return os << '(' << v.x << ", " << v.y << ')';
}

static std::ostream &operator<<(std::ostream &os, const glm::vec3 &v) {
  return os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
}

static std::ostream &operator<<(std::ostream &os, const glm::vec4 &v) {
  return os << '(' << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ')';
}

template <typename T>
static std::ostream &operator<<(std::ostream &os, const std::vector<T> &arr) {
  os << '[';
  if (arr.empty())
    goto out;

  os << arr.front();
  for (int i = 1; i < arr.size(); i++)
    os << ", " << arr[i];
out:
  return os << ']';
}

std::vector<float> gen_flare_tex(int tex_size) {
  std::vector<float> pixels(tex_size * tex_size);
  float sigma2 = tex_size / 2.0;
  float A = 1.0;
  for (int i = 0; i < tex_size; ++i) {
    float i1 = i - tex_size / 2;
    for (int j = 0; j < tex_size; ++j) {
      float j1 = j - tex_size / 2;
      // gamma corrected gauss
      pixels[i * tex_size + j] = pow(
          A * exp(-((i1 * i1) / (2 * sigma2) + (j1 * j1) / (2 * sigma2))), 2.2);
    }
  }
  return pixels;
}

template <typename T> class Value {
  T prev;
  T value;

public:
  explicit Value(T _value, bool init = false) : value{_value} {
    if (init)
      prev = _value;
  }
  operator T() const { return value; }
  T *operator->() { return &value; }
  T *operator&() { return &value; }
  bool has_changed() {
    if (prev != value) {
      prev = value;
      return true;
    }
    return false;
  }
};
