#pragma once
#include <array>
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

std::vector<glm::vec4> initialize_vel(const std::vector<glm::vec4> &pos) {
  std::mt19937 rng;
  std::uniform_real_distribution<> dis(0, 1);

  std::vector<glm::vec4> arr;
  arr.reserve(pos.size());
  for (int i = 0; i < pos.size(); i++) {
    glm::vec3 vel = glm::cross(glm::vec3(pos[i]), glm::vec3(0, 1, 0));
    float orbital_vel = sqrt(2.0 * glm::length(vel));
    vel = glm::normalize(vel) * orbital_vel;
    arr.emplace_back(vel, 0.0);
  }
  return arr;
}

float max(const glm::vec3 &v) {
  return std::max(std::abs(v.x), std::max(std::abs(v.y), std::abs(v.z)));
}

std::vector<glm::vec4> initialize_pos(int N, float &d) {
  std::mt19937 rng;
  std::uniform_real_distribution<> dis(0, 1);

  std::vector<glm::vec4> arr;
  arr.reserve(N);
  d = 0;
  for (int i = 0; i < N; i++) {
    glm::vec4 particle;
    float t = dis(rng) * 2 * 3.141592653;
    float s = dis(rng) * 100;
    particle.x = cos(t) * s;
    particle.z = sin(t) * s;
    particle.y = dis(rng) * 4;

    particle.w = 1.f;
    arr.push_back(particle);
    d = std::max(d, max(glm::vec3{particle.x, particle.y, particle.z}));
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

static std::ostream &operator<<(std::ostream &os,
                                const std::array<int, 8> &arr) {
  os << '[';
  for (int i = 0; i < 7; i++) {
    os << arr[i] << ", ";
  }
  return os << arr.back() << ']';
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
