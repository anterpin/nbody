#pragma once
#include <exception>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

void check(bool cond, const std::string &message) {
  if (!cond)
    throw std::runtime_error(message);
}

#include <bits/stdc++.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/vec3.hpp>

std::vector<glm::vec3> initialize_random(int N) {
  constexpr float sd = 1.0;
  /* std::random_device r; */
  /* std::default_random_engine gen(r()); */
  std::default_random_engine gen;

  std::normal_distribution<float> dist(0, sd);

  std::vector<glm::vec3> arr(N);
  for (auto &x : arr)
    x = glm::vec3(dist(gen), dist(gen), dist(gen));
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
