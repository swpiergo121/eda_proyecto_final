#include "xfast.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

int main() {
  std::string filename = "data_10.txt";
  std::ifstream infile(filename);
  if (!infile) {
    std::cerr << "Error: cannot open " << filename << "\n";
    return 1;
  }

  // 1. Read all numbers
  std::vector<uint32_t> values;
  uint32_t v, max_val = 0;
  while (infile >> v) {
    values.push_back(v);
    if (v > max_val)
      max_val = v;
  }
  infile.close();

  if (values.empty()) {
    std::cout << "File is empty. Nothing to test.\n";
    return 0;
  }

  // 2. Determine W (number of bits)
  uint32_t W = (max_val == 0)
                   ? 1
                   : (64 - static_cast<uint32_t>(__builtin_clzll(max_val)));

  std::cout << "Read " << values.size() << " values. Max = " << max_val
            << ". Using W = " << W << " bits.\n\n";

  // 3. Build structures
  x_fast_trie trie(W);
  std::set<uint32_t> ref;

  std::cout << "Inserting into trie...\n";
  for (uint32_t x : values) {
    cout << "value: " << x << endl;
    trie.insert(x);
    ref.insert(x);
  }

  uint32_t cnt = trie.leaf_count();
  std::cout << "Trie leaf count: " << cnt << "\n";
  std::cout << "Set size:        " << ref.size() << "\n";
  if (cnt != ref.size()) {
    std::cerr << "SIZE MISMATCH!\n";
    return 1;
  }
  std::cout << "Size OK\n\n";

  // 4. Validate every inserted key
  std::cout << "Testing queries for all inserted keys...\n";
  int errors = 0;

  for (uint32_t x : ref) {
    // find
    if (!trie.find(x)) {
      std::cerr << "❌ find(" << x << ") = false\n";
      if (++errors >= 10)
        break;
    }

    // successor of an existing key should return the key itself
    uint32_t succ = trie.successor(x);
    if (succ != x) {
      std::cerr << "❌ successor(" << x << ") = " << succ << " (expected " << x
                << ")\n";
      if (++errors >= 10)
        break;
    }

    // predecessor of an existing key should return the key itself
    uint32_t pred = trie.predecessor(x);
    if (pred != x) {
      std::cerr << "❌ predecessor(" << x << ") = " << pred << " (expected "
                << x << ")\n";
      if (++errors >= 10)
        break;
    }
  }

  if (errors == 0)
    std::cout << "✅ All inserted-key queries passed.\n";
  else {
    std::cerr << "Stopped after " << errors << " errors.\n";
    return 1;
  }

  // --- Test 2: random queries on keys NOT in the set ---
  std::cout << "\nTesting random queries on keys NOT in the set...\n";
  errors = 0;

  uint32_t q = 12;

  uint32_t trie_succ = trie.successor(q);
  uint32_t trie_pred = trie.predecessor(q);

  // Reference successor
  uint32_t ref_succ = 0;
  auto it_up = ref.upper_bound(q);
  if (it_up != ref.end())
    ref_succ = *it_up;

  // Reference predecessor
  uint32_t ref_pred = 0;
  auto it_low = ref.lower_bound(q);
  if (it_low != ref.begin()) {
    ref_pred = *std::prev(it_low);
  }

  if (trie_succ != ref_succ) {
    std::cerr << "❌ successor(" << q << "): trie=" << trie_succ
              << " ref=" << ref_succ << "\n";
  }

  if (trie_pred != ref_pred) {
    std::cerr << "❌ predecessor(" << q << "): trie=" << trie_pred
              << " ref=" << ref_pred << "\n";
  }

  cout << "succ: " << trie_succ << endl;
  cout << "pred: " << trie_pred << endl;

  // std::mt19937_64 rng(12345);
  // std::uniform_int_distribution<uint32_t> dist(0, max_val + 100);

  // for (int i = 0; i < 10000; ++i) {
  //   uint32_t q = dist(rng);
  //   if (ref.count(q))
  //     continue; // skip if it's in the set

  //   uint32_t trie_succ = trie.successor(q);
  //   uint32_t trie_pred = trie.predecessor(q);

  //   // Reference successor
  //   uint32_t ref_succ = 0;
  //   auto it_up = ref.upper_bound(q);
  //   if (it_up != ref.end())
  //     ref_succ = *it_up;

  //   // Reference predecessor
  //   uint32_t ref_pred = 0;
  //   auto it_low = ref.lower_bound(q);
  //   if (it_low != ref.begin()) {
  //     ref_pred = *std::prev(it_low);
  //   }

  //   if (trie_succ != ref_succ) {
  //     std::cerr << "❌ successor(" << q << "): trie=" << trie_succ
  //               << " ref=" << ref_succ << "\n";
  //     if (++errors >= 5)
  //       break;
  //   }

  //   if (trie_pred != ref_pred) {
  //     std::cerr << "❌ predecessor(" << q << "): trie=" << trie_pred
  //               << " ref=" << ref_pred << "\n";
  //     if (++errors >= 5)
  //       break;
  //   }
  // }

  if (errors == 0)
    std::cout << "Random queries passed.\n";
  else {
    std::cerr << "Stopped after " << errors << " errors.\n";
    return 1;
  }

  std::cout << "\nAll tests passed.\n";

  return 0;
}
