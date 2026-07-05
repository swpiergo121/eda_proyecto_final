#include "str_xfast.h"
#include <iostream>

int main() {
  StringXFastTrie trie(5); // 4 chars * 8 bits = W = 32

  trie.insert("beta");
  trie.insert("alpha");
  trie.insert("gamma");

  std::cout << (trie.find("alpha") ? "found" : "missing") << "\n";

  auto succ = trie.successor("alpha");
  if (succ)
    std::cout << "after alpha: " << *succ << "\n"; // beta

  auto pred = trie.predecessor("gamma");
  if (pred)
    std::cout << "before gamma: " << *pred << "\n"; // beta

  return 0;
}
