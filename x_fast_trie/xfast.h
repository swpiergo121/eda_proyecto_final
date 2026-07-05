#ifndef XFAST_H_
#define XFAST_H_

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

class x_fast_trie {

public:
  class node {
  public:
    uint64_t prefix;
    uint32_t level;
    node *left = nullptr;
    node *right = nullptr;

    // For use in leaves
    uint64_t key = 0;
    node *prev = nullptr;
    node *next = nullptr;
    bool is_leaf = false;

    node(uint64_t pre, uint32_t lvl) : prefix(pre), level(lvl) {}
  };

  uint32_t W;
  vector<unordered_map<uint64_t, node *>> LSS;

  explicit x_fast_trie(uint32_t num_bits) : W(num_bits), LSS(num_bits + 1) {
    root_ = new node(0, 0);
    LSS[0][0] = root_;
    head_ = tail_ = nullptr;
    lc = 0;
  }

  // FIX 1: Delete every node exactly once via LSS.
  //        Do NOT recurse on left/right, because descendant pointers alias
  //        the same leaf from multiple ancestors.
  ~x_fast_trie() {
    for (auto &level_map : LSS) {
      for (auto &kv : level_map) {
        delete kv.second;
      }
    }
  }

  node *root_;
  node *head_;
  node *tail_;
  int lc;

  bool find(uint64_t key) {
    auto [level, n] = lowest_ancestor(key);
    return level == W;
  }

  int leaf_count() const { return lc; }

  pair<uint32_t, node *> lowest_ancestor(uint64_t key) {
    uint32_t low = 0;
    uint32_t high = W;

    while (low < high) {
      uint32_t mid = (low + high + 1) / 2;
      uint64_t pref = key >> (W - mid);
      auto it = LSS[mid].find(pref);
      if (it != LSS[mid].end()) {
        low = mid;
      } else {
        high = mid - 1;
      }
    }
    uint64_t pref = (low == 0) ? 0 : (key >> (W - low));
    return {low, LSS[low].at(pref)};
  }

  node *min_leaf(node *n) {
    if (!n)
      return nullptr;
    while (!n->is_leaf) {
      n = n->left ? n->left : n->right;
    }
    return n;
  }

  node *max_leaf(node *n) {
    if (!n)
      return nullptr;
    while (!n->is_leaf) {
      n = n->right ? n->right : n->left;
    }
    return n;
  }

  void fix_descendants(uint64_t key) {
    for (int lvl = static_cast<int>(W) - 1; lvl >= 0; --lvl) {
      uint64_t prefix = (lvl == 0) ? 0 : (key >> (W - lvl));
      node *n = LSS[lvl].at(prefix);

      uint64_t left_pref = (prefix << 1);
      uint64_t right_pref = (prefix << 1) | 1;

      bool has_left = (LSS[lvl + 1].find(left_pref) != LSS[lvl + 1].end());
      bool has_right = (LSS[lvl + 1].find(right_pref) != LSS[lvl + 1].end());

      if (!has_left) {
        n->left = min_leaf(n->right);
      }
      if (!has_right) {
        n->right = max_leaf(n->left);
      }
    }
  }

  // Standard ceiling / floor (returns key itself if present)
  uint64_t successor(uint64_t key) {
    auto [level, n] = lowest_ancestor(key);
    if (level == W) {
      return key;
    }

    uint32_t bit = (key >> (W - level - 1)) & 1;

    if (bit == 0) {
      node *succ = n->left;
      if (succ == nullptr)
        return 0;
      return succ->key;
    } else {
      node *pred = n->right;
      if (pred == nullptr || pred->next == nullptr)
        return 0;
      return pred->next->key;
    }
  }

  uint64_t predecessor(uint64_t key) {
    auto [level, n] = lowest_ancestor(key);
    if (level == W) {
      return key;
    }

    uint32_t bit = (key >> (W - level - 1)) & 1;

    if (bit == 0) {
      node *succ = n->left;
      if (succ == nullptr || succ->prev == nullptr)
        return 0;
      return succ->prev->key;
    } else {
      node *pred = n->right;
      if (pred == nullptr)
        return 0;
      return pred->key;
    }
  }

  // FIX 2: Strict successor (in-order next element, never returns key itself)
  uint64_t strict_successor(uint64_t key) {
    auto it = LSS[W].find(key);
    if (it != LSS[W].end()) {
      if (it->second->next)
        return it->second->next->key;
      return 0;
    }
    return successor(key);
  }

  // Strict predecessor (in-order previous element, never returns key itself)
  uint64_t strict_predecessor(uint64_t key) {
    auto it = LSS[W].find(key);
    if (it != LSS[W].end()) {
      if (it->second->prev)
        return it->second->prev->key;
      return 0;
    }
    return predecessor(key);
  }

  uint64_t min_key() const { return head_ ? head_->key : 0; }

  uint64_t max_key() const { return tail_ ? tail_->key : 0; }

  void insert(uint64_t key) {
    if (find(key))
      return;

    lc++;

    node *leaf = new node(key, W);
    leaf->is_leaf = true;
    leaf->key = key;
    leaf->left = nullptr;
    leaf->right = nullptr;
    leaf->prev = nullptr;
    leaf->next = nullptr;

    if (head_ == nullptr) {
      head_ = tail_ = leaf;
    } else {
      auto pred_opt = predecessor(key);
      auto succ_opt = successor(key);

      if (pred_opt != 0) {
        node *pred_node = LSS[W].at(pred_opt);
        leaf->prev = pred_node;
        leaf->next = pred_node->next;
        if (pred_node->next)
          pred_node->next->prev = leaf;
        pred_node->next = leaf;
      } else if (succ_opt != 0) {
        node *succ_node = LSS[W].at(succ_opt);
        leaf->next = succ_node;
        leaf->prev = succ_node->prev;
        if (succ_node->prev)
          succ_node->prev->next = leaf;
        succ_node->prev = leaf;
      } else {
        head_ = tail_ = leaf;
        leaf->prev = leaf->next = nullptr;
      }

      if (pred_opt == 0)
        head_ = leaf;
      if (succ_opt == 0)
        tail_ = leaf;
    }

    // Walk down from root to create missing internal nodes
    node *cur = root_;
    for (uint32_t lvl = 1; lvl < W; ++lvl) {
      uint64_t pref = key >> (W - lvl);
      uint32_t bit = (key >> (W - lvl)) & 1;

      node *&child_ref = (bit == 0) ? cur->left : cur->right;
      if (LSS[lvl].find(pref) == LSS[lvl].end()) {
        child_ref = new node(pref, lvl);
        LSS[lvl][pref] = child_ref;
      }
      cur = LSS[lvl].at(pref);
    }

    // Link leaf as child of level W-1 node
    if (W > 0) {
      uint32_t bit = key & 1;
      if (bit == 0)
        cur->left = leaf;
      else
        cur->right = leaf;
    }

    LSS[W][key] = leaf;
    fix_descendants(key);
  }

  void remove(uint64_t key) { (void)key; }
};

#endif // XFAST_H_
