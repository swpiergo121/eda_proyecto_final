#include "str_xfast.h"
#include <iostream>

int main() {
  StringXFastTrie trie(15); // 4 chars * 8 bits = W = 32

  trie.insert("hoynes");
  trie.insert("inocentemente");
  trie.insert("indiscriminado");

  std::cout << (trie.find("inocentemente") ? "found" : "missing") << "\n";

  auto succ = trie.successor("hoynes");
  if (succ)
    std::cout << "after hoynes: " << *succ << "\n"; // beta

  auto pred = trie.predecessor("inocentemente");
  if (pred)
    std::cout << "before inocentemente: " << *pred << "\n"; // beta

  return 0;
}
