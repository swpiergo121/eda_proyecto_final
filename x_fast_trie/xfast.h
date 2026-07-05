#ifndef XFAST_H_
#define XFAST_H_

#include <cmath>
#include <cstdint>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

class x_fast_trie {

  class node {

  public:
    uint32_t prefix;
    uint32_t level;
    node *left = nullptr;
    node *right = nullptr;

    // For use in leafs
    uint32_t key;
    node *prev = nullptr;
    node *next = nullptr;
    bool is_leaf;

    node(uint32_t pre, uint32_t level) : prefix(pre), level(level) {}
  };

public:
  uint32_t W;
  vector<std::unordered_map<uint32_t, node *>> LSS;

  explicit x_fast_trie(uint32_t num_bits) : W(num_bits), LSS(num_bits + 1) {
    // level 0, prefix "" (0 bits)
    root_ = new node(0, 0);
    LSS[0][0] = root_;
    head_ = tail_ = nullptr;

    lc = 0;
  }

  ~x_fast_trie() {
    // Clean up all nodes (omitted for brevity – simple DFS works)
  }

  node *root_;
  // smallest leaf
  node *head_;
  // largest leaf
  node *tail_;
  int lc;

  // looks up for key
  bool find(uint32_t key) {
    auto [level, n] = lowest_ancestor(key);
    return level == W;
  }

  int leaf_count() { return lc; }

  pair<uint32_t, node *> lowest_ancestor(uint32_t key) {
    uint32_t low = 0;
    uint32_t high = W;

    while (low < high) {
      uint32_t mid = (low + high + 1) / 2;
      uint32_t pref = key >> (W - mid);
      auto it = LSS[mid].find(pref);
      if (it != LSS[mid].end()) {
        low = mid;
      } else {
        high = mid - 1;
      }
    }
    uint32_t pref = (low == 0) ? 0 : (key >> (W - low));
    ;
    return {low, LSS[low].at(pref)};
  }

  // Returns the minimum leaf in the subtree rooted at n.
  // n is guaranteed to have at least one leaf under it.
  node *min_leaf(node *n) {
    while (!n->is_leaf) {
      n = n->left ? n->left : n->right;
    }
    return n;
  }

  // Returns the maximum leaf in the subtree rooted at n.
  node *max_leaf(node *n) {
    while (!n->is_leaf) {
      n = n->right ? n->right : n->left;
    }
    return n;
  }

  void fix_descendants(uint32_t key) {
    uint32_t prefix = key;
    for (int lvl = W - 1; lvl >= 0; --lvl) {
      prefix = prefix >> 1;

      // Use .at() so you get an exception instead of a segfault
      // if the prefix is somehow missing (should never happen after the
      // walk-down).
      node *n = LSS[lvl].at(prefix);

      if (n->left == nullptr) {
        n->left = min_leaf(n->right);
      }
      if (n->right == nullptr) { // <-- separate if, NOT else if
        n->right = max_leaf(n->left);
      }
    }
  }

  // looks up for the key value pair with the smallest key larger than any or
  // ecual to the key
  uint32_t successor(uint32_t key) {
    auto [level, n] = lowest_ancestor(key);
    // key exists
    if (level == W) {
      return key;
    }

    uint32_t bit = (key >> (W - level - 1)) & 1;

    if (bit == 0) {
      node *succ = n->left;
      if (succ == nullptr) {
        return 0;
      }
      return succ->key;
    } else {
      node *pred = n->right;
      if (pred == nullptr || pred->next == nullptr) {
        return 0;
      }
      return pred->next->key;
    }
  }

  // looks up for the key value pair with the largest key smaller than any or
  // ecual to the key
  uint32_t predecessor(uint32_t key) {
    auto [level, n] = lowest_ancestor(key);
    // key exists
    if (level == W) {
      return key;
    }

    uint32_t bit = (key >> (W - level - 1)) & 1;

    if (bit == 0) {
      node *succ = n->left;
      if (succ == nullptr || succ->prev == nullptr) {
        return 0;
      }
      return succ->prev->key;
    } else {
      node *pred = n->right;
      if (pred == nullptr) {
        return 0;
      }
      return pred->key;
    }
  }

  // inserts the key with the value
  void insert(uint32_t key) {
    if (find(key)) {
      // doesn't allows duplicates
      return;
    }

    lc++;

    node *leaf = new node(key, W);
    leaf->is_leaf = true;
    leaf->key = key;
    leaf->left = nullptr;
    leaf->left = nullptr;
    leaf->right = nullptr;
    leaf->right = nullptr;

    // cout << "after node creation" << endl;

    if (head_ == nullptr) {
      head_ = tail_ = leaf;
      // cout << "empty trie" << endl;
    } else {
      // 1. find pred and succ
      auto pred_opt = predecessor(key);
      auto succ_opt = successor(key);
      // 2. Create leaf and splice it into the linked list
      // We assume only positive numbers
      if (pred_opt != 0) {
        node *pred_node = LSS[W].at(pred_opt);
        leaf->prev = pred_node;
        leaf->next = pred_node->next;
        if (pred_node->next) {
          pred_node->next->prev = leaf;
        }
        pred_node->next = leaf;
      } else if (succ_opt != 0) {
        node *succ_node = LSS[W].at(succ_opt);
        leaf->next = succ_node;
        leaf->prev = succ_node->prev;
        if (succ_node->prev) {
          succ_node->prev->next = leaf;
        }
        succ_node->prev = leaf;
      } else {
        head_ = tail_ = leaf;
        leaf->prev = leaf->next = nullptr;
      }

      if (pred_opt == 0) {
        head_ = leaf;
      }
      if (succ_opt == 0) {
        tail_ = leaf;
      }
    }

    // 3. Walk down fromm root to create the missing internal nodes
    // cout << "Walk down" << endl;
    node *cur = root_;
    for (uint32_t lvl = 1; lvl < W; ++lvl) {
      uint32_t pref = key >> (W - lvl);
      uint32_t bit = (key >> (W - lvl)) & 1; // the bit we branch on

      node *&child_ref = (bit == 0) ? cur->left : cur->right;
      if (LSS[lvl].find(pref) == LSS[lvl].end()) {
        child_ref = new node(pref, lvl);
        LSS[lvl][pref] = child_ref;
      }
      cur = LSS[lvl].at(pref);
    }

    // 4. Link the pre-created leaf as the child of the level W-1 node
    if (W > 0) {
      uint32_t bit = key & 1; // last bit = (key >> 0) & 1
      if (bit == 0) {
        cur->left = leaf;
      } else {
        cur->right = leaf;
      }
    }

    // 5. NOW put the leaf into LSS[W]
    LSS[W][key] = leaf;

    // 6. Walk up fixing descendants

    // 6. Walk up to fix descendant
    // cout << "Fix descendants" << endl;
    fix_descendants(key);
  }

  // removes the key
  void remove(uint32_t key, uint32_t v) {
    // TODO
  }
};

#endif // XFAST_H_
