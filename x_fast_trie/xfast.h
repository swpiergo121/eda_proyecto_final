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

typedef unsigned __int128 trie_key;

class x_fast_trie {

public:
  class node {
  public:
    trie_key prefix;
    trie_key level;
    node *left = nullptr;
    node *right = nullptr;

    // For use in leaves
    bool is_leaf = false;

    node(trie_key pre, trie_key lvl) : prefix(pre), level(lvl) {}
  };

  trie_key W;
  vector<unordered_map<trie_key, node *>> LSS;

  explicit x_fast_trie(trie_key num_bits) : W(num_bits), LSS(num_bits + 1) {
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

  bool find(trie_key key) {
    auto [level, n] = lowest_ancestor(key);
    return level == W;
  }

  vector<trie_key> get_closest(trie_key key, int n) {
    vector<trie_key> result;
    if (n <= 0 || head_ == nullptr)
      return result;

    // 1. Find a starting leaf.
    // If key exists, successor(key) returns the key itself.
    // If key doesn't exist, it returns the smallest element > key.
    trie_key start_key = successor(key);
    if (start_key == 0) {
      // The key is greater than everything in the trie;
      // the closest values are at the end.
      start_key = max_key();
    }

    node *curr = LSS[W].at(start_key);

    // 2. Determine how many to go back from the start_key.
    // We want roughly n/2 behind and n/2 ahead.
    int go_back = n / 2;
    node *temp = curr;
    while (temp && go_back > 0) {
      temp = temp->left;
      go_back--;
    }

    // If we went back, the 'temp' pointer is now the leftmost element of our
    // window. If we hit the head, we start from the head.
    node *window_start = temp ? temp : head_;

    // 3. Collect up to 'n' values moving forward.
    node *walker = window_start;
    while (walker && result.size() < static_cast<size_t>(n)) {
      result.push_back(walker->prefix);
      walker = walker->right;
    }

    return result;
  }

  int leaf_count() const { return lc; }

  pair<trie_key, node *> lowest_ancestor(trie_key key) {
    trie_key low = 0;
    trie_key high = W;

    while (low < high) {
      trie_key mid = (low + high + 1) / 2;
      trie_key pref = key >> (W - mid);
      auto it = LSS[mid].find(pref);
      if (it != LSS[mid].end()) {
        low = mid;
      } else {
        high = mid - 1;
      }
    }
    trie_key pref = (low == 0) ? 0 : (key >> (W - low));
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

  void fix_descendants(trie_key key) {
    for (int lvl = static_cast<int>(W) - 1; lvl >= 0; --lvl) {
      trie_key prefix = (lvl == 0) ? 0 : (key >> (W - lvl));
      node *n = LSS[lvl].at(prefix);

      trie_key left_pref = (prefix << 1);
      trie_key right_pref = (prefix << 1) | 1;

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
  trie_key successor(trie_key key) {
    auto [level, n] = lowest_ancestor(key);
    if (level == W) {
      return key;
    }

    trie_key bit = (key >> (W - level - 1)) & 1;

    if (bit == 0) {
      node *succ = n->left;
      if (succ == nullptr)
        return 0;
      return succ->prefix;
    } else {
      node *pred = n->right;
      if (pred == nullptr || pred->right == nullptr)
        return 0;
      return pred->right->prefix;
    }
  }

  trie_key predecessor(trie_key key) {
    auto [level, n] = lowest_ancestor(key);
    if (level == W) {
      return key;
    }

    trie_key bit = (key >> (W - level - 1)) & 1;

    if (bit == 0) {
      node *succ = n->left;
      if (succ == nullptr || succ->left == nullptr)
        return 0;
      return succ->left->prefix;
    } else {
      node *pred = n->right;
      if (pred == nullptr)
        return 0;
      return pred->prefix;
    }
  }

  // FIX 2: Strict successor (in-order next element, never returns key itself)
  trie_key strict_successor(trie_key key) {
    auto it = LSS[W].find(key);
    if (it != LSS[W].end()) {
      if (it->second->right)
        return it->second->right->prefix;
      return 0;
    }
    return successor(key);
  }

  // Strict predecessor (in-order previous element, never returns key itself)
  trie_key strict_predecessor(trie_key key) {
    auto it = LSS[W].find(key);
    if (it != LSS[W].end()) {
      if (it->second->left)
        return it->second->left->prefix;
      return 0;
    }
    return predecessor(key);
  }

  trie_key min_key() const { return head_ ? head_->prefix : 0; }

  trie_key max_key() const { return tail_ ? tail_->prefix : 0; }

  void insert(trie_key key) {
    if (find(key))
      return;

    lc++;

    node *leaf = new node(key, W);
    leaf->is_leaf = true;
    leaf->left = nullptr;
    leaf->right = nullptr;

    if (head_ == nullptr) {
      head_ = tail_ = leaf;
    } else {
      auto pred_opt = predecessor(key);
      auto succ_opt = successor(key);

      if (pred_opt != 0) {
        node *pred_node = LSS[W].at(pred_opt);
        leaf->left = pred_node;
        leaf->right = pred_node->right;
        if (pred_node->right)
          pred_node->right->left = leaf;
        pred_node->right = leaf;
      } else if (succ_opt != 0) {
        node *succ_node = LSS[W].at(succ_opt);
        leaf->right = succ_node;
        leaf->left = succ_node->left;
        if (succ_node->left) {
          succ_node->left->right = leaf;
        }
        succ_node->left = leaf;
      } else {
        head_ = tail_ = leaf;
        leaf->left = leaf->right = nullptr;
      }

      if (pred_opt == 0)
        head_ = leaf;
      if (succ_opt == 0)
        tail_ = leaf;
    }

    // Walk down from root to create missing internal nodes
    node *cur = root_;
    for (trie_key lvl = 1; lvl < W; ++lvl) {
      trie_key pref = key >> (W - lvl);
      trie_key bit = (key >> (W - lvl)) & 1;

      node *&child_ref = (bit == 0) ? cur->left : cur->right;
      if (LSS[lvl].find(pref) == LSS[lvl].end()) {
        child_ref = new node(pref, lvl);
        LSS[lvl][pref] = child_ref;
      }
      cur = LSS[lvl].at(pref);
    }

    // Link leaf as child of level W-1 node
    if (W > 0) {
      trie_key bit = key & 1;
      if (bit == 0)
        cur->left = leaf;
      else
        cur->right = leaf;
    }

    LSS[W][key] = leaf;
    fix_descendants(key);
  }

  void remove(trie_key key) {
    // TODO
  }
};

#endif // XFAST_H_
