#pragma once
#include "./utils.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <vector>

using glm::vec3;
using std::vector;
struct Quadrant { // Cubant?
  vec3 lbf;       // left bottom front corner
  float width;
  vec3 com; // center of mass
  float mass = 0;
  int children = 0;
  Quadrant(const vec3 &_lbf, float _width) : lbf{_lbf}, width{_width} {}
  bool inside(const vec3 &pos) const {
    return lbf.x <= pos.x && lbf.y <= pos.y && lbf.z <= pos.z &&
           pos.x < lbf.x + width && pos.y < lbf.y + width &&
           pos.z < lbf.z + width;
  }
  int get_subquadrant(const vec3 &pos) const {
    const float hw = width / 2;
    int sq = 0;
    if (pos.x >= lbf.x + hw)
      sq |= 1;
    if (pos.y >= lbf.y + hw)
      sq |= 2;
    if (pos.z >= lbf.z + hw)
      sq |= 4;
    return sq;
  }
};

static std::ostream &operator<<(std::ostream &os, const Quadrant &q) {
  return os << "{ mass: " << q.mass << ", pos: " << q.lbf
            << ", width: " << q.width << " }";
}

class BarnesHutTree {
  static constexpr float DOMAIN = 200.0f;
  vector<Quadrant> tree;
  void insert_node(int i, const vec3 &pos) {
#define node (tree[i])
    if (!node.inside(pos)) {
      std::cerr << "node is not inside " << node << ' ' << pos << '\n';
      exit(-1);
    }
    if (node.children) { // it has children
      node.mass += 1.0;
      insert_node(node.children + node.get_subquadrant(pos), pos);
      return;
    }
    if (node.mass < 0.5) { // there is no node;
      node.mass = 1.0;
      node.com = pos;
      return;
    }
    // subdivide
    static constexpr float eps = 0.00001;
    vec3 diff = node.com - pos;
    float dist = glm::dot(diff, diff);
    if (dist < eps) { // if for some reason two or more particles
                      // are too close together
                      // we consider them as one bigger particle
      node.mass += 1.0;
      return;
    }

    node.children = tree.size();

    const float hw = node.width / 2;
#define lbf (tree[i].lbf)
    for (int k = 0; k < 8; k++) {
      vec3 v = vec3{lbf.x + ((k & 1) ? hw : 0), lbf.y + ((k & 2) ? hw : 0),
                    lbf.z + ((k & 4) ? hw : 0)};
      tree.emplace_back(v, hw);
    }
    // tree.emplace_back(lbf, hw);                               // 0
    // tree.emplace_back(vec3{lbf.x + hw, lbf.y, lbf.z}, hw);            //
    // 1 tree.emplace_back(vec3{lbf.x, lbf.y + hw, lbf.z}, hw); // 2
    // tree.emplace_back(vec3{lbf.x + hw, lbf.y + hw, lbf.z}, hw);       //
    // 3 tree.emplace_back(vec3{lbf.x, lbf.y, lbf.z + hw}, hw); // 4
    // tree.emplace_back(vec3{lbf.x + hw, lbf.y, lbf.z + hw}, hw);       //
    // 5 tree.emplace_back(vec3{lbf.x, lbf.y + hw, lbf.z + hw}, hw); // 6
    // tree.emplace_back(vec3{lbf.x + hw, lbf.y + hw, lbf.z + hw}, hw);  //
    // 7
#undef lbf

    insert_node(node.children + node.get_subquadrant(node.com), node.com);
    insert_node(node.children + node.get_subquadrant(pos), pos);
#undef node
  }

  void compute_com(int i) {
    auto &node = tree[i];
    if (!node.children) {
      return;
    }
    node.com = vec3(0.0);
    for (int i = 0; i < 8; i++) {
      const auto &child = tree[node.children + i];
      node.com += child.mass * child.com;
    }
    node.com /= node.mass; // node.mass won't be zero
  }

public:
  BarnesHutTree() {}
  void create_tree(const vector<glm::vec4> &positions) {
    int n = positions.size();
    tree.clear();
    tree.push_back(
        Quadrant(vec3(-DOMAIN / 2, -DOMAIN / 2, -DOMAIN / 2), DOMAIN));
    for (int i = 0; i < n; i++) {
      float w = positions[i].w;
      vec3 pos =
          vec3{positions[i].x / w, positions[i].y / w, positions[i].z / w};
      if (!tree[0].inside(pos)) {
        std::cout << "Node " << i << " is out of the the domain " << pos
                  << '\n';
        continue;
      }
      insert_node(0, pos);
    }
    compute_com(0);
  }
  const vector<Quadrant> &get_tree() const { return tree; }
};
