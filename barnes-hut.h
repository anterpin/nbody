#pragma once
#include "./utils.h"
#include <algorithm>
#include <array>
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
  float eps2 = 0.9f;
  float damping = 0.99998f;
  float EPS = 0.01;
  static constexpr float MAX_DOMAIN = 5000.0f;
  void create_child(int i, int q, const vec3 &pos, float mass) {
    children[i][q] = children.size();
    first_child[i] = children.size();
    children.push_back({0});
    com.emplace_back(pos.x, pos.y, pos.z, mass);
    first_child.push_back(0);
    next.push_back(0);

    const float hw = sizes[i] / 2;
    float x = (q & 1) ? lbf[i].x + hw : lbf[i].x;
    float y = (q & 2) ? lbf[i].y + hw : lbf[i].y;
    float z = (q & 4) ? lbf[i].z + hw : lbf[i].z;

    lbf.emplace_back(x, y, z);
    sizes.push_back(hw);
  }

  void insert_node(int i, const vec3 &pos, float mass) {
    while (true) {
#ifdef DEBUG
      assert(is_inside(lbf.at(i), sizes[i], pos));
#endif

      if (first_child[i]) { // it has children
        com[i].w += mass;
        int q = get_subquadrant(lbf[i], sizes[i], pos);
        if (!children[i][q]) { // do not exist this child
          create_child(i, q, pos, mass);
          break;
        }

        i = children[i][q];
        continue;
      }
      // subdivide
      vec3 diff = vec3{com[i].x, com[i].y, com[i].z} - pos;
      float dist = glm::dot(diff, diff);
      if (dist < EPS) { // if for some reason two or more particles
                        // are too close together
                        // we consider them as one bigger particle
        // no ajust on the center of mass
        com[i].w += mass;
        break;
      }

      const float hw = sizes[i] / 2;

      int qa = get_subquadrant(lbf[i], sizes[i], com[i]);
      int qb = get_subquadrant(lbf[i], sizes[i], pos);

      create_child(i, qa, vec3{com[i].x, com[i].y, com[i].z}, com[i].w);
      com[i].w += mass;
      if (qb != qa) {
        create_child(i, qb, pos, mass);
        break;
      }
      i = children[i][qa];
    }
  }

  void postprocess(int i, int nex) {
    next[i] = nex;
    if (!first_child[i]) {
      return;
    }
    com[i].x = 0;
    com[i].y = 0;
    com[i].z = 0;
    int prev = nex;
    for (int k = 7; k >= 0; k--) {
      int q = children[i][k];
      if (!q)
        continue;
      postprocess(q, prev);
      prev = q;
      const auto &child = com[q];
      com[i].x += child.x * child.w;
      com[i].y += child.y * child.w;
      com[i].z += child.z * child.w;
    }
    first_child[i] = prev;
    com[i].x /= com[i].w; // node.mass won't be zero
    com[i].y /= com[i].w; // node.mass won't be zero
    com[i].z /= com[i].w; // node.mass won't be zero
#ifdef DEBUG
    assert(com[i].w > 0.5);
    assert(is_inside(lbf.at(i), sizes[i], vec3{com[i].x, com[i].y, com[i].z}));
#endif
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
  vector<std::array<int, 8>> children;
  vector<int> first_child;
  vector<int> next;
  BarnesHutTree() {}
  void set_damping(float d) { damping = d; }
  void set_domain(float d) { domain = std::min(d, MAX_DOMAIN); }
  void set_threshold(float th) { threshold = th; }
  void set_eps2(float e) { eps2 = e; }
  void set_eps(float e) { EPS = e; }
  void create_tree(const vector<glm::vec4> &positions) {
    int n = positions.size();

    lbf.clear();
    sizes.clear();
    com.clear();
    children.clear();
    next.clear();
    first_child.clear();

    int first = 0;
    vec3 first_cube{-domain / 2, -domain / 2, -domain / 2};
    while (first < n) {
      float w = positions[first].w;
      vec3 pos{positions[first].x / w, positions[first].y / w,
               positions[first].z / w};
      if (is_inside(first_cube, domain, positions[first++])) {
        lbf.emplace_back(-domain / 2, -domain / 2, -domain / 2);
        sizes.push_back(domain);
        com.emplace_back(pos.x, pos.y, pos.z, 1);
        children.push_back({0});
        first_child.push_back(0);
        next.push_back(-1);
        break;
      } else {
#ifdef DEBUG
        std::cout << "Node " << first << " is out of the the domain " << pos
                  << '\n';
#endif
      }
    }
#ifdef DEBUG
    assert(lbf.size());
#endif

    for (int i = first; i < n; i++) {
      float w = positions[i].w;
      vec3 pos =
          vec3{positions[i].x / w, positions[i].y / w, positions[i].z / w};
      if (!is_inside(lbf[0], sizes[0], pos)) {
#ifdef DEBUG
        std::cout << "Node " << i << " is out of the the domain " << pos
                  << '\n';
#endif
        continue;
      }
      insert_node(0, pos, 1);
    }
    postprocess(0, -1);
  }

  vec3 calc_acceleration(const vec3 &pos) const {
    int i = 0;
    vec3 acc = vec3(0);
    while (i != -1) {
      vec3 p = vec3{com[i].x, com[i].y, com[i].z};
      if (first_child[i] == 0 || sizes[i] / glm::length(p - pos) < threshold) {
        auto a = interact(pos, p, com[i].w);
        acc += a;
        i = next[i];
      } else {
        i = first_child[i];
      }
    }
    return acc;
  }

  void update_pos_and_velocities(vector<glm::vec4> &pos, vector<glm::vec4> &vel,
                                 float G, float dt) {
    float ma = 0;
    for (int i = 0; i < pos.size(); i++) {
      auto a = calc_acceleration(pos[i]);
      a *= G;
      vel[i] += glm::vec4(a * dt, 0);
      pos[i] += vel[i] * dt;
      ma = std::max(ma, max(pos[i]));
    }
    set_domain(ma * 3);
  }
};
