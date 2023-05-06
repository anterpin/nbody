#pragma once
#include "./utils.h"
#include <algorithm>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

using glm::vec3;
using std::vector;

static bool is_inside(const glm::vec3 &lbf, float size, const vec3 &pos) {
  float w = size + 0.000001;
  return lbf.x <= pos.x && lbf.y <= pos.y && lbf.z <= pos.z &&
         pos.x <= lbf.x + w && pos.y <= lbf.y + w && pos.z <= lbf.z + w;
}

static int get_subquadrant(const glm::vec3 &lbf, float size, const vec3 &pos) {
  const float hw = size / 2;
  int sq = 0;
  if (lbf.x + hw < pos.x)
    sq |= 1;
  if (lbf.y + hw < pos.y)
    sq |= 2;
  if (lbf.z + hw < pos.z)
    sq |= 4;
  return sq;
}

class BarnesHutTree {
  float domain = 200.0f;
  float threshold = 0.5f;
  float eps2 = 0.4f;
  static constexpr float MAX_DOMAIN = 1000.0f;
  static constexpr float EPS = 0.00001;
  void insert_node(int i, const vec3 &pos, float mass = 1.0) {
    assert(is_inside(lbf.at(i), sizes[i], pos));

    if (children[i]) { // it has children
      com[i].w += mass;
      int q = children[i] + get_subquadrant(lbf[i], sizes[i], pos);
      int tmp = com[q].w;
      insert_node(q, pos, mass);

      if (tmp < 0.5) {
        next[i] = q;
      }
      return;
    }
    if (com[i].w < 0.5) { // there is no particle;
      com[i].w = mass;
      com[i].x = pos.x;
      com[i].y = pos.y;
      com[i].z = pos.z;
      return;
    }
    // subdivide
    vec3 diff = vec3{com[i].x, com[i].y, com[i].z} - pos;
    float dist = glm::dot(diff, diff);
    if (dist < EPS) { // if for some reason two or more particles
                      // are too close together
                      // we consider them as one bigger particle
      // no ajust on the center of mass
      com[i].w += mass;
      return;
    }

    children[i] = children.size();

    const float hw = sizes[i] / 2;

    int qa = children[i] + get_subquadrant(lbf[i], sizes[i], com[i]);
    int qb = children[i] + get_subquadrant(lbf[i], sizes[i], pos);

    for (int k = 0; k < 8; k++) {
      float x = (k & 1) ? lbf[i].x + hw : lbf[i].x;
      float y = (k & 2) ? lbf[i].y + hw : lbf[i].y;
      float z = (k & 4) ? lbf[i].z + hw : lbf[i].z;
      lbf.emplace_back(x, y, z);
      sizes.push_back(hw);
      com.emplace_back(0, 0, 0, 0);
      children.push_back(0);
      parents.push_back(i);
      next.push_back(0);
    }
    insert_node(qa, vec3{com[i].x, com[i].y, com[i].z}, com[i].w);
    insert_node(qb, pos, mass);
    next[i] = qb;

    com[i].w += mass;
  }

  void compute_next(int i, int nex = -1) {
    next[i] = nex;
    if (!children[i]) {
      return;
    }
    int prev = nex;
    for (int k = 7; k >= 0; k--) {
      int q = children[i] + k;
      if (com[q].w < 0.5)
        continue;
      compute_next(q, prev);
      prev = q;
    }
  }
  void compute_com(int i) {
    if (!children[i]) {
      return;
    }
    com[i].x = 0;
    com[i].y = 0;
    com[i].z = 0;
    for (int k = 0; k < 8; k++) {
      compute_com(children[i] + k);
      const auto &child = com[children[i] + k];
      com[i].x += child.x * child.w;
      com[i].y += child.y * child.w;
      com[i].z += child.z * child.w;
    }
    if (com[i].w > 0.5) {
      com[i].x /= com[i].w; // node.mass won't be zero
      com[i].y /= com[i].w; // node.mass won't be zero
      com[i].z /= com[i].w; // node.mass won't be zero
    }
    assert(is_inside(lbf.at(i), sizes[i], vec3{com[i].x, com[i].y, com[i].z}));
  }

public:
  vec3 interact(const vec3 &a, const vec3 &b, float m) const {
    vec3 r = b - a;
    float ds = glm::dot(r, r) + eps2;
    return r * m * (1.0f / std::sqrt(ds * ds * ds));
  }
  vector<glm::vec3> lbf;
  vector<float> sizes;
  vector<glm::vec4> com;
  vector<int> children;
  vector<int> parents;
  vector<int> next;
  BarnesHutTree() {}
  void set_domain(float d) { domain = std::min(d, MAX_DOMAIN); }
  void set_threshold(float th) { threshold = th; }
  void create_tree(const vector<glm::vec4> &positions) {
    int n = positions.size();
    lbf.clear();
    sizes.clear();
    com.clear();
    children.clear();
    parents.clear();
    next.clear();

    lbf.emplace_back(-domain / 2, -domain / 2, -domain / 2);

    sizes.push_back(domain);

    children.push_back(0);

    com.emplace_back(0, 0, 0, 0);

    parents.push_back(-1);

    next.push_back(-1);

    for (int i = 0; i < n; i++) {
      float w = positions[i].w;
      vec3 pos =
          vec3{positions[i].x / w, positions[i].y / w, positions[i].z / w};
      if (!is_inside(lbf[0], sizes[0], pos)) {
#ifdef DEBUG
        std::cout << "Node " << i << " is out of the the domain "
                  << pos uint i = gl_GlobalInvocationID.x;
        << '\n';
#endif
        continue;
      }
      insert_node(0, pos);
    }
    compute_com(0);
    compute_next(0);
  }
  vec3 calc_acceleration(const vec3 &pos) const {
    int i = 0;
    vec3 acc = vec3(0);
    int rep = 0;
    while (i != -1) {
      float m = com.at(i).w;
      vec3 p = vec3{com.at(i).x, com.at(i).y, com.at(i).z};
      float d = glm::length(p - pos);
      float s = sizes[i];
      if (s / d < threshold || children.at(i) == 0) {
        auto a = interact(pos, p, m);
        if (m > 0.5) {
          acc += a;
        }
        i = next.at(i);
      } else {
        i = children.at(i);
        while (com[i].w < 0.5)
          i++;
      }
    }
    return acc;
  }

  void update_pos_and_velocities(vector<glm::vec4> &pos, vector<glm::vec4> &vel,
                                 float G, float dt) {
    float ma = 0;
    for (int i = 0; i < pos.size(); i++) {
      auto a = calc_acceleration(pos[i]);
      a *= *&G;
      vel[i] += glm::vec4(a * *&dt, 0);
      pos[i] += vel[i] * *&dt;
      ma = std::max(ma, max(pos[i]));
    }
    set_domain(ma * 3);
  }
};
